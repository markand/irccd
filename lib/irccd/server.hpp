/*
 * server.hpp -- an IRC server
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

#ifndef IRCCD_SERVER_HPP
#define IRCCD_SERVER_HPP

/**
 * \file server.hpp
 * \brief IRC Server.
 */

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "elapsed-timer.hpp"
#include "server-state.hpp"
#include "signals.hpp"

namespace irccd {

/**
 * \brief Identity to use when connecting.
 */
class ServerIdentity {
public:
	std::string name{"irccd"};			//!< identity name
	std::string nickname{"irccd"};			//!< nickname to show
	std::string username{"irccd"};			//!< username to use for connection
	std::string realname{"IRC Client Daemon"};	//!< the full real name
	std::string ctcpversion{"IRC Client Daemon"};	//!< the CTCP version to define
};

/**
 * \brief A channel to join with an optional password.
 */
class ServerChannel {
public:
	std::string name;				//!< the channel to join
	std::string password;				//!< the optional password
};

/**
 * \brief Prefixes for nicknames.
 */
enum class ServerChanMode {
	Creator		= 'O',				//!< Channel creator
	HalfOperator	= 'h',				//!< Half operator
	Operator	= 'o',				//!< Channel operator
	Protection	= 'a',				//!< Unkillable
	Voiced		= 'v'				//!< Voice power
};

/**
 * \brief Describe a whois information.
 */
class ServerWhois {
public:
	std::string nick;				//!< user's nickname
	std::string user;				//!< user's user
	std::string host;				//!< hostname
	std::string realname;				//!< realname
	std::vector<std::string> channels;		//!< the channels where the user is
};

/**
 * \brief Server information
 *
 * This class contains everything needed to connect to a server.
 */
class ServerInfo {
public:
	enum {
		Ipv6		= (1 << 0),		//!< Connect using IPv6
		Ssl		= (1 << 1),		//!< Use SSL
		SslVerify	= (1 << 2)		//!< Verify SSL
	};

	std::string name;				//!< Server's name
	std::string host;				//!< Hostname
	std::string password;				//!< Optional server password
	std::uint16_t port{6667};			//!< Server's port
	std::uint8_t flags{0};				//!< Optional flags
	std::map<ServerChanMode, char> modes;		//!< IRC modes (e.g. @~)
};

/**
 * \brief Contains settings to tweak the server
 *
 * This class contains additional settings that tweaks the server operations.
 */
class ServerSettings {
public:
	enum {
		AutoRejoin	= (1 << 0),		//!< Auto rejoin a channel after being kicked
		JoinInvite	= (1 << 1)		//!< Join a channel on invitation
	};

	std::vector<ServerChannel> channels;		//!< List of channel to join
	std::string command{"!"};			//!< The command character to trigger plugin command
	std::int8_t reconnectTries{-1};			//!< Number of tries to reconnect before giving up
	std::uint16_t reconnectDelay{30};		//!< Number of seconds to wait before trying to connect
	std::uint8_t flags{0};				//!< Optional flags
	std::uint16_t pingTimeout{300};			//!< Time in seconds before ping timeout is announced
};

/**
 * \brief Some variables that are needed in many places internally.
 */
class ServerCache {
public:
	ElapsedTimer pingTimer;				//!< Track elapsed time for ping timeout.
	std::int8_t reconnectCurrent{1};		//!< Number of reconnection already tested.

	/**
	 * Map of names being build by channels.
	 */
	std::map<std::string, std::set<std::string>> namesMap;

	/**
	 * Map of whois being build by nicknames.
	 */
	std::map<std::string, ServerWhois> whoisMap;
};

/**
 * \brief The class that connect to a IRC server
 *
 * The server is a class that stores callbacks which will be called on IRC events. It is the lowest part of the
 * connection to a server, it can be used directly by the user to connect to a server.
 *
 * The server has several signals that will be emitted when data has arrived.
 *
 * When adding a server to the irccd instance using Irccd::addServer, these signals are connected to generate
 * events that will be dispatched to the plugins and to the transports.
 *
 * Note: the server is set in non blocking mode, commands are placed in a queue and sent when only when they are ready.
 */
class Server {
public:
	/**
	 * Bridge for libircclient.
	 */
	class Session;

