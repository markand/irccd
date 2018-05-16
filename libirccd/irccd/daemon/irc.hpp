/*
 * irc.hpp -- low level IRC functions
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

#ifndef IRCCD_IRC_HPP
#define IRCCD_IRC_HPP

/**
 * \file irc.hpp
 * \brief Low level IRC functions.
 */

#include <irccd/sysconfig.hpp>

#include <functional>
#include <string>
#include <utility>
#include <vector>

#include <boost/asio.hpp>

#if defined(IRCCD_HAVE_SSL)
#   include <boost/asio/ssl.hpp>
#endif

namespace irccd {

namespace irc {

class message;

/**
 * \brief Describe errors.
 */
enum class err {
    nosuchnick = 401,
    nosuchserver = 402,
    nosuchchannel = 403,
    cannotsendtochan = 404,
    toomanychannels = 405,
    wasnosuchnick = 406,
    toomanytargets = 407,
    noorigin = 409,
    norecipient = 411,
    notexttosend = 412,
    notoplevel = 413,
    wildtoplevel = 414,
    unknowncommand = 421,
    nomotd = 422,
    noadmininfo = 423,
    fileerror = 424,
    nonicknamegiven = 431,
    erroneusnickname = 432,
    nicknameinuse = 433,
    nickcollision = 436,
    usernotinchannel = 441,
    notonchannel = 442,
    useronchannel = 443,
    nologin = 444,
    summondisabled = 445,
    usersdisabled = 446,
    notregistered = 451,
    needmoreparams = 461,
    alreadyregistred = 462,
    nopermforhost = 463,
    passwdmismatch = 464,
    yourebannedcreep = 465,
    keyset = 467,
    channelisfull = 471,
    unknownmode = 472,
    inviteonlychan = 473,
    bannedfromchan = 474,
    badchannelkey = 475,
    noprivileges = 481,
    chanoprivsneeded = 482,
    cantkillserver = 483,
    nooperhost = 491,
    umodeunknownflag = 501,
    usersdontmatch = 502
};

/**
 * \brief Describe numeric replies.
 */
enum class rpl {
    none = 300,
    userhost = 302,
    ison = 303,
    away = 301,
    unaway = 305,
    nowaway = 306,
    whoisuser = 311,
    whoisserver = 312,
    whoisoperator = 313,
    whoisidle = 317,
    endofwhois = 318,
    whoischannels = 319,
    whowasuser = 314,
    endofwhowas = 369,
    liststart = 321,
    list = 322,
    listend = 323,
    channelmodeis = 324,
    notopic = 331,
    topic = 332,
    inviting = 341,
    summoning = 342,
    version = 351,
    whoreply = 352,
    endofwho = 315,
    namreply = 353,
    endofnames = 366,
    links = 364,
    endoflinks = 365,
    banlist = 367,
    endofbanlist = 368,
    info = 371,
    endofinfo = 374,
    motdstart = 375,
    motd = 372,
    endofmotd = 376,
    youreoper = 381,
    rehashing = 382,
    time = 391,
    userstart = 392,
    users = 393,
    endofusers = 394,
    nousers = 395,
    tracelink = 200,
    traceconnecting = 201,
    tracehandshake = 202,
    traceunknown = 203,
    traceoperator = 204,
    traceuser = 205,
    traceserver = 206,
    tracenewtype = 208,
    tracelog = 261,
    statslinkinfo = 211,
    statscommands = 212,
    statscline = 213,
    statsnline = 214,
    statsiline = 215,
    statskline = 216,
    statsyline = 218,
    endofstats = 219,
    statslline = 241,
    statsuptime = 242,
    statsoline = 243,
    statshline = 244,
    umodeis = 221,
    luserclient = 251,
    luserop = 252,
    luserunknown = 253,
    luserchannels = 254,
    luserme = 255,
    adminme = 256,
    adminloc1 = 257,
    adminloc2 = 258,
    adminemail = 259
};

/**
 * \brief Describe a IRC message
 */
class message {
private:
    std::string prefix_;             //!< optional prefix
    std::string command_;            //!< command (maybe string or code)
    std::vector<std::string> args_;  //!< parameters

public:
    /**
     * Constructor.
     *
     * \param prefix the optional prefix
     * \param command the command string or number
     * \param args the arguments
     */
    inline message(std::string prefix = "", std::string command = "", std::vector<std::string> args = {}) noexcept
        : prefix_(std::move(prefix))
        , command_(std::move(command))
        , args_(std::move(args))
    {
    }

