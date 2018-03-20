/*
 * server.hpp -- an IRC server
 *
 * Copyright (c) 2013-2018 David Demelier <markand@malikania.fr>
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

#ifndef IRCCD_DAEMON_SERVER_HPP
#define IRCCD_DAEMON_SERVER_HPP

/**
 * \file server.hpp
 * \brief IRC Server.
 */

#include <irccd/sysconfig.hpp>

#include <cstdint>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include <boost/signals2/signal.hpp>

#include <json.hpp>

#include "irc.hpp"

namespace irccd {

class server;

/**
 * \brief Prefixes for nicknames.
 */
enum class channel_mode {
    creator         = 'O',                  //!< Channel creator
    half_op         = 'h',                  //!< Half operator
    op              = 'o',                  //!< Channel operator
    protection      = 'a',                  //!< Unkillable
    voiced          = 'v'                   //!< Voice power
};

/**
 * \brief A channel to join with an optional password.
 */
class channel {
public:
    std::string name;                       //!< the channel to join
    std::string password;                   //!< the optional password
};

/**
 * \brief Describe a whois information.
 */
class whois {
public:
    std::string nick;                       //!< user's nickname
    std::string user;                       //!< user's user
    std::string host;                       //!< hostname
    std::string realname;                   //!< realname
    std::vector<std::string> channels;      //!< the channels where the user is
};

/**
 * \brief Connection success event.
 */
class connect_event {
public:
    std::shared_ptr<class server> server;   //!< The server.
};

/**
 * \brief Connection success event.
 */
class disconnect_event {
public:
    std::shared_ptr<class server> server;   //!< The server.
};

/**
 * \brief Invite event.
 */
class invite_event {
public:
    std::shared_ptr<class server> server;   //!< The server.
    std::string origin;                     //!< The originator.
    std::string channel;                    //!< The channel.
    std::string nickname;                   //!< The nickname (you).
};

/**
 * \brief Join event.
 */
class join_event {
public:
    std::shared_ptr<class server> server;   //!< The server.
    std::string origin;                     //!< The originator.
    std::string channel;                    //!< The channel.
};

/**
 * \brief Kick event.
 */
class kick_event {
public:
    std::shared_ptr<class server> server;   //!< The server.
    std::string origin;                     //!< The originator.
    std::string channel;                    //!< The channel.
    std::string target;                     //!< The target.
    std::string reason;                     //!< The reason (Optional).
};

/**
 * \brief Message event.
 */
class message_event {
public:
    std::shared_ptr<class server> server;   //!< The server.
    std::string origin;                     //!< The originator.
    std::string channel;                    //!< The channel.
    std::string message;                    //!< The message.
};

/**
 * \brief CTCP action event.
 */
class me_event {
public:
    std::shared_ptr<class server> server;   //!< The server.
    std::string origin;                     //!< The originator.
    std::string channel;                    //!< The channel.
    std::string message;                    //!< The message.
};

/**
 * \brief Mode event.
 */
class mode_event {
public:
    std::shared_ptr<class server> server;   //!< The server.
    std::string origin;                     //!< The originator.
    std::string channel;                    //!< The channel or target.
    std::string mode;                       //!< The mode.
    std::string limit;                      //!< The optional limit.
    std::string user;                       //!< The optional user.
    std::string mask;                       //!< The optional ban mask.
};

/**
 * \brief Names listing event.
 */
class names_event {
public:
    std::shared_ptr<class server> server;   //!< The server.
    std::string channel;                    //!< The channel.
    std::vector<std::string> names;         //!< The names.
};

/**
 * \brief Nick change event.
 */
class nick_event {
public:
    std::shared_ptr<class server> server;   //!< The server.
    std::string origin;                     //!< The originator.
    std::string nickname;                   //!< The new nickname.
};

/**
 * \brief Notice event.
 */
class notice_event {
public:
    std::shared_ptr<class server> server;   //!< The server.
    std::string origin;                     //!< The originator.
    std::string channel;                    //!< The channel or target.
    std::string message;                    //!< The message.
};

/**
 * \brief Part event.
 */
class part_event {
public:
    std::shared_ptr<class server> server;   //!< The server.
    std::string origin;                     //!< The originator.
    std::string channel;                    //!< The channel.
    std::string reason;                     //!< The reason.
};

/**
 * \brief Query event.
 */
class query_event {
public:
    std::shared_ptr<class server> server;   //!< The server.
    std::string origin;                     //!< The originator.
    std::string message;                    //!< The message.
};

/**
 * \brief Topic event.
 */
class topic_event {
public:
    std::shared_ptr<class server> server;   //!< The server.
    std::string origin;                     //!< The originator.
    std::string channel;                    //!< The channel.
    std::string topic;                      //!< The topic message.
};

/**
 * \brief Whois event.
 */
class whois_event {
public:
    std::shared_ptr<class server> server;   //!< The server.
    class whois whois;                            //!< The whois information.
};

/**
 * \brief The class that connect to a IRC server
 *
 * The server is a class that stores callbacks which will be called on IRC
 * events. It is the lowest part of the connection to a server, it can be used
 * directly by the user to connect to a server.
 *
 * The server has several signals that will be emitted when data has arrived.
 *
 * When adding a server to the ServerService in irccd, these signals are
 * connected to generate events that will be dispatched to the plugins and to
 * the transports.
 *
 * Note: the server is set in non blocking mode, commands are placed in a queue
 * and sent when only when they are ready.
 */
class server : public std::enable_shared_from_this<server> {
public:
    /**
     * \brief Various options for server.
     */
    enum {
        ipv6        = (1 << 0),             //!< Connect using IPv6
        ssl         = (1 << 1),             //!< Use SSL
        ssl_verify  = (1 << 2),             //!< Verify SSL
        auto_rejoin = (1 << 3),             //!< Auto rejoin a kick
        join_invite = (1 << 4)              //!< Join a channel on invitation
    };

