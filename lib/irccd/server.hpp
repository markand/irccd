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

#include <json.hpp>

#include "elapsed-timer.hpp"
#include "server-state.hpp"
#include "signals.hpp"
#include "sysconfig.hpp"

namespace irccd {

class Server;

/**
 * \brief Prefixes for nicknames.
 */
enum class ChannelMode {
    Creator         = 'O',                          //!< Channel creator
    HalfOperator    = 'h',                          //!< Half operator
    Operator        = 'o',                          //!< Channel operator
    Protection      = 'a',                          //!< Unkillable
    Voiced          = 'v'                           //!< Voice power
};

/**
 * \brief A channel to join with an optional password.
 */
class Channel {
public:
    std::string name;                               //!< the channel to join
    std::string password;                           //!< the optional password
};

/**
 * \brief Describe a whois information.
 */
class Whois {
public:
    std::string nick;                               //!< user's nickname
    std::string user;                               //!< user's user
    std::string host;                               //!< hostname
    std::string realname;                           //!< realname
    std::vector<std::string> channels;              //!< the channels where the user is
};

/**
 * \brief Some variables that are needed in many places internally.
 */
class Cache {
public:
    ElapsedTimer pingTimer;                         //!< Track elapsed time for ping timeout.
    std::int8_t reconnectCurrent{1};                //!< Number of reconnection already tested.

    /**
     * Map of names being build by channels.
     */
    std::map<std::string, std::set<std::string>> namesMap;

    /**
     * Map of whois being build by nicknames.
     */
    std::map<std::string, Whois> whoisMap;
};

/**
 * \brief Channel event.
 */
class ChannelModeEvent {
public:
    std::shared_ptr<Server> server;         //!< The server.
    std::string origin;                     //!< The originator.
    std::string channel;                    //!< The channel.
    std::string mode;                       //!< The mode.
    std::string argument;                   //!< The mode argument (Optional).
};

/**
 * \brief Channel notice event.
 */
class ChannelNoticeEvent {
public:
    std::shared_ptr<Server> server;         //!< The server.
    std::string origin;                     //!< The originator.
    std::string channel;                    //!< The channel.
    std::string message;                    //!< The notice message.
};

/**
 * \brief Connection success event.
 */
class ConnectEvent {
public:
    std::shared_ptr<Server> server;         //!< The server.
};

/**
 * \brief Invite event.
 */
class InviteEvent {
public:
    std::shared_ptr<Server> server;         //!< The server.
    std::string origin;                     //!< The originator.
    std::string channel;                    //!< The channel.
    std::string nickname;                   //!< The nickname (you).
};

/**
 * \brief Join event.
 */
class JoinEvent {
public:
    std::shared_ptr<Server> server;         //!< The server.
    std::string origin;                     //!< The originator.
    std::string channel;                    //!< The channel.
};

/**
 * \brief Kick event.
 */
class KickEvent {
public:
    std::shared_ptr<Server> server;         //!< The server.
    std::string origin;                     //!< The originator.
    std::string channel;                    //!< The channel.
    std::string target;                     //!< The target.
    std::string reason;                     //!< The reason (Optional).
};

/**
 * \brief Message event.
 */
class MessageEvent {
public:
    std::shared_ptr<Server> server;         //!< The server.
    std::string origin;                     //!< The originator.
    std::string channel;                    //!< The channel.
    std::string message;                    //!< The message.
};

/**
 * \brief CTCP action event.
 */
class MeEvent {
public:
    std::shared_ptr<Server> server;         //!< The server.
    std::string origin;                     //!< The originator.
    std::string channel;                    //!< The channel.
    std::string message;                    //!< The message.
};

/**
 * \brief Mode event.
 */
class ModeEvent {
public:
    std::shared_ptr<Server> server;         //!< The server.
    std::string origin;                     //!< The originator.
    std::string mode;                       //!< The mode.
};

/**
 * \brief Names listing event.
 */
class NamesEvent {
public:
    std::shared_ptr<Server> server;         //!< The server.
    std::string channel;                    //!< The channel.
    std::vector<std::string> names;         //!< The names.
};

/**
 * \brief Nick change event.
 */
class NickEvent {
public:
    std::shared_ptr<Server> server;         //!< The server.
    std::string origin;                     //!< The originator.
    std::string nickname;                   //!< The new nickname.
};

/**
 * \brief Notice event.
 */
class NoticeEvent {
public:
    std::shared_ptr<Server> server;         //!< The server.
    std::string origin;                     //!< The originator.
    std::string message;                    //!< The message.
};

/**
 * \brief Part event.
 */
class PartEvent {
public:
    std::shared_ptr<Server> server;         //!< The server.
    std::string origin;                     //!< The originator.
    std::string channel;                    //!< The channel.
    std::string reason;                     //!< The reason.
};

/**
 * \brief Query event.
 */
class QueryEvent {
public:
    std::shared_ptr<Server> server;         //!< The server.
    std::string origin;                     //!< The originator.
    std::string message;                    //!< The message.
};

/**
 * \brief Topic event.
 */
class TopicEvent {
public:
    std::shared_ptr<Server> server;         //!< The server.
    std::string origin;                     //!< The originator.
    std::string channel;                    //!< The channel.
    std::string topic;                      //!< The topic message.
};

/**
 * \brief Whois event.
 */
class WhoisEvent {
public:
    std::shared_ptr<Server> server;         //!< The server.
    Whois whois;                            //!< The whois information.
};

/**
 * \brief The class that connect to a IRC server
 *
 * The server is a class that stores callbacks which will be called on IRC events. It is the lowest part of the connection to a server, it
 * can be used directly by the user to connect to a server.
 *
 * The server has several signals that will be emitted when data has arrived.
 *
 * When adding a server to the ServerService in irccd, these signals are connected to generate events that will be dispatched to the plugins
 * and to the transports.
 *
 * Note: the server is set in non blocking mode, commands are placed in a queue and sent when only when they are ready.
 */
class Server : public std::enable_shared_from_this<Server> {
public:
    /**
     * Bridge for libircclient.
     */
    class Session;

