/*
 * server.cpp -- an IRC server
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

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <stdexcept>

#include <libirc_rfcnumeric.h>

#include <logger.h>
#include <util.h>

#include "server.h"

#if defined(WITH_JS)
#  include "js.h"
#endif

namespace irccd {

bool Server::isSelf(const std::string &nick) const noexcept
{
	char target[32]{0};

	irc_target_get_nick(nick.c_str(), target, sizeof (target));

	return m_identity.nickname == target;
}

void Server::extractPrefixes(const std::string &line)
{
	std::pair<char, char> table[16];
	std::string buf = line.substr(7);

	for (int i = 0; i < 16; ++i)
		table[i] = std::make_pair(-1, -1);

	int j = 0;
	bool readModes = true;
	for (size_t i = 0; i < buf.size(); ++i) {
		if (buf[i] == '(')
			continue;
		if (buf[i] == ')') {
			j = 0;
			readModes = false;
			continue;
		}

		if (readModes)
			table[j++].first = buf[i];
		else
			table[j++].second = buf[i];
	}

	// Put these as a map of mode to prefix
	for (int i = 0; i < 16; ++i) {
		auto key = static_cast<ServerChanMode>(table[i].first);
		auto value = table[i].second;

		m_info.modes.emplace(key, value);
	}
}

std::string Server::cleanPrefix(std::string nickname) const noexcept
{
	if (nickname.length() > 0)
		for (const auto &pair : m_info.modes)
			if (nickname[0] == pair.second)
				nickname.erase(0, 1);

	return nickname;
}

void Server::handleConnect(const char *, const char **) noexcept
{
	/* Reset the number of tried reconnection. */
	m_settings.recocurrent = 0;

	/* Don't forget to change state and notify. */
	next(ServerState::Connected);
	onConnect();

	/* Auto join listed channels. */
	for (const ServerChannel &channel : m_settings.channels) {
		log::info() << "server " << m_info.name << ": auto joining " << channel.name << std::endl;
		join(channel.name, channel.password);
	}
}

void Server::handleChannel(const char *orig, const char **params) noexcept
{
	onMessage(strify(orig), strify(params[0]), strify(params[1]));
}

void Server::handleChannelMode(const char *orig, const char **params) noexcept
{
	onChannelMode(strify(orig), strify(params[0]), strify(params[1]), strify(params[2]));
}

void Server::handleChannelNotice(const char *orig, const char **params) noexcept
{
	onChannelNotice(strify(orig), strify(params[0]), strify(params[1]));
}

void Server::handleCtcpAction(const char *orig, const char **params) noexcept
{
	onMe(strify(orig), strify(params[0]), strify(params[1]));
}

void Server::handleInvite(const char *orig, const char **params) noexcept
{
	/* If joininvite is set, join the channel */
	if ((m_settings.flags & ServerSettings::JoinInvite) && isSelf(strify(params[0])))
		join(strify(params[1]));

	/*
	 * The libircclient says that invite contains the target nickname, it's quite
	 * uncommon to need it so it is passed as the last argument to be
	 * optional in the plugin.
	 */
	onInvite(strify(orig), strify(params[1]), strify(params[0]));
}

void Server::handleJoin(const char *orig, const char **params) noexcept
{
	onJoin(strify(orig), strify(params[0]));
}

void Server::handleKick(const char *orig, const char **params) noexcept
{
	/* Rejoin the channel if the option has been set and I was kicked. */
	if ((m_settings.flags & ServerSettings::AutoRejoin) && isSelf(strify(params[1])))
		join(strify(params[0]));

	onKick(strify(orig), strify(params[0]), strify(params[1]), strify(params[2]));
}

void Server::handleMode(const char *orig, const char **params) noexcept
{
	onMode(strify(orig), strify(params[1]));
}

void Server::handleNick(const char *orig, const char **params) noexcept
{
	/* Update our nickname. */
	if (isSelf(strify(orig)))
		m_identity.nickname = strify(params[0]);

	onNick(strify(orig), strify(params[0]));
}

void Server::handleNotice(const char *orig, const char **params) noexcept
{
	/*
	 * As for handleInvite, the notice provides the target nickname, we discard it.
	 */
	onNotice(strify(orig), strify(params[1]));
}

