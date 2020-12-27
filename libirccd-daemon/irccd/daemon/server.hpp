/*
 * server.hpp -- an IRC server
 *
 * Copyright (c) 2013-2020 David Demelier <markand@malikania.fr>
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
#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <variant>
#include <vector>

#include <json.hpp>

#include "irc.hpp"

namespace irccd::daemon {

class server;

/**
 * \brief Prefixes for nicknames.
 * \ingroup daemon-servers
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
 * \ingroup daemon-servers
 */
struct channel {
	std::string name;                       //!< the channel to join
	std::string password;                   //!< the optional password
};

/**
 * \brief Describe a whois information.
 * \ingroup daemon-server-events
 */
struct whois_info {
	std::string nick;                       //!< user's nickname
	std::string user;                       //!< user's user
	std::string hostname;                   //!< hostname
	std::string realname;                   //!< realname
	std::vector<std::string> channels;      //!< the channels where the user is
};

/**
 * \brief Connection success event.
 * \ingroup daemon-server-events
 */
struct connect_event {
	std::shared_ptr<class server> server;   //!< The server.
};

/**
 * \brief Connection success event.
 * \ingroup daemon-server-events
 */
struct disconnect_event {
	std::shared_ptr<class server> server;   //!< The server.
};

/**
 * \brief Invite event.
 * \ingroup daemon-server-events
 */
struct invite_event {
	std::shared_ptr<class server> server;   //!< The server.
	std::string origin;                     //!< The originator.
	std::string channel;                    //!< The channel.
	std::string nickname;                   //!< The nickname (you).
};

/**
 * \brief Join event.
 * \ingroup daemon-server-events
 */
struct join_event {
	std::shared_ptr<class server> server;   //!< The server.
	std::string origin;                     //!< The originator.
	std::string channel;                    //!< The channel.
};

/**
 * \brief Kick event.
 * \ingroup daemon-server-events
 */
struct kick_event {
	std::shared_ptr<class server> server;   //!< The server.
	std::string origin;                     //!< The originator.
	std::string channel;                    //!< The channel.
	std::string target;                     //!< The target.
	std::string reason;                     //!< The reason (Optional).
};

/**
 * \brief Message event.
 * \ingroup daemon-server-events
 */
struct message_event {
	std::shared_ptr<class server> server;   //!< The server.
	std::string origin;                     //!< The originator.
	std::string channel;                    //!< The channel.
	std::string message;                    //!< The message.
};

/**
 * \brief CTCP action event.
 * \ingroup daemon-server-events
 */
struct me_event {
	std::shared_ptr<class server> server;   //!< The server.
	std::string origin;                     //!< The originator.
	std::string channel;                    //!< The channel.
	std::string message;                    //!< The message.
};

/**
 * \brief Mode event.
 * \ingroup daemon-server-events
 */