    /**
     * Get the prefix.
     *
     * \return the prefix
     */
    inline const std::string& prefix() const noexcept
    {
        return prefix_;
    }

    /**
     * Get the command.
     *
     * \return the command
     */
    inline const std::string& command() const noexcept
    {
        return command_;
    }

    /**
     * Get the arguments.
     *
     * \return the arguments
     */
    inline const std::vector<std::string>& args() const noexcept
    {
        return args_;
    }

    /**
     * Check if the message is defined.
     *
     * \return true if not empty
     */
    inline operator bool() const noexcept
    {
        return !command_.empty();
    }

    /**
     * Check if the message is empty.
     *
     * \return true if empty
     */
    inline bool operator!() const noexcept
    {
        return command_.empty();
    }

    /**
     * Check if the command is of the given enum number.
     *
     * \return true if command is a number and equals to e
     */
    template <typename Enum>
    inline bool is(Enum e) const noexcept
    {
        try {
            return std::stoi(command_) == static_cast<int>(e);
        } catch (...) {
            return false;
        }
    }

    /**
     * Convenient function that returns an empty string if the nth argument is
     * not defined.
     *
     * \param index the index
     * \return a string or empty if out of bounds
     */
    inline const std::string& arg(unsigned short index) const noexcept
    {
        static const std::string dummy;

        return (index >= args_.size()) ? dummy : args_[index];
    }

    /**
     * Tells if the message is a CTCP.
     *
     * \param index the param index (maybe out of bounds)
     * \return true if CTCP
     */
    bool is_ctcp(unsigned index) const noexcept;

    /**
     * Parse a CTCP message.
     *
     * \pre is_ctcp(index)
     * \param index the param index
     * \return the CTCP command
     */
    std::string ctcp(unsigned index) const;

    /**
     * Parse a IRC message.
     *
     * \param line the buffer content (without `\r\n`)
     * \return the message (maybe empty if line is empty)
     */
    static message parse(const std::string& line);
};

/**
 * \brief Describe a user.
 */
class user {
private:
    std::string nick_;
    std::string host_;

public:
    /**
     * Construct a user.
     *
     * \param nick the nickname
     * \param host the hostname
     */
    inline user(std::string nick, std::string host) noexcept
        : nick_(std::move(nick))
        , host_(std::move(host))
    {
    }

    /**
     * Get the nick part.
     *
     * \return the nickname
     */
    inline const std::string& nick() const noexcept
    {
        return nick_;
    }

    /**
     * Get the host part.
     *
     * \return the host part
     */
    inline const std::string& host() const noexcept
    {
        return host_;
    }

    /**
     * Parse a nick/host combination.
     *
     * \param line the line to parse
     * \return a user
     */
    static user parse(const std::string& line);
};

/**
 * \brief Abstract connection to a server.
 */
class connection {
public:
    /**
     * Handler for connecting.
     */
    using connect_handler = std::function<void (boost::system::error_code)>;

    /**
     * Handler for receiving.
     */
    using recv_handler = std::function<void (boost::system::error_code, message)>;

    /**
     * Handler for sending.
     */
    using send_handler = std::function<void (boost::system::error_code)>;

private:
    boost::asio::ip::tcp::resolver resolver_;
    boost::asio::streambuf input_;
    boost::asio::streambuf output_;

#if !defined(NDEBUG)
    bool is_connecting_{false};
    bool is_receiving_{false};
    bool is_sending_{false};
#endif

protected:
    /**
     * Use boost::asio::async_resolve and boost::asio::async_connect on the
     * given real socket type.
     *
     * \param socket the socket
     * \param host the hostname
     * \param service the service or port number
     * \param handler the non-null handler
     */
    template <typename Socket>
    void wrap_connect(Socket& socket,
                      const std::string& host,
                      const std::string& service,
                      const connect_handler& handler) noexcept;