    /**
     * \brief Describe current server state.
     */
    enum class state_t {
        disconnected,       //!< not connected at all,
        connecting,         //!< network connection in progress,
        identifying,        //!< sending nick, user and password commands,
        waiting,            //!< waiting for reconnection,
        connected           //!< ready for use.
    };

    /**
     * Signal: on_connect
     * ----------------------------------------------------------
     *
     * Triggered when the server is successfully connected.
     */
    boost::signals2::signal<void (connect_event)> on_connect;

    /**
     * Signal: on_die
     * ----------------------------------------------------------
     *
     * The server is dead.
     */
    boost::signals2::signal<void (disconnect_event)> on_die;

    /**
     * Signal: on_invite
     * ----------------------------------------------------------
     *
     * Triggered when an invite has been sent to you (the bot).
     */
    boost::signals2::signal<void (invite_event)> on_invite;

    /**
     * Signal: on_join
     * ----------------------------------------------------------
     *
     * Triggered when a user has joined the channel, it also includes you.
     */
    boost::signals2::signal<void (join_event)> on_join;

    /**
     * Signal: on_kick
     * ----------------------------------------------------------
     *
     * Triggered when someone has been kicked from a channel.
     */
    boost::signals2::signal<void (kick_event)> on_kick;

    /**
     * Signal: on_message
     * ----------------------------------------------------------
     *
     * Triggered when a message on a channel has been sent.
     */
    boost::signals2::signal<void (message_event)> on_message;

    /**
     * Signal: on_me
     * ----------------------------------------------------------
     *
     * Triggered on a CTCP Action.
     *
     * This is both used in a channel and in a private message so the target may
     * be a channel or your nickname.
     */
    boost::signals2::signal<void (me_event)> on_me;

    /**
     * Signal: on_mode
     * ----------------------------------------------------------
     *
     * Triggered when the server changed your user mode.
     */
    boost::signals2::signal<void (mode_event)> on_mode;

    /**
     * Signal: on_names
     * ----------------------------------------------------------
     *
     * Triggered when names listing has finished on a channel.
     */
    boost::signals2::signal<void (names_event)> on_names;

    /**
     * Signal: on_nick
     * ----------------------------------------------------------
     *
     * Triggered when someone changed its nickname, it also includes you.
     */
    boost::signals2::signal<void (nick_event)> on_nick;

    /**
     * Signal: on_notice
     * ----------------------------------------------------------
     *
     * Triggered when someone has sent a notice to you.
     */
    boost::signals2::signal<void (notice_event)> on_notice;

    /**
     * Signal: on_part
     * ----------------------------------------------------------
     *
     * Triggered when someone has left the channel.
     */
    boost::signals2::signal<void (part_event)> on_part;