struct mode_event {
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
 * \ingroup daemon-server-events
 */
struct names_event {
	std::shared_ptr<class server> server;   //!< The server.
	std::string channel;                    //!< The channel.
	std::vector<std::string> names;         //!< The names.
};

/**
 * \brief Nick change event.
 * \ingroup daemon-server-events
 */
struct nick_event {
	std::shared_ptr<class server> server;   //!< The server.
	std::string origin;                     //!< The originator.
	std::string nickname;                   //!< The new nickname.
};

/**
 * \brief Notice event.
 * \ingroup daemon-server-events
 */
struct notice_event {
	std::shared_ptr<class server> server;   //!< The server.
	std::string origin;                     //!< The originator.
	std::string channel;                    //!< The channel or target.
	std::string message;                    //!< The message.
};

/**
 * \brief Part event.
 * \ingroup daemon-server-events
 */
struct part_event {
	std::shared_ptr<class server> server;   //!< The server.
	std::string origin;                     //!< The originator.
	std::string channel;                    //!< The channel.
	std::string reason;                     //!< The reason.
};

/**
 * \brief Topic event.
 * \ingroup daemon-server-events
 */
struct topic_event {
	std::shared_ptr<class server> server;   //!< The server.
	std::string origin;                     //!< The originator.
	std::string channel;                    //!< The channel.
	std::string topic;                      //!< The topic message.
};

/**
 * \brief Whois event.
 * \ingroup daemon-server-events
 */
struct whois_event {
	std::shared_ptr<class server> server;   //!< The server.
	whois_info whois;                       //!< The whois information.
};

/**
 * \brief Store all possible events.
 * \ingroup daemon-server-events
 */
using event = std::variant<
	std::monostate,
	connect_event,
	disconnect_event,
	invite_event,
	join_event,
	kick_event,
	me_event,
	message_event,
	mode_event,
	names_event,
	nick_event,
	notice_event,
	part_event,
	topic_event,
	whois_event
>;

/**
 * \brief The class that connect to a IRC server.
 * \ingroup daemon-servers
 *
 * This class is higher level than irc connection, it does identify process,
 * parsing message, translating messages and queue'ing user requests.
 */
class server : public std::enable_shared_from_this<server> {
public:
	/**
	 * Completion handler once network connection is complete.
	 */
	using connect_handler = std::function<void (std::error_code)>;

	/**
	 * Completion handler once a network message has arrived.
	 */
	using recv_handler = std::function<void (std::error_code, event)>;

	/**
	 * \brief Various options for server.
	 */
	enum class options : std::uint8_t {
		none            = 0,            //!< No options
		ipv4            = (1 << 0),     //!< Connect using IPv4
		ipv6            = (1 << 1),     //!< Connect using IPv6
		ssl             = (1 << 2),     //!< Use SSL
		auto_rejoin     = (1 << 3),     //!< Auto rejoin a kick
		auto_reconnect  = (1 << 4),     //!< Auto reconnect on disconnection
		join_invite     = (1 << 5)      //!< Join a channel on invitation
	};

	/**
	 * \brief Describe current server state.
	 */
	enum class state : std::uint8_t {
		disconnected,                   //!< not connected at all,
		connecting,                     //!< network connection in progress,
		identifying,                    //!< sending nick, user and password commands,
		connected                       //!< ready for use.
	};

protected:
	/**
	 * \brief Server state.
	 */
	state state_{state::disconnected};

private:
	// Requested and joined channels.
	std::vector<channel> rchannels_;
	std::set<std::string> jchannels_;

	// Identifier.
	std::string id_;

	// Connection information.
	std::string hostname_;
	std::string password_;
	std::uint16_t port_{6667};
	options options_;

	// Identity.
	std::string nickname_{"irccd"};
	std::string username_{"irccd"};
	std::string realname_{"IRC Client Daemon"};
	std::string ctcpversion_{"IRC Client Daemon"};

	// Settings.
	std::string command_char_{"!"};
	std::uint16_t recodelay_{30};
	std::uint16_t timeout_{900};

	// Server information.
	std::map<channel_mode, char> modes_;

	// Misc.
	boost::asio::io_service& service_;
	boost::asio::deadline_timer timer_;
	std::shared_ptr<irc::connection> conn_;
	std::deque<std::string> queue_;
	std::map<std::string, std::set<std::string>> names_map_;
	std::map<std::string, whois_info> whois_map_;

	auto dispatch_connect(const irc::message&, const recv_handler&) -> bool;
	auto dispatch_endofnames(const irc::message&, const recv_handler&) -> bool;
	auto dispatch_endofwhois(const irc::message&, const recv_handler&) -> bool;
	auto dispatch_invite(const irc::message&, const recv_handler&) -> bool;
	auto dispatch_isupport(const irc::message&) -> bool;
	auto dispatch_join(const irc::message&, const recv_handler&) -> bool;
	auto dispatch_kick(const irc::message&, const recv_handler&) -> bool;
	auto dispatch_mode(const irc::message&, const recv_handler&) -> bool;
	auto dispatch_namreply(const irc::message&) -> bool;
	auto dispatch_nick(const irc::message&, const recv_handler&) -> bool;
	auto dispatch_notice(const irc::message&, const recv_handler&) -> bool;
	auto dispatch_part(const irc::message&, const recv_handler&) -> bool;
	auto dispatch_ping(const irc::message&) -> bool;
	auto dispatch_privmsg(const irc::message&, const recv_handler&) -> bool;
	auto dispatch_topic(const irc::message&, const recv_handler&) -> bool;
	auto dispatch_whoischannels(const irc::message&) -> bool;
	auto dispatch_whoisuser(const irc::message&) -> bool;
	auto dispatch(const irc::message&, const recv_handler&) -> bool;