    /**
     * \brief Various options for server.
     */
    enum {
        Ipv6        = (1 << 0),                     //!< Connect using IPv6
        Ssl         = (1 << 1),                     //!< Use SSL
        SslVerify   = (1 << 2),                     //!< Verify SSL
        AutoRejoin  = (1 << 3),                     //!< Auto rejoin a channel after being kicked
        JoinInvite  = (1 << 4)                      //!< Join a channel on invitation
    };

    /**
     * Signal: onChannelMode
     * ----------------------------------------------------------
     *
     * Triggered when someone changed the channel mode.
     */
    Signal<ChannelModeEvent> onChannelMode;

    /**
     * Signal: onChannelNotice
     * ----------------------------------------------------------
     *
     * Triggered when a notice has been sent on a channel.
     */
    Signal<ChannelNoticeEvent> onChannelNotice;

    /**
     * Signal: onConnect
     * ----------------------------------------------------------
     *
     * Triggered when the server is successfully connected.
     */
    Signal<ConnectEvent> onConnect;

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
     */
    Signal<InviteEvent> onInvite;

    /**
     * Signal: onJoin
     * ----------------------------------------------------------
     *
     * Triggered when a user has joined the channel, it also includes you.
     */
    Signal<JoinEvent> onJoin;

    /**
     * Signal: onKick
     * ----------------------------------------------------------
     *
     * Triggered when someone has been kicked from a channel.
     */
    Signal<KickEvent> onKick;

    /**
     * ServerEvent: onMessage
     * ----------------------------------------------------------
     *
     * Triggered when a message on a channel has been sent.
     */
    Signal<MessageEvent> onMessage;

    /**
     * Signal: onMe
     * ----------------------------------------------------------
     *
     * Triggered on a CTCP Action.
     *
     * This is both used in a channel and in a private message so the target may be a channel or your nickname.
     */
    Signal<MeEvent> onMe;