    /**
     * Signal: on_query
     * ----------------------------------------------------------
     *
     * Triggered when someone has sent you a private message.
     */
    boost::signals2::signal<void (query_event)> on_query;

    /**
     * Signal: on_topic
     * ----------------------------------------------------------
     *
     * Triggered when someone changed the channel topic.
     */
    boost::signals2::signal<void (topic_event)> on_topic;

    /**
     * Signal: on_whois
     * ----------------------------------------------------------
     *
     * Triggered when whois information has been received.
     */
    boost::signals2::signal<void (whois_event)> on_whois;

private:
    state_t state_{state_t::disconnected};

    // Requested and joined channels.
    std::vector<channel> rchannels_;
    std::vector<std::string> jchannels_;

    // Identifier.
    std::string name_;

    // Connection information.
    std::string host_;
    std::string password_;
    std::uint16_t port_{6667};
    std::uint8_t flags_{0};

    // Identity.
    std::string nickname_;
    std::string username_;
    std::string realname_{"IRC Client Daemon"};
    std::string ctcpversion_{"IRC Client Daemon"};

    // Settings.
    std::string command_char_{"!"};
    std::int8_t recotries_{-1};
    std::uint16_t recodelay_{30};
    std::uint16_t timeout_{1000};

    // Server information.
    std::map<channel_mode, char> modes_;

    // Misc.
    boost::asio::io_service& service_;
    boost::asio::deadline_timer timer_;
    std::unique_ptr<irc::connection> conn_;
    std::int8_t recocur_{0};
    std::map<std::string, std::set<std::string>> names_map_;
    std::map<std::string, class whois> whois_map_;

    void remove_joined_channel(const std::string& channel);

    void dispatch_connect(const irc::message&);
    void dispatch_endofnames(const irc::message&);
    void dispatch_endofwhois(const irc::message&);
    void dispatch_invite(const irc::message&);
    void dispatch_isupport(const irc::message&);
    void dispatch_join(const irc::message&);
    void dispatch_kick(const irc::message&);
    void dispatch_mode(const irc::message&);
    void dispatch_namreply(const irc::message&);
    void dispatch_nick(const irc::message&);
    void dispatch_notice(const irc::message&);
    void dispatch_part(const irc::message&);
    void dispatch_ping(const irc::message&);
    void dispatch_privmsg(const irc::message&);
    void dispatch_topic(const irc::message&);
    void dispatch_whoischannels(const irc::message&);
    void dispatch_whoisuser(const irc::message&);
    void dispatch(const irc::message&);

    void handle_recv(boost::system::error_code code, irc::message message);
    void handle_connect(boost::system::error_code);
    void recv();
    void identify();
    void wait();

public:
    /**
     * Split a channel from the form channel:password into a server_channel
     * object.
     *
     * \param value the value
     * \return a channel
     */
    static channel split_channel(const std::string& value);

    /**
     * Construct a server.
     *
     * \param service the service
     * \param name the identifier
     */
    server(boost::asio::io_service& service, std::string name);

    /**
     * Destructor. Close the connection if needed.
     */
    virtual ~server();

    /**
     * Get the current server state.
     *
     * \return the state
     */
    inline state_t state() const noexcept
    {
        return state_;
    }

    /**
     * Get the server identifier.
     *
     * \return the id
     */
    inline const std::string& name() const noexcept
    {
        return name_;
    }

    /**
     * Get the hostname.
     *
     * \return the hostname
     */
    inline const std::string& host() const noexcept
    {
        return host_;
    }

    /**
     * Set the hostname.
     *
     * \param host the hostname
     */
    inline void set_host(std::string host) noexcept
    {
        host_ = std::move(host);
    }

    /**
     * Get the password.
     *
     * \return the password
     */
    inline const std::string& password() const noexcept
    {
        return password_;
    }

    /**
     * Set the password.
     *
     * An empty password means no password.
     *
     * \param password the password
     */
    inline void set_password(std::string password) noexcept
    {
        password_ = std::move(password);
    }

    /**
     * Get the port.
     *
     * \return the port
     */
    inline std::uint16_t port() const noexcept
    {
        return port_;
    }

    /**
     * Set the port.
     *
     * \param port the port
     */
    inline void set_port(std::uint16_t port) noexcept
    {
        port_ = port;
    }

    /**
     * Get the flags.
     *
     * \return the flags
     */
    inline std::uint8_t flags() const noexcept
    {
        return flags_;
    }