    /**
     * Use boost::asio::asynd_read_until on the given real socket type.
     *
     * \param socket the socket
     * \param handler the non-null handler
     */
    template <typename Socket>
    void wrap_recv(Socket& socket, const recv_handler& handler) noexcept;

    /**
     * Use boost::asio::asynd_write on the given real socket type.
     *
     * \param socket the socket
     * \param handler the non-null handler
     */
    template <typename Socket>
    void wrap_send(Socket& socket, const send_handler& handler) noexcept;

    /**
     * Do the connection.
     *
     * Derived implementation may call wrap_connect on its underlying socket.
     *
     * \param host the hostname
     * \param service the service or port number
     * \param handler the non-null handler
     */
    virtual void do_connect(const std::string& host,
                            const std::string& service,
                            const connect_handler& handler) noexcept = 0;

    /**
     * Receive some data.
     *
     * \param handler the non-null handler
     */
    virtual void do_recv(const recv_handler& handler) noexcept = 0;

    /**
     * Send data.
     *
     * \param handler the non-null handler
     */
    virtual void do_send(const send_handler& handler) noexcept = 0;

public:
    /**
     * Default constructor.
     */
    inline connection(boost::asio::io_service& service)
        : resolver_(service)
    {
    }

    /**
     * Virtual destructor defaulted.
     */
    virtual ~connection() = default;

    /**
     * Connect to the host.
     *
     * \pre handler the handler
     * \pre another connect operation must not be running
     * \param host the host
     * \param service the service or port number
     * \param handler the non-null handler
     */
    void connect(const std::string& host,
                 const std::string& service,
                 const connect_handler& handler) noexcept;

    /**
     * Start receiving data.
     *
     * The handler must not throw exceptions and `this` must be valid in the
     * lifetime of the handler.
     *
     * \pre another recv operation must not be running
     * \pre handler != nullptr
     * \param handler the handler to call
     */
    void recv(const recv_handler& handler) noexcept;

    /**
     * Start sending data.
     *
     * The handler must not throw exceptions and `this` must be valid in the
     * lifetime of the handler.
     *
     * \pre another send operation must not be running
     * \pre handler != nullptr
     * \param message the raw message
     * \param handler the handler to call
     */
    void send(std::string message, const send_handler& handler);
};

/**
 * \brief Clear TCP connection
 */
class ip_connection : public connection {
private:
    boost::asio::ip::tcp::socket socket_;

protected:
    /**
     * \copydoc connection::do_connect
     */
    void do_connect(const std::string& host,
                    const std::string& service,
                    const connect_handler& handler) noexcept override;

    /**
     * \copydoc connection::do_recv
     */
    void do_recv(const recv_handler& handler) noexcept override;

    /**
     * \copydoc connection::do_send
     */
    void do_send(const send_handler& handler) noexcept override;

public:
    /**
     * Constructor.
     *
     * \param service the io service
     */
    inline ip_connection(boost::asio::io_service& service)
        : connection(service)
        , socket_(service)
    {
    }
};

#if defined(IRCCD_HAVE_SSL)

/**
 * \brief SSL connection
 */
class tls_connection : public connection {
private:
    boost::asio::ssl::context context_;
    boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket_;

protected:
    /**
     * \copydoc connection::do_connect
     */
    void do_connect(const std::string& host,
                    const std::string& service,
                    const connect_handler& handler) noexcept override;

    /**
     * \copydoc connection::do_recv
     */
    void do_recv(const recv_handler& handler) noexcept override;

    /**
     * \copydoc connection::do_send
     */
    void do_send(const send_handler& handler) noexcept override;

public:
    /**
     * Constructor.
     *
     * \param service the io service
     */
    inline tls_connection(boost::asio::io_service& service)
        : connection(service)
        , context_(boost::asio::ssl::context::sslv23)
        , socket_(service, context_)
    {
    }
};

#endif // !IRCCD_HAVE_SSL

} // !irc

} // !irccd

#endif // !IRCCD_IRC_HPP