void Server::handleNumeric(unsigned int event, const char **params, unsigned int c) noexcept
{
	if (event == LIBIRC_RFC_RPL_NAMREPLY) {
		/*
		 * Called multiple times to list clients on a channel.
		 *
		 * params[0] == originator
		 * params[1] == '='
		 * params[2] == channel
		 * params[3] == list of users with their prefixes
		 *
		 * IDEA for the future: maybe give the appropriate mode as a second parameter in onNames.
		 */
		if (c < 4 || params[2] == nullptr || params[3] == nullptr)
			return;

		std::vector<std::string> users = util::split(params[3], " \t");

		/* The listing may add some prefixes, remove them if needed */
		for (std::string u : users)
			m_namesMap[params[2]].insert(cleanPrefix(u));
	} else if (event == LIBIRC_RFC_RPL_ENDOFNAMES) {
		/*
		 * Called when end of name listing has finished on a channel.
		 *
		 * params[0] == originator
		 * params[1] == channel
		 * params[2] == End of NAMES list
		 */

		if (c < 3 || params[1] == nullptr)
			return;

		auto it = m_namesMap.find(params[1]);
		if (it != m_namesMap.end()) {
			onNames(params[1], it->second);

			/* Don't forget to remove the list */
			m_namesMap.erase(it);
		}
	} else if (event == LIBIRC_RFC_RPL_WHOISUSER) {
		/*
		 * Called when whois information has been partially received.
		 *
		 * params[0] == originator
		 * params[1] == nickname
		 * params[2] == username
		 * params[3] == host
		 * params[4] == * (no idea what is that)
		 * params[5] == realname
		 */
		if (c < 6 || !params[1] || !params[2] || !params[3] || !params[5])
			return;

		ServerWhois info;

		info.nick = strify(params[1]);
		info.user = strify(params[2]);
		info.host = strify(params[3]);
		info.realname = strify(params[5]);

		m_whoisMap.emplace(info.nick, info);
	} else if (event == LIBIRC_RFC_RPL_WHOISCHANNELS) {
		/*
		 * Called when we have received channels for one user.
		 *
		 * params[0] == originator
		 * params[1] == nickname
		 * params[2] == list of channels with their prefixes
		 */
		if (c < 3 || !params[1] || !params[2])
			return;

		auto it = m_whoisMap.find(params[1]);
		if (it != m_whoisMap.end()) {
			std::vector<std::string> channels = util::split(params[2], " \t");

			/* Clean their prefixes */
			for (auto &s : channels)
				s = cleanPrefix(s);

			/* Insert */
			it->second.channels = std::move(channels);
		}
	} else if (event == LIBIRC_RFC_RPL_ENDOFWHOIS) {
		/*
		 * Called when whois is finished.
		 *
		 * params[0] == originator
		 * params[1] == nickname
		 * params[2] == End of WHOIS list
		 */

		auto it = m_whoisMap.find(params[1]);
		if (it != m_whoisMap.end()) {
			onWhois(it->second);

			/* Don't forget to remove */
			m_whoisMap.erase(it);
		}
	} else if (event == /* RPL_BOUNCE */ 5) {
		/*
		 * The event 5 is usually RPL_BOUNCE, but we always see it as ISUPPORT.
		 */
		for (unsigned int i = 0; i < c; ++i) {
			if (strncmp(params[i], "PREFIX", 6) == 0) {
				extractPrefixes(params[i]);
				break;
			}
		}
	}
}

void Server::handlePart(const char *orig, const char **params) noexcept
{
	onPart(strify(orig), strify(params[0]), strify(params[1]));
}

void Server::handleQuery(const char *orig, const char **params) noexcept
{
	onQuery(strify(orig), strify(params[1]));
}

void Server::handleTopic(const char *orig, const char **params) noexcept
{
	onTopic(strify(orig), strify(params[0]), strify(params[1]));
}

ServerChannel Server::splitChannel(const std::string &value)
{
	auto pos = value.find(':');

	if (pos != std::string::npos)
		return ServerChannel{value.substr(0, pos), value.substr(pos + 1)};

	return ServerChannel{value, ""};
}