	/**
	 * Signal: onChannelMode
	 * ----------------------------------------------------------
	 *
	 * Triggered when someone changed the channel mode.
	 *
	 * Arguments:
	 * - the origin,
	 * - the channel,
	 * - the mode,
	 * - the optional mode argument.
	 */
	Signal<std::string, std::string, std::string, std::string> onChannelMode;

	/**
	 * Signal: onChannelNotice
	 * ----------------------------------------------------------
	 *
	 * Triggered when a notice has been sent on a channel.
	 *
	 * Arguments:
	 *   - the origin (the nickname who has sent the notice),
	 *   - the channel name,
	 *   - the notice message.
	 */
	Signal<std::string, std::string, std::string> onChannelNotice;

	/**
	 * Signal: onConnect
	 * ----------------------------------------------------------
	 *
	 * Triggered when the server is successfully connected.
	 */
	Signal<> onConnect;

	/**
	 * Signal: onDie
	 * ----------------------------------------------------------
	 *
	 * The server is dead.
	 */
	Signal<> onDie;

	/**
	 * Signal: onInvite
	 * ----------------------------------------------------------
	 *
	 * Triggered when an invite has been sent to you (the bot).
	 *
	 * Arguments:
	 *   - the origin,
	 *   - the channel,
	 *   - your nickname.
	 */
	Signal<std::string, std::string, std::string> onInvite;

	/**
	 * Signal: onJoin
	 * ----------------------------------------------------------
	 *
	 * Triggered when a user has joined the channel, it also includes you.
	 *
	 * Arguments:
	 *   - the origin (may be you),
	 *   - the channel.
	 */
	Signal<std::string, std::string> onJoin;

	/**
	 * Signal: onKick
	 * ----------------------------------------------------------
	 *
	 * Triggered when someone has been kicked from a channel.
	 *
	 * Arguments:
	 *   - the origin,
	 *   - the channel,
	 *   - the target who has been kicked,
	 *   - the optional reason.
	 */
	Signal<std::string, std::string, std::string, std::string> onKick;

	/**
	 * ServerEvent: onMessage
	 * ----------------------------------------------------------
	 *
	 * Triggered when a message on a channel has been sent.
	 *
	 * Arguments:
	 *   - the origin,
	 *   - the channel,
	 *   - the message.
	 */
	Signal<std::string, std::string, std::string> onMessage;

	/**
	 * Signal: onMe
	 * ----------------------------------------------------------
	 *
	 * Triggered on a CTCP Action.
	 *
	 * This is both used in a channel and in a private message so the target may be a channel or your nickname.
	 *
	 * Arguments:
	 *   - the origin,
	 *   - the target,
	 *   - the message.
	 */
	Signal<std::string, std::string, std::string> onMe;

	/**
	 * Signal: onMode
	 * ----------------------------------------------------------
	 *
	 * Triggered when the server changed your user mode.
	 *
	 * Arguments:
	 *   - the origin,
	 *   - the mode (e.g +i).
	 */
	Signal<std::string, std::string> onMode;

	/**
	 * Signal: onNames
	 * ----------------------------------------------------------
	 *
	 * Triggered when names listing has finished on a channel.
	 *
	 * Arguments:
	 *   - the channel,
	 *   - the ordered list of names.
	 */
	Signal<std::string, std::set<std::string>> onNames;

	/**
	 * Signal: onNick
	 * ----------------------------------------------------------
	 *
	 * Triggered when someone changed its nickname, it also includes you.
	 *
	 * Arguments:
	 *   - the old nickname (may be you),
	 *   - the new nickname.
	 */
	Signal<std::string, std::string> onNick;

	/**
	 * Signal: onNotice
	 * ----------------------------------------------------------
	 *
	 * Triggered when someone has sent a notice to you.
	 *
	 * Arguments:
	 *   - the origin,
	 *   - the notice message.
	 */
	Signal<std::string, std::string> onNotice;