    /**
     * Set the flags.
     *
     * \param flags the flags
     */
    inline void set_flags(std::uint8_t flags) noexcept
    {
#if !defined(HAVE_SSL)
        assert(!(flags & ssl));
#endif

        flags_ = flags;
    }

    /**
     * Get the nickname.
     *
     * \return the nickname
     */
    inline const std::string& nickname() const noexcept
    {
        return nickname_;
    }

    /**
     * Set the nickname.
     *
     * If the server is connected, send a nickname command to the IRC server,
     * otherwise change it locally.
     *
     * \param nickname the nickname
     */
    virtual void set_nickname(std::string nickname);

    /**
     * Get the username.
     *
     * \return the username
     */
    inline const std::string& username() const noexcept
    {
        return username_;
    }

    /**
     * Set the username.
     *
     * \param name the username
     * \note the username will be changed on the next connection
     */
    inline void set_username(std::string name) noexcept
    {
        username_ = std::move(name);
    }

    /**
     * Get the realname.
     *
     * \return the realname
     */
    inline const std::string& realname() const noexcept
    {
        return realname_;
    }

    /**
     * Set the realname.
     *
     * \param realname the username
     * \note the username will be changed on the next connection
     */
    inline void set_realname(std::string realname) noexcept
    {
        realname_ = std::move(realname);
    }

    /**
     * Get the CTCP version.
     *
     * \return the CTCP version
     */
    inline const std::string& ctcp_version() const noexcept
    {
        return ctcpversion_;
    }

    /**
     * Set the CTCP version.
     *
     * \param ctcpversion the version
     */
    void set_ctcp_version(std::string ctcpversion);

    /**
     * Get the command character.
     *
     * \return the character
     */
    inline const std::string& command_char() const noexcept
    {
        return command_char_;
    }

    /**
     * Set the command character.
     *
     * \pre !command_char_.empty()
     * \param command_char the command character
     */
    inline void set_command_char(std::string command_char) noexcept
    {
        assert(!command_char.empty());

        command_char_ = std::move(command_char);
    }

    /**
     * Get the number of reconnections before giving up.
     *
     * \return the number of reconnections
     */
    inline std::int8_t reconnect_tries() const noexcept
    {
        return recotries_;
    }

    /**
     * Set the number of reconnections to test before giving up.
     *
     * A value less than 0 means infinite.
     *
     * \param reconnect_tries the number of reconnections
     */
    inline void set_reconnect_tries(std::int8_t reconnect_tries) noexcept
    {
        recotries_ = reconnect_tries;
    }

    /**
     * Get the reconnection delay before retrying.
     *
     * \return the number of seconds
     */
    inline std::uint16_t reconnect_delay() const noexcept
    {
        return recodelay_;
    }

    /**
     * Set the number of seconds before retrying.
     *
     * \param reconnect_delay the number of seconds
     */
    inline void set_reconnect_delay(std::uint16_t reconnect_delay) noexcept
    {
        recodelay_ = reconnect_delay;
    }

    /**
     * Get the ping timeout.
     *
     * \return the ping timeout
     */
    inline std::uint16_t ping_timeout() const noexcept
    {
        return timeout_;
    }

    /**
     * Set the ping timeout before considering a server as dead.
     *
     * \param ping_timeout the delay in seconds
     */
    inline void set_ping_timeout(std::uint16_t ping_timeout) noexcept
    {
        timeout_ = ping_timeout;
    }

    /**
     * Get the list of channels joined.
     *
     * \return the channels
     */
    inline const std::vector<std::string>& channels() const noexcept
    {
        return jchannels_;
    }

    /**
     * Determine if the nickname is the bot itself.
     *
     * \param nick the nickname to check
     * \return true if it is the bot
     */
    bool is_self(const std::string& nick) const noexcept;

    /**
     * Start connecting.
     */
    virtual void connect() noexcept;

    /**
     * Force disconnection.
     */
    virtual void disconnect() noexcept;

    /**
     * Asks for a reconnection.
     */
    virtual void reconnect() noexcept;

    /**
     * Invite a user to a channel.
     *
     * \param target the target nickname
     * \param channel the channel
     */
    virtual void invite(std::string target, std::string channel);