Server::Server(ServerInfo info, ServerIdentity identity, ServerSettings settings)
	: m_info(std::move(info))
	, m_settings(std::move(settings))
	, m_identity(std::move(identity))
	, m_session(nullptr, nullptr)
	, m_state(ServerState::Connecting)
	, m_next(ServerState::Undefined)
{
	irc_callbacks_t callbacks;

	/*
	 * GCC 4.9.2 triggers some missing-field-initializers warnings when
	 * using uniform initialization so use a std::memset as a workaround.
	 */
	std::memset(&callbacks, 0, sizeof (irc_callbacks_t));

	/*
	 * Convert the raw pointer functions from libircclient to Server member
	 * function.
	 *
	 * While doing this, discard useless arguments.
	 */
	callbacks.event_channel = [] (irc_session_t *session, const char *, const char *orig, const char **params, unsigned) {
		static_cast<Server *>(irc_get_ctx(session))->handleChannel(orig, params);
	};
	callbacks.event_channel_notice = [] (irc_session_t *session, const char *, const char *orig, const char **params, unsigned) {
		static_cast<Server *>(irc_get_ctx(session))->handleChannelNotice(orig, params);
	};
	callbacks.event_connect = [] (irc_session_t *session, const char *, const char *orig, const char **params, unsigned) {
		static_cast<Server *>(irc_get_ctx(session))->handleConnect(orig, params);
	};
	callbacks.event_ctcp_action = [] (irc_session_t *session, const char *, const char *orig, const char **params, unsigned) {
		static_cast<Server *>(irc_get_ctx(session))->handleCtcpAction(orig, params);
	};
	callbacks.event_invite = [] (irc_session_t *session, const char *, const char *orig, const char **params, unsigned) {
		static_cast<Server *>(irc_get_ctx(session))->handleInvite(orig, params);
	};
	callbacks.event_join = [] (irc_session_t *session, const char *, const char *orig, const char **params, unsigned) {
		static_cast<Server *>(irc_get_ctx(session))->handleJoin(orig, params);
	};
	callbacks.event_kick = [] (irc_session_t *session, const char *, const char *orig, const char **params, unsigned) {
		static_cast<Server *>(irc_get_ctx(session))->handleKick(orig, params);
	};
	callbacks.event_mode = [] (irc_session_t *session, const char *, const char *orig, const char **params, unsigned) {
		static_cast<Server *>(irc_get_ctx(session))->handleChannelMode(orig, params);
	};
	callbacks.event_nick = [] (irc_session_t *session, const char *, const char *orig, const char **params, unsigned) {
		static_cast<Server *>(irc_get_ctx(session))->handleNick(orig, params);
	};
	callbacks.event_notice = [] (irc_session_t *session, const char *, const char *orig, const char **params, unsigned) {
		static_cast<Server *>(irc_get_ctx(session))->handleNotice(orig, params);
	};
	callbacks.event_numeric = [] (irc_session_t *session, unsigned int event, const char *, const char **params, unsigned int count) {
		static_cast<Server *>(irc_get_ctx(session))->handleNumeric(event, params, count);
	};
	callbacks.event_part = [] (irc_session_t *session, const char *, const char *orig, const char **params, unsigned) {
		static_cast<Server *>(irc_get_ctx(session))->handlePart(orig, params);
	};
	callbacks.event_privmsg = [] (irc_session_t *session, const char *, const char *orig, const char **params, unsigned) {
		static_cast<Server *>(irc_get_ctx(session))->handleQuery(orig, params);
	};
	callbacks.event_topic = [] (irc_session_t *session, const char *, const char *orig, const char **params, unsigned) {
		static_cast<Server *>(irc_get_ctx(session))->handleTopic(orig, params);
	};
	callbacks.event_umode = [] (irc_session_t *session, const char *, const char *orig, const char **params, unsigned) {
		static_cast<Server *>(irc_get_ctx(session))->handleMode(orig, params);
	};

	m_session = Session{irc_create_session(&callbacks), irc_destroy_session};

	/* Save this to the session */
	irc_set_ctx(m_session.get(), this);
	irc_set_ctcp_version(m_session.get(), m_identity.ctcpversion.c_str());
}

Server::~Server()
{
	irc_disconnect(m_session.get());
}

void Server::update() noexcept
{
	if (m_next.type() != ServerState::Undefined) {
		log::debug() << "server " << m_info.name << ": switching to state ";

		switch (m_next.type()) {
		case ServerState::Connecting:
			log::debug() << "\"Connecting\"" << std::endl;
			break;
		case ServerState::Connected:
			log::debug() << "\"Connected\"" << std::endl;
			break;
		case ServerState::Disconnected:
			log::debug() << "\"Disconnected\"" << std::endl;
			break;
		default:
			break;
		}

		m_state = std::move(m_next);
		m_next = ServerState::Undefined;
	}
}

void Server::sync(fd_set &setinput, fd_set &setoutput) noexcept
{
	/*
	 * 1. Send maximum of command possible if available for write */
	/*
	 * Break on the first failure to avoid changing the order of the
	 * commands if any of them fails.
	 */
	bool done = false;

	while (!m_queue.empty() && !done) {
		if (m_queue.front()())
			m_queue.pop();
		else
			done = true;
	}

	/* 2. Read data */
	irc_process_select_descriptors(m_session.get(), &setinput, &setoutput);
}

#if defined(WITH_JS)

void Server::prototype(js::Context &ctx)
{
	ctx.getGlobal<void>("\xff""\xff""Server-proto");
}

#endif

} // !irccd