    /**
     * Signal: onMode
     * ----------------------------------------------------------
     *
     * Triggered when the server changed your user mode.
     */
    Signal<ModeEvent> onMode;

    /**
     * Signal: onNames
     * ----------------------------------------------------------
     *
     * Triggered when names listing has finished on a channel.
     */
    Signal<NamesEvent> onNames;

    /**
     * Signal: onNick
     * ----------------------------------------------------------
     *
     * Triggered when someone changed its nickname, it also includes you.
     */
    Signal<NickEvent> onNick;

    /**
     * Signal: onNotice
     * ----------------------------------------------------------
     *
     * Triggered when someone has sent a notice to you.
     */
    Signal<NoticeEvent> onNotice;

    /**
     * Signal: onPart
     * ----------------------------------------------------------
     *
     * Triggered when someone has left the channel.
     */
    Signal<PartEvent> onPart;

    /**
     * Signal: onQuery
     * ----------------------------------------------------------
     *
     * Triggered when someone has sent you a private message.
     */
    Signal<QueryEvent> onQuery;

    /**
     * Signal: onTopic
     * ----------------------------------------------------------
     *
     * Triggered when someone changed the channel topic.
     */
    Signal<TopicEvent> onTopic;

    /**
     * Signal: onWhois
     * ----------------------------------------------------------
     *
     * Triggered when whois information has been received.
     */
    Signal<WhoisEvent> onWhois;

private:
    // Misc.
    std::map<ChannelMode, char> m_modes;

    // Requested and joined channels.
    std::vector<Channel> m_rchannels;
    std::vector<std::string> m_jchannels;

    // Identifier.
    std::string m_name;

    // Connection information
    std::string m_host;
    std::string m_password;
    std::uint16_t m_port{6667};
    std::uint8_t m_flags{0};

    // Identity.
    std::string m_nickname{"irccd"};
    std::string m_username{"irccd"};
    std::string m_realname{"IRC Client Daemon"};
    std::string m_ctcpversion{"IRC Client Daemon"};

    // Settings.
    std::string m_commandCharacter{"!"};
    std::int8_t m_reconnectTries{-1};
    std::uint16_t m_reconnectDelay{30};
    std::uint16_t m_pingTimeout{300};

    // TODO: find another way.
    Cache m_cache;

    // Queue of requests to send.
    std::queue<std::function<bool ()>> m_queue;

    // libircclient session (bridge).
    std::unique_ptr<Session> m_session;

    // States.
    std::unique_ptr<State> m_state;
    std::unique_ptr<State> m_stateNext;

    // Private helpers.
    void removeJoinedChannel(const std::string &channel);

    // Handle libircclient callbacks.
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
     * Convert a JSON object as a server.
     *
     * Used in JavaScript API and transport commands.
     *
     * \param object the object
     * \return the server
     * \throw std::exception on failures
     */
    IRCCD_EXPORT static std::shared_ptr<Server> fromJson(const nlohmann::json &object);

    /**
     * Split a channel from the form channel:password into a ServerChannel object.
     *
     * \param value the value
     * \return a channel
     */
    IRCCD_EXPORT static Channel splitChannel(const std::string &value);

    /**
     * Construct a server.
     *
     * \param name the identifier
     */
    IRCCD_EXPORT Server(std::string name);

    /**
     * Destructor. Close the connection if needed.
     */
    IRCCD_EXPORT virtual ~Server();

    /**
     * Get the server identifier.
     *
     * \return the id
     */
    inline const std::string &name() const noexcept
    {
        return m_name;
    }

    /**
     * Get the hostname.
     *
     * \return the hostname
     */
    inline const std::string &host() const noexcept
    {
        return m_host;
    }

    /**
     * Set the hostname.
     *
     * \param host the hostname
     */
    inline void setHost(std::string host) noexcept
    {
        m_host = std::move(host);
    }

    /**
     * Get the password.
     *
     * \return the password
     */
    inline const std::string &password() const noexcept
    {
        return m_password;
    }