    /**
     * Join a channel, the password is optional and can be kept empty.
     *
     * \param channel the channel to join
     * \param password the optional password
     */
    virtual void join(std::string channel, std::string password = "");

    /**
     * Kick someone from the channel. Please be sure to have the rights
     * on that channel because errors won't be reported.
     *
     * \param target the target to kick
     * \param channel from which channel
     * \param reason the optional reason
     */
    virtual void kick(std::string target, std::string channel, std::string reason = "");

    /**
     * Send a CTCP Action as known as /me. The target may be either a
     * channel or a nickname.
     *
     * \param target the nickname or the channel
     * \param message the message
     */
    virtual void me(std::string target, std::string message);

    /**
     * Send a message to the specified target or channel.
     *
     * \param target the target
     * \param message the message
     */
    virtual void message(std::string target, std::string message);

    /**
     * Change channel/user mode.
     *
     * \param channel the channel or nickname
     * \param mode the mode
     * \param limit the optional limit
     * \param user the optional user
     * \param mask the optional ban mask
     */
    virtual void mode(std::string channel,
                      std::string mode,
                      std::string limit = "",
                      std::string user = "",
                      std::string mask = "");

    /**
     * Request the list of names.
     *
     * \param channel the channel
     */
    virtual void names(std::string channel);

    /**
     * Send a private notice.
     *
     * \param target the target
     * \param message the notice message
     */
    virtual void notice(std::string target, std::string message);

    /**
     * Part from a channel.
     *
     * Please note that the reason is not supported on all servers so if you
     * want portability, don't provide it.
     *
     * \param channel the channel to leave
     * \param reason the optional reason
     */
    virtual void part(std::string channel, std::string reason = "");

    /**
     * Send a raw message to the IRC server. You don't need to add
     * message terminators.
     *
     * \pre state() == state_t::connected
     * \param raw the raw message (without `\r\n\r\n`)
     */
    virtual void send(std::string raw);

    /**
     * Change the channel topic.
     *
     * \param channel the channel
     * \param topic the desired topic
     */
    virtual void topic(std::string channel, std::string topic);

    /**
     * Request for whois information.
     *
     * \param target the target nickname
     */
    virtual void whois(std::string target);
};

/**
 * \brief Server error.
 */
class server_error : public boost::system::system_error {
public:
    /**
     * \brief Server related errors.
     */
    enum error {
        //!< No error.
        no_error = 0,

        //!< The specified server was not found.
        not_found,

        //!< The specified identifier is invalid.
        invalid_identifier,

        //!< The server is not connected.
        not_connected,

        //!< The server is already connected.
        already_connected,

        //!< Server with same name already exists.
        already_exists,

        //!< The specified port number is invalid.
        invalid_port,

        //!< The specified reconnect tries number is invalid.
        invalid_reconnect_tries,

        //!< The specified reconnect reconnect number is invalid.
        invalid_reconnect_timeout,

        //!< The specified host was invalid.
        invalid_hostname,

        //!< The channel was empty or invalid.
        invalid_channel,

        //!< The mode given was empty.
        invalid_mode,

        //!< The nickname was empty or invalid.
        invalid_nickname,

        //!< The username was empty or invalid.
        invalid_username,

        //!< The realname was empty or invalid.
        invalid_realname,

        //!< Invalid password property.
        invalid_password,

        //!< Invalid ping timeout.
        invalid_ping_timeout,

        //!< Invalid ctcp version.
        invalid_ctcp_version,

        //!< Invalid command character.
        invalid_command_char,

        //!< SSL was requested but is disabled.
        ssl_disabled,
    };

private:
    std::string name_;

public:
    /**
     * Constructor.
     *
     * \param name the server name
     * \param code the error code
     */
    server_error(std::string name, error code) noexcept;

    /**
     * Get the server that triggered the error.
     *
     * \return the name
     */
    inline const std::string& name() const noexcept
    {
        return name_;
    }
};

/**
 * Get the server error category singleton.
 *
 * \return the singleton
 */
const boost::system::error_category& server_category();

/**
 * Create a boost::system::error_code from server_error::error enum.
 *
 * \param e the error code
 */
boost::system::error_code make_error_code(server_error::error e);

} // !irccd

namespace boost {

namespace system {

template <>
struct is_error_code_enum<irccd::server_error::error> : public std::true_type {
};

} // !system

} // !boost

#endif // !IRCCD_DAEMON_SERVER_HPP