	// I/O and connection.
	void flush();
	void identify();
	void handle_send(const std::error_code&);
	void handle_recv(const std::error_code&, const irc::message&, const recv_handler&);
	void handle_connect(const std::error_code&, const connect_handler&);

public:
	/**
	 * Construct a server.
	 *
	 * \pre !host.empty()
	 * \param service the service
	 * \param id the identifier
	 * \param hostname the hostname
	 */
	server(boost::asio::io_service& service, std::string id, std::string hostname = "localhost");

	/**
	 * Destructor. Close the connection if needed.
	 */
	virtual ~server();

	/**
	 * Get the current server state.
	 *
	 * \return the state
	 */
	auto get_state() const noexcept -> state;

	/**
	 * Get the server identifier.
	 *
	 * \return the id
	 */
	auto get_id() const noexcept -> const std::string&;

	/**
	 * Get the hostname.
	 *
	 * \return the hostname
	 */
	auto get_hostname() const noexcept -> const std::string&;

	/**
	 * Get the password.
	 *
	 * \return the password
	 */
	auto get_password() const noexcept -> const std::string&;

	/**
	 * Set the password.
	 *
	 * An empty password means no password.
	 *
	 * \param password the password
	 */
	void set_password(std::string password) noexcept;

	/**
	 * Get the port.
	 *
	 * \return the port
	 */
	auto get_port() const noexcept -> std::uint16_t;

	/**
	 * Set the port.
	 *
	 * \param port the port
	 */
	void set_port(std::uint16_t port) noexcept;

	/**
	 * Get the options flags.
	 *
	 * \return the flags
	 */
	auto get_options() const noexcept -> options;

	/**
	 * Set the options flags.
	 *
	 * \param flags the flags
	 */
	void set_options(options flags) noexcept;

	/**
	 * Get the nickname.
	 *
	 * \return the nickname
	 */
	auto get_nickname() const noexcept -> const std::string&;

	/**
	 * Set the nickname.
	 *
	 * If the server is connected, send a nickname command to the IRC server,
	 * otherwise change it instantly.
	 *
	 * \param nickname the nickname
	 */
	void set_nickname(std::string nickname);

	/**
	 * Get the username.
	 *
	 * \return the username
	 */
	auto get_username() const noexcept -> const std::string&;

	/**
	 * Set the username.
	 *
	 * \param name the username
	 * \note the username will be changed on the next connection
	 */
	void set_username(std::string name) noexcept;

	/**
	 * Get the realname.
	 *
	 * \return the realname
	 */
	auto get_realname() const noexcept -> const std::string&;

	/**
	 * Set the realname.
	 *
	 * \param realname the username
	 * \note the username will be changed on the next connection
	 */
	void set_realname(std::string realname) noexcept;

	/**
	 * Get the CTCP version.
	 *
	 * \return the CTCP version
	 */
	auto get_ctcp_version() const noexcept -> const std::string&;

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
	auto get_command_char() const noexcept -> const std::string&;

	/**
	 * Set the command character.
	 *
	 * \pre !command_char_.empty()
	 * \param command_char the command character
	 */
	void set_command_char(std::string command_char) noexcept;

	/**
	 * Get the reconnection delay before retrying.
	 *
	 * \return the number of seconds
	 */
	auto get_reconnect_delay() const noexcept -> std::uint16_t;

	/**
	 * Set the number of seconds before retrying.
	 *
	 * \param reconnect_delay the number of seconds
	 */
	void set_reconnect_delay(std::uint16_t reconnect_delay) noexcept;

	/**
	 * Get the ping timeout.
	 *
	 * \return the ping timeout
	 */
	auto get_ping_timeout() const noexcept -> std::uint16_t;

	/**
	 * Set the ping timeout before considering a server as dead.
	 *
	 * \param ping_timeout the delay in seconds
	 */
	void set_ping_timeout(std::uint16_t ping_timeout) noexcept;

	/**
	 * Get the list of channels joined.
	 *
	 * \return the channels
	 */
	auto get_channels() const noexcept -> const std::set<std::string>&;

	/**
	 * Determine if the nickname is the bot itself.
	 *
	 * \param nick the nickname to check
	 * \return true if it is the bot
	 */
	auto is_self(std::string_view nick) const noexcept -> bool;

	/**
	 * Start connecting.
	 *
	 * This only initiate TCP connection and/or SSL handshaking, the identifying
	 * process may take some time and you must repeatedly call recv() to wait
	 * for connect_event.
	 *
	 * \pre handler != nullptr
	 * \param handler the completion handler
	 * \note the server must be kept alive until completion
	 */
	virtual void connect(connect_handler handler) noexcept;

	/**
	 * Force disconnection.
	 */
	virtual void disconnect();

	/**
	 * Wait for reconnect delay.
	 *
	 * \pre another wait operation must not be running
	 * \pre get_state() == state::disconnected
	 */
	virtual void wait(connect_handler handler);

	/**
	 * Receive next event.
	 *
	 * \pre handler != nullptr
	 * \param handler the handler
	 * \note the server must be kept alive until completion
	 */
	virtual void recv(recv_handler handler) noexcept;

	/**
	 * Invite a user to a channel.
	 *
	 * \param target the target nickname
	 * \param channel the channel
	 */
	virtual void invite(std::string_view target, std::string_view channel);

	/**
	 * Join a channel, the password is optional and can be kept empty.
	 *
	 * \param channel the channel to join
	 * \param password the optional password
	 */
	virtual void join(std::string_view channel, std::string_view password = "");

	/**
	 * Kick someone from the channel. Please be sure to have the rights
	 * on that channel because errors won't be reported.
	 *
	 * \param target the target to kick
	 * \param channel from which channel
	 * \param reason the optional reason
	 */
	virtual void kick(std::string_view target,
	                  std::string_view channel,
	                  std::string_view reason = "");

	/**
	 * Send a CTCP Action as known as /me. The target may be either a
	 * channel or a nickname.
	 *
	 * \param target the nickname or the channel
	 * \param message the message
	 */
	virtual void me(std::string_view target, std::string_view message);

	/**
	 * Send a message to the specified target or channel.
	 *
	 * \param target the target
	 * \param message the message
	 */
	virtual void message(std::string_view target, std::string_view message);

	/**
	 * Change channel/user mode.
	 *
	 * \param channel the channel or nickname
	 * \param mode the mode
	 * \param limit the optional limit
	 * \param user the optional user
	 * \param mask the optional ban mask
	 */
	virtual void mode(std::string_view channel,
	                  std::string_view mode,
	                  std::string_view limit = "",
	                  std::string_view user = "",
	                  std::string_view mask = "");

	/**
	 * Request the list of names.
	 *
	 * \param channel the channel
	 */
	virtual void names(std::string_view channel);

	/**
	 * Send a private notice.
	 *
	 * \param target the target
	 * \param message the notice message
	 */
	virtual void notice(std::string_view target, std::string_view message);

	/**
	 * Part from a channel.
	 *
	 * Please note that the reason is not supported on all servers so if you
	 * want portability, don't provide it.
	 *
	 * \param channel the channel to leave
	 * \param reason the optional reason
	 */
	virtual void part(std::string_view channel, std::string_view reason = "");