    /**
     * Set the password.
     *
     * An empty password means no password.
     *
     * \param password the password
     */
    inline void setPassword(std::string password) noexcept
    {
        m_password = std::move(password);
    }

    /**
     * Get the port.
     *
     * \return the port
     */
    inline std::uint16_t port() const noexcept
    {
        return m_port;
    }

    /**
     * Set the port.
     *
     * \param port the port
     */
    inline void setPort(std::uint16_t port) noexcept
    {
        m_port = port;
    }

    /**
     * Get the flags.
     *
     * \return the flags
     */
    inline std::uint8_t flags() const noexcept
    {
        return m_flags;
    }

    /**
     * Set the flags.
     *
     * \pre flags must be valid
     * \param flags the flags
     */
    inline void setFlags(std::uint8_t flags) noexcept
    {
        assert(flags <= 0x1f);

        m_flags = flags;
    }

    /**
     * Get the nickname.
     *
     * \return the nickname
     */
    inline const std::string &nickname() const noexcept
    {
        return m_nickname;
    }

    /**
     * Set the nickname.
     *
     * If the server is connected, send a nickname command to the IRC server, otherwise change it locally.
     *
     * \param nickname the nickname
     */
    IRCCD_EXPORT void setNickname(std::string nickname);

    /**
     * Get the username.
     *
     * \return the username
     */
    inline const std::string &username() const noexcept
    {
        return m_username;
    }

    /**
     * Set the username.
     *
     * \param name the username
     * \note the username will be changed on the next connection
     */
    inline void setUsername(std::string name) noexcept
    {
        m_username = std::move(name);
    }

    /**
     * Get the realname.
     *
     * \return the realname
     */
    inline const std::string &realname() const noexcept
    {
        return m_realname;
    }

    /**
     * Set the realname.
     *
     * \param name the username
     * \note the username will be changed on the next connection
     */
    inline void setRealname(std::string realname) noexcept
    {
        m_realname = std::move(realname);
    }

    /**
     * Get the CTCP version.
     *
     * \return the CTCP version
     */
    inline const std::string &ctcpVersion() const noexcept
    {
        return m_ctcpversion;
    }

    /**
     * Set the CTCP version.
     *
     * \param ctcpversion the version
     */
    IRCCD_EXPORT void setCtcpVersion(std::string ctcpversion);

    /**
     * Get the command character.
     *
     * \return the character
     */
    inline const std::string &commandCharacter() const noexcept
    {
        return m_commandCharacter;
    }

    /**
     * Set the command character.
     *
     * \pre !commandCharacter.empty()
     * \param commandCharacter the command character
     */
    inline void setCommandCharacter(std::string commandCharacter) noexcept
    {
        assert(!commandCharacter.empty());

        m_commandCharacter = std::move(commandCharacter);
    }

    /**
     * Get the number of reconnections before giving up.
     *
     * \return the number of reconnections
     */
    inline std::int8_t reconnectTries() const noexcept
    {
        return m_reconnectTries;
    }

    /**
     * Set the number of reconnections to test before giving up.
     *
     * A value less than 0 means infinite.
     *
     * \param reconnectTries the number of reconnections
     */
    inline void setReconnectTries(std::int8_t reconnectTries) noexcept
    {
        m_reconnectTries = reconnectTries;
    }

    /**
     * Get the reconnection delay before retrying.
     *
     * \return the number of seconds
     */
    inline std::uint16_t reconnectDelay() const noexcept
    {
        return m_reconnectDelay;
    }

    /**
     * Set the number of seconds before retrying.
     *
     * \param reconnectDelay the number of seconds
     */
    inline void setReconnectDelay(std::uint16_t reconnectDelay) noexcept
    {
        m_reconnectDelay = reconnectDelay;
    }

    /**
     * Get the ping timeout.
     *
     * \return the ping timeout
     */
    inline std::uint16_t pingTimeout() const noexcept
    {
        return m_pingTimeout;
    }

    /**
     * Set the ping timeout before considering a server as dead.
     *
     * \param pingTimeout the delay in seconds
     */
    inline void setPingTimeout(std::uint16_t pingTimeout) noexcept
    {
        m_pingTimeout = pingTimeout;
    }

