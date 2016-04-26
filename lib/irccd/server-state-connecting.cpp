/*
 * server-state-connecting.cpp -- connecting state
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

#include "server-state-connecting.hpp"
#include "server-state-connected.hpp"
#include "server-state-disconnected.hpp"
#include "server-private.hpp"
#include "sysconfig.hpp"

#if !defined(IRCCD_SYSTEM_WINDOWS)
#  include <sys/types.h>
#  include <netinet/in.h>
#  include <arpa/nameser.h>
#  include <resolv.h>
#endif

namespace irccd {

namespace state {

namespace {

bool connect(Server &server)
{
	const ServerInfo &info = server.info();
	const ServerIdentity &identity = server.identity();
	const char *password = info.password.empty() ? nullptr : info.password.c_str();
	std::string host = info.host;
	int code;

	/* libircclient requires # for SSL connection */
#if defined(WITH_SSL)
	if (info.flags & ServerInfo::Ssl)
		host.insert(0, 1, '#');
	if (!(info.flags & ServerInfo::SslVerify))
		irc_option_set(server.session(), LIBIRC_OPTION_SSL_NO_VERIFY);
#endif

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

} // !namespace

void Connecting::prepare(Server &server, fd_set &setinput, fd_set &setoutput, net::Handle &maxfd)
{
	/*
	 * The connect function will either fail if the hostname wasn't resolved
	 * or if any of the internal functions fail.
	 *
	 * It returns success if the connection was successful but it does not
	 * mean that connection is established.
	 *
	 * Because this function will be called repeatidly, the connection was started and we're still not
	 * connected in the specified timeout time, we mark the server as disconnected.
	 *
	 * Otherwise, the libircclient event_connect will change the state.
	 */
	const ServerInfo &info = server.info();

	if (m_started) {
		const ServerSettings &settings = server.settings();

		if (m_timer.elapsed() > static_cast<unsigned>(settings.reconnect_timeout * 1000)) {
			log::warning() << "server " << info.name << ": timeout while connecting" << std::endl;
			server.next(std::make_unique<state::Disconnected>());
		} else if (!irc_is_connected(server.session())) {
			log::warning() << "server " << info.name << ": error while connecting: ";
			log::warning() << irc_strerror(irc_errno(server.session())) << std::endl;

			if (settings.reconnect_tries != 0)
				log::warning() << "server " << info.name << ": retrying in "
					       << settings.reconnect_timeout << " seconds" << std::endl;

			server.next(std::make_unique<state::Disconnected>());
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
#if !defined(IRCCD_SYSTEM_WINDOWS)
		(void)res_init();
#endif
		log::info() << "server " << info.name << ": trying to connect to " << info.host << ", port " << info.port << std::endl;

		if (!connect(server)) {
			log::warning() << "server " << info.name << ": disconnected while connecting: ";
			log::warning() << irc_strerror(irc_errno(server.session())) << std::endl;
			server.next(std::make_unique<state::Disconnected>());
		} else {
			m_started = true;
		}
	}
}

std::string Connecting::ident() const
{
	return "Connecting";
}

} // !state

} // !irccd