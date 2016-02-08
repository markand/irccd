/*
 * server-state.cpp -- server current state
 *
 * Copyright (c) 2013-2016 David Demelier <markand@malikania.fr>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <cassert>

#include <irccd-config.h>

#if !defined(_WIN32)
#  include <sys/types.h>
#  include <netinet/in.h>
#  include <arpa/nameser.h>
#  include <resolv.h>
#endif

#include "server-state.h"
#include "server.h"

namespace irccd {

bool ServerState::connect(Server &server)
{
	const ServerInfo &info = server.info();
	const ServerIdentity &identity = server.identity();
	const char *password = info.password.empty() ? nullptr : info.password.c_str();
	std::string host = info.host;
	int code;

	/* libircclient requires # for SSL connection */
	if (info.flags & ServerInfo::Ssl)
		host.insert(0, 1, '#');
	if (!(info.flags & ServerInfo::SslVerify))
		irc_option_set(server.session(), LIBIRC_OPTION_SSL_NO_VERIFY);

	if (info.flags & ServerInfo::Ipv6) {
		code = irc_connect6(server.session(), host.c_str(), info.port, password,
				    identity.nickname.c_str(),
				    identity.username.c_str(),
				    identity.realname.c_str());
	} else {
		code = irc_connect(server.session(), host.c_str(), info.port, password,
				   identity.nickname.c_str(),
				   identity.username.c_str(),
				   identity.realname.c_str());
	}

	return code == 0;
}

void ServerState::prepareConnected(Server &server, fd_set &setinput, fd_set &setoutput, net::Handle &maxfd)
{
	if (!irc_is_connected(server.session())) {
		const ServerSettings &settings = server.settings();

		log::warning() << "server " << server.info().name << ": disconnected" << std::endl;

		if (settings.recotimeout > 0) {
			log::warning() << "server " << server.info().name << ": retrying in "
					  << settings.recotimeout << " seconds" << std::endl;
		}

		server.next(ServerState::Disconnected);
	} else {
		irc_add_select_descriptors(server.session(), &setinput, &setoutput, reinterpret_cast<int *>(&maxfd));
	}
}

void ServerState::prepareConnecting(Server &server, fd_set &setinput, fd_set &setoutput, net::Handle &maxfd)
{
	/*
	 * The connect function will either fail if the hostname wasn't resolved
	 * or if any of the internal functions fail.
	 *
	 * It returns success if the connection was successful but it does not
	 * mean that connection is established.
	 *
	 * Because this function will be called repeatidly from the
	 * ServerManager, if the connection was started and we're still not
	 * connected in the specified timeout time, we mark the server
	 * as disconnected.
	 *
	 * Otherwise, the libircclient event_connect will change the state.
	 */
	const ServerInfo &info = server.info();

	if (m_started) {
		const ServerSettings &settings = server.settings();

		if (m_timer.elapsed() > static_cast<unsigned>(settings.recotimeout * 1000)) {
			log::warning() << "server " << info.name << ": timeout while connecting" << std::endl;
			server.next(ServerState::Disconnected);
		} else if (!irc_is_connected(server.session())) {
			log::warning() << "server " << info.name << ": error while connecting: "
					  << irc_strerror(irc_errno(server.session())) << std::endl;

			if (settings.recotimeout > 0) {
				log::warning() << "server " << info.name << ": retrying in " << settings.recotimeout << " seconds" << std::endl;
			}

			server.next(ServerState::Disconnected);
		} else {
			irc_add_select_descriptors(server.session(), &setinput, &setoutput, reinterpret_cast<int *>(&maxfd));
		}
	} else {
		/*
		 * This is needed if irccd is started before DHCP or if
		 * DNS cache is outdated.
		 *
		 * For more information see bug #190.
		 */
#if !defined(_WIN32)
		(void)res_init();
#endif
		log::info() << "server " << info.name << ": trying to connect to " << info.host << ", port " << info.port << std::endl;

		if (!connect(server)) {
			log::warning() << "server " << info.name << ": disconnected while connecting: "
					  << irc_strerror(irc_errno(server.session())) << std::endl;
			server.next(ServerState::Disconnected);
		} else {
			m_started = true;
		}
	}
}

void ServerState::prepareDisconnected(Server &server, fd_set &, fd_set &, net::Handle &)
{
	const ServerInfo &info = server.info();
	ServerSettings &settings = server.settings();

	/* if ServerSettings::recotries it set to -1, reconnection is completely disabled. */
	if (settings.recotries < 0) {
		log::warning() << "server " << info.name << ": reconnection disabled, skipping" << std::endl;
		server.onDie();
	} else if ((settings.recocurrent + 1) > settings.recotries) {
		log::warning() << "server " << info.name << ": giving up" << std::endl;
		server.onDie();
	} else {
		if (m_timer.elapsed() > static_cast<unsigned>(settings.recotimeout * 1000)) {
			irc_disconnect(server.session());

			settings.recocurrent ++;
			server.next(ServerState::Connecting);
		}
	}
}

ServerState::ServerState(Type type)
	: m_type(type)
{
	assert(static_cast<int>(m_type) >= static_cast<int>(ServerState::Undefined));
	assert(static_cast<int>(m_type) <= static_cast<int>(ServerState::Disconnected));
}

void ServerState::prepare(Server &server, fd_set &setinput, fd_set &setoutput, net::Handle &maxfd)
{
	switch (m_type) {
	case Connecting:
		prepareConnecting(server, setinput, setoutput, maxfd);
		break;
	case Connected:
		prepareConnected(server, setinput, setoutput, maxfd);
		break;
	case Disconnected:
		prepareDisconnected(server, setinput, setoutput, maxfd);
		break;
	default:
		break;
	}
}

} // !irccd