    /**
     * Get the list of channels joined.
     *
     * \return the channels
     */
    inline const std::vector<std::string> &channels() const noexcept
    {
        return m_jchannels;
    }

    /**
     * Access the cache.
     *
     * \return the cache
     * \warning use with care
     */
    inline Cache &cache() noexcept
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
    inline void next(std::unique_ptr<State> state) noexcept
    {
        m_stateNext = std::move(state);
    }

    /**
     * Switch to next state if it has.
     */
    IRCCD_EXPORT void update() noexcept;

    /**
     * Force disconnection.
     */
    IRCCD_EXPORT void disconnect() noexcept;

    /**
     * Asks for a reconnection.
     */
    IRCCD_EXPORT void reconnect() noexcept;

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
    IRCCD_EXPORT void sync(fd_set &setinput, fd_set &setoutput);

    /**
     * Determine if the nickname is the bot itself.
     *
     * \param nick the nickname to check
     * \return true if it is the bot
     */
    IRCCD_EXPORT bool isSelf(const std::string &nick) const noexcept;

    /**
     * Change the channel mode.
     *
     * \param channel the channel
     * \param mode the new mode
     */
    IRCCD_EXPORT virtual void cmode(std::string channel, std::string mode);

    /**
     * Send a channel notice.
     *
     * \param channel the channel
     * \param message message notice
     */
    IRCCD_EXPORT virtual void cnotice(std::string channel, std::string message);

    /**
     * Invite a user to a channel.
     *
     * \param target the target nickname
     * \param channel the channel
     */
    IRCCD_EXPORT virtual void invite(std::string target, std::string channel);

    /**
     * Join a channel, the password is optional and can be kept empty.
     *
     * \param channel the channel to join
     * \param password the optional password
     */
    IRCCD_EXPORT virtual void join(std::string channel, std::string password = "");

    /**
     * Kick someone from the channel. Please be sure to have the rights
     * on that channel because errors won't be reported.
     *
     * \param target the target to kick
     * \param channel from which channel
     * \param reason the optional reason
     */
    IRCCD_EXPORT virtual void kick(std::string target, std::string channel, std::string reason = "");

    /**
     * Send a CTCP Action as known as /me. The target may be either a
     * channel or a nickname.
     *
     * \param target the nickname or the channel
     * \param message the message
     */
    IRCCD_EXPORT virtual void me(std::string target, std::string message);

    /**
     * Send a message to the specified target or channel.
     *
     * \param target the target
     * \param message the message
     */
    IRCCD_EXPORT virtual void message(std::string target, std::string message);

    /**
     * Change your user mode.
     *
     * \param mode the mode
     */
    IRCCD_EXPORT virtual void mode(std::string mode);

    /**
     * Request the list of names.
     *
     * \param channel the channel
     */
    IRCCD_EXPORT virtual void names(std::string channel);

    /**
     * Send a private notice.
     *
     * \param target the target
     * \param message the notice message
     */
    IRCCD_EXPORT virtual void notice(std::string target, std::string message);

    /**
     * Part from a channel.
     *
     * Please note that the reason is not supported on all servers so if you want portability, don't provide it.
     *
     * \param channel the channel to leave
     * \param reason the optional reason
     */
    IRCCD_EXPORT virtual void part(std::string channel, std::string reason = "");

    /**
     * Send a raw message to the IRC server. You don't need to add
     * message terminators.
     *
     * \warning Use this function with care
     * \param raw the raw message (without `\r\n\r\n`)
     */
    IRCCD_EXPORT virtual void send(std::string raw);

    /**
     * Change the channel topic.
     *
     * \param channel the channel
     * \param topic the desired topic
     */
    IRCCD_EXPORT virtual void topic(std::string channel, std::string topic);

    /**
     * Request for whois information.
     *
     * \param target the target nickname
     */
    IRCCD_EXPORT virtual void whois(std::string target);
};

} // !irccd

#endif // !IRCCD_SERVER_HPP