	/**
	 * Signal: onPart
	 * ----------------------------------------------------------
	 *
	 * Triggered when someone has left the channel.
	 *
	 * Arguments:
	 *   - the origin,
	 *   - the channel that the nickname has left,
	 *   - the optional reason.
	 */
	Signal<std::string, std::string, std::string> onPart;

	/**
	 * Signal: onQuery
	 * ----------------------------------------------------------
	 *
	 * Triggered when someone has sent you a private message.
	 *
	 * Arguments:
	 *   - the origin,
	 *   - the message.
	 */
	Signal<std::string, std::string> onQuery;

	/**
	 * Signal: onTopic
	 * ----------------------------------------------------------
	 *
	 * Triggered when someone changed the channel topic.
	 *
	 * Arguments:
	 *   - the origin,
	 *   - the channel,
	 *   - the new topic.
	 */
	Signal<std::string, std::string, std::string> onTopic;

	/**
	 * Signal: onWhois
	 * ----------------------------------------------------------
	 *
	 * Triggered when whois information has been received.
	 *
	 * Arguments:
	 *   - the whois object.
	 */
	Signal<ServerWhois> onWhois;

private:
	ServerInfo m_info;
	ServerSettings m_settings;
	ServerIdentity m_identity;
	ServerCache m_cache;

	/* queue of requests to send */
	std::queue<std::function<bool ()>> m_queue;

	/* libircclient session (bridge) */
	std::unique_ptr<Session> m_session;

	/* States */
	std::unique_ptr<ServerState> m_state;
	std::unique_ptr<ServerState> m_stateNext;

	/* Handle libircclient callbacks */
	void handleChannel(const char *, const char **) noexcept;
	void handleChannelMode(const char *, const char **) noexcept;
	void handleChannelNotice(const char *, const char **) noexcept;
	void handleConnect(const char *, const char **) noexcept;
	void handleCtcpAction(const char *, const char **) noexcept;
	void handleInvite(const char *, const char **) noexcept;
	void handleJoin(const char *, const char **) noexcept;
	void handleKick(const char *, const char **) noexcept;
	void handleMode(const char *, const char **) noexcept;
	void handleNick(const char *, const char **) noexcept;
	void handleNotice(const char *, const char **) noexcept;
	void handleNumeric(unsigned int, const char **, unsigned int) noexcept;
	void handlePart(const char *, const char **) noexcept;
	void handlePing(const char *, const char **) noexcept;
	void handleQuery(const char *, const char **) noexcept;
	void handleTopic(const char *, const char **) noexcept;

public:
	/**
	 * Split a channel from the form channel:password into a ServerChannel object.
	 *
	 * \param value the value
	 * \return a channel
	 */
	static ServerChannel splitChannel(const std::string &value);

	/**
	 * Construct a server.
	 *
	 * \param info the information
	 * \param identity the identity
	 * \param settings the settings
	 */
	Server(ServerInfo info, ServerIdentity identity = {}, ServerSettings settings = {});

	/**
	 * Destructor. Close the connection if needed.
	 */
	virtual ~Server();

	/**
	 * Get the server information.
	 *
	 * \return the server information
	 */
	inline const ServerInfo &info() const noexcept
	{
		return m_info;
	}

	/**
	 * Get the server settings.
	 *
	 * \note some settings will be used only after the next reconnection
	 * \return the settings
	 */
	inline ServerSettings &settings() noexcept
	{
		return m_settings;
	}

	/**
	 * Get the server settings.
	 *
	 * \return the settings
	 */
	inline const ServerSettings &settings() const noexcept
	{
		return m_settings;
	}

	/**
	 * Access the identity.
	 *
	 * \return the identity
	 */
	inline const ServerIdentity &identity() const noexcept
	{
		return m_identity;
	}

	/**
	 * Access the cache.
	 *
	 * \return the cache
	 * \warning use with care
	 */
	inline ServerCache &cache() noexcept
	{
		return m_cache;
	}