	/**
	 * Send a raw message to the IRC server. You don't need to add
	 * message terminators.
	 *
	 * If the server is not yet connected, the command is postponed and will be
	 * ran when ready.
	 *
	 * \param raw the raw message (without `\r\n\r\n`)
	 */
	virtual void send(std::string_view raw);

	/**
	 * Change the channel topic.
	 *
	 * \param channel the channel
	 * \param topic the desired topic
	 */
	virtual void topic(std::string_view channel, std::string_view topic);

	/**
	 * Request for whois information.
	 *
	 * \param target the target nickname
	 */
	virtual void whois(std::string_view target);
};

/**
 * \cond IRCCD_HIDDEN_SYMBOLS
 */

/**
 * Apply bitwise XOR.
 *
 * \param v1 the first value
 * \param v2 the second value
 * \return the new value
 */
inline auto operator^(server::options v1, server::options v2) noexcept -> server::options
{
	return static_cast<server::options>(static_cast<unsigned>(v1) ^ static_cast<unsigned>(v2));
}

/**
 * Apply bitwise AND.
 *
 * \param v1 the first value
 * \param v2 the second value
 * \return the new value
 */
inline auto operator&(server::options v1, server::options v2) noexcept -> server::options
{
	return static_cast<server::options>(static_cast<unsigned>(v1) & static_cast<unsigned>(v2));
}

/**
 * Apply bitwise OR.
 *
 * \param v1 the first value
 * \param v2 the second value
 * \return the new value
 */
inline auto operator|(server::options v1, server::options v2) noexcept -> server::options
{
	return static_cast<server::options>(static_cast<unsigned>(v1) | static_cast<unsigned>(v2));
}

/**
 * Apply bitwise NOT.
 *
 * \param v the value
 * \return the complement
 */
inline auto operator~(server::options v) noexcept -> server::options
{
	return static_cast<server::options>(~static_cast<unsigned>(v));
}

/**
 * Assign bitwise OR.
 *
 * \param v1 the first value
 * \param v2 the second value
 * \return the new value
 */
inline auto operator|=(server::options& v1, server::options v2) noexcept -> server::options&
{
	return v1 = v1 | v2;
}

/**
 * Assign bitwise AND.
 *
 * \param v1 the first value
 * \param v2 the second value
 * \return the new value
 */
inline auto operator&=(server::options& v1, server::options v2) noexcept -> server::options&
{
	return v1 = v1 & v2;
}

/**
 * Assign bitwise XOR.
 *
 * \param v1 the first value
 * \param v2 the second value
 * \return the new value
 */
inline auto operator^=(server::options& v1, server::options v2) noexcept -> server::options&
{
	return v1 = v1 ^ v2;
}

/**
 * \endcond
 */

/**
 * \brief Server error.
 * \ingroup daemon-servers
 */
class server_error : public std::system_error {
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

		//!< The specified reconnect delay number is invalid.
		invalid_reconnect_delay,

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

		//!< Message (PRIVMSG) was invalid
		invalid_message,

		//!< SSL was requested but is disabled.
		ssl_disabled,

		//!< IPv4 or IPv6 must be defined.
		invalid_family
	};

public:
	/**
	 * Constructor.
	 *
	 * \param code the error code
	 */
	server_error(error code) noexcept;
};

/**
 * Get the server error category singleton.
 *
 * \return the singleton
 */
auto server_category() -> const std::error_category&;

/**
 * Create a std::error_code from server_error::error enum.
 *
 * \param e the error code
 * \return the error code
 */
auto make_error_code(server_error::error e) -> std::error_code;

} // !irccd::daemon

/**
 * \cond IRCCD_HIDDEN_SYMBOLS
 */

namespace std {

template <>
struct is_error_code_enum<irccd::daemon::server_error::error> : public std::true_type {
};

} // !std

/**
 * \endcond
 */

#endif // !IRCCD_DAEMON_SERVER_HPP