	/**
	 * Get the private session.
	 *
	 * \return the session
	 */
	inline Session &session() noexcept
	{
		return *m_session;
	}

	/**
	 * Set the next state, it is not changed immediately but on next iteration.
	 *
	 * \param state the new state
	 */
	inline void next(std::unique_ptr<ServerState> state) noexcept
	{
		m_stateNext = std::move(state);
	}

	/**
	 * Switch to next state if it has.
	 */
	void update() noexcept;

	/**
	 * Force disconnection.
	 */
	void disconnect() noexcept;

	/**
	 * Asks for a reconnection.
	 */
	void reconnect() noexcept;

	/**
	 * Prepare the IRC session.
	 *
	 * \warning Not thread-safe
	 */
	inline void prepare(fd_set &setinput, fd_set &setoutput, net::Handle &maxfd) noexcept
	{
		m_state->prepare(*this, setinput, setoutput, maxfd);
	}

	/**
	 * Process incoming/outgoing data after selection.
	 *
	 * \param setinput
	 * \param setoutput
	 * \throw any exception that have been throw from user functions
	 */
	void sync(fd_set &setinput, fd_set &setoutput);

	/**
	 * Determine if the nickname is the bot itself.
	 *
	 * \param nick the nickname to check
	 * \return true if it is the bot
	 */
	bool isSelf(const std::string &nick) const noexcept;

	/**
	 * Change the channel mode.
	 *
	 * \param channel the channel
	 * \param mode the new mode
	 */
	void cmode(std::string channel, std::string mode);

	/**
	 * Send a channel notice.
	 *
	 * \param channel the channel
	 * \param message message notice
	 */
	void cnotice(std::string channel, std::string message);

	/**
	 * Invite a user to a channel.
	 *
	 * \param target the target nickname
	 * \param channel the channel
	 */
	void invite(std::string target, std::string channel);

	/**
	 * Join a channel, the password is optional and can be kept empty.
	 *
	 * \param channel the channel to join
	 * \param password the optional password
	 */
	void join(std::string channel, std::string password = "");

	/**
	 * Kick someone from the channel. Please be sure to have the rights
	 * on that channel because errors won't be reported.
	 *
	 * \param target the target to kick
	 * \param channel from which channel
	 * \param reason the optional reason
	 */
	void kick(std::string target, std::string channel, std::string reason = "");

	/**
	 * Send a CTCP Action as known as /me. The target may be either a
	 * channel or a nickname.
	 *
	 * \param target the nickname or the channel
	 * \param message the message
	 */
	void me(std::string target, std::string message);

	/**
	 * Send a message to the specified target or channel.
	 *
	 * \param target the target
	 * \param message the message
	 */
	void message(std::string target, std::string message);

	/**
	 * Change your user mode.
	 *
	 * \param mode the mode
	 */
	void mode(std::string mode);

	/**
	 * Request the list of names.
	 *
	 * \param channel the channel
	 */
	void names(std::string channel);

	/**
	 * Change your nickname.
	 *
	 * \param newnick the new nickname to use
	 */
	void nick(std::string newnick);

	/**
	 * Send a private notice.
	 *
	 * \param target the target
	 * \param message the notice message
	 */
	void notice(std::string target, std::string message);

	/**
	 * Part from a channel.
	 *
	 * Please note that the reason is not supported on all servers so if you want portability, don't provide it.
	 *
	 * \param channel the channel to leave
	 * \param reason the optional reason
	 */
	void part(std::string channel, std::string reason = "");

	/**
	 * Send a raw message to the IRC server. You don't need to add
	 * message terminators.
	 *
	 * \warning Use this function with care
	 * \param raw the raw message (without `\r\n\r\n`)
	 */
	void send(std::string raw);

	/**
	 * Change the channel topic.
	 *
	 * \param channel the channel
	 * \param topic the desired topic
	 */
	void topic(std::string channel, std::string topic);

	/**
	 * Request for whois information.
	 *
	 * \param target the target nickname
	 */
	void whois(std::string target);
};

} // !irccd

#endif // !IRCCD_SERVER_HPP
