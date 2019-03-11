/*
 * server.cpp -- an IRC server
 *
 * Copyright (c) 2013-2019 David Demelier <markand@malikania.fr>
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

#include <irccd/sysconfig.hpp>

#include <boost/predef/os.h>
#include <boost/format.hpp>

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <stdexcept>

#if !BOOST_OS_WINDOWS
#	include <sys/types.h>
#	include <netinet/in.h>
#	include <arpa/nameser.h>
#	include <resolv.h>
#endif

#include <irccd/json_util.hpp>
#include <irccd/string_util.hpp>

#include "server.hpp"

using boost::format;
using boost::str;

namespace irccd::daemon {

namespace {

/*
 * clean_prefix
 * ------------------------------------------------------------------
 *
 * Remove the user prefix only if it is present in the mode table, for example
 * removes @ from @irccd if and only if @ is a character mode (e.g. operator).
 */
auto clean_prefix(const std::map<channel_mode, char>& modes, std::string nickname) -> std::string
{
	if (nickname.length() == 0)
		return nickname;

	for (const auto& pair : modes)
		if (nickname[0] == pair.second)
			nickname.erase(0, 1);

	return nickname;
}

/*
 * isupport_extract_prefixes
 * ------------------------------------------------------------------
 *
 * Read modes from the IRC event numeric.
 */
auto isupport_extract_prefixes(const std::string& line) -> std::map<channel_mode, char>
{
	// FIXME: what if line has different size?
	std::pair<char, char> table[16];
	std::string buf = line.substr(7);
	std::map<channel_mode, char> modes;

	for (int i = 0; i < 16; ++i)
		table[i] = std::make_pair(-1, -1);

	int j = 0;
	bool read_modes = true;

	for (size_t i = 0; i < buf.size(); ++i) {
		if (buf[i] == '(')
			continue;
		if (buf[i] == ')') {
			j = 0;
			read_modes = false;
			continue;
		}

		if (read_modes)
			table[j++].first = buf[i];
		else
			table[j++].second = buf[i];
	}

	// Put these as a map of mode to prefix.
	for (int i = 0; i < 16; ++i) {
		auto key = static_cast<channel_mode>(table[i].first);
		auto value = table[i].second;

		modes.emplace(key, value);
	}

	return modes;
}

} // !namespace

auto server::dispatch_connect(const irc::message&, const recv_handler& handler) -> bool
{
	state_ = state::connected;
	handler({}, connect_event{shared_from_this()});

	for (const auto& channel : rchannels_)
		join(channel.name, channel.password);

	return true;
}

auto server::dispatch_endofnames(const irc::message& msg, const recv_handler& handler) -> bool
{
	/*
	 * Called when end of name listing has finished on a channel.
	 *
	 * params[0] == originator
	 * params[1] == channel
	 * params[2] == End of NAMES list
	 */
	if (msg.args.size() < 3 || msg.get(1) == "")
		return false;

	const auto it = names_map_.find(msg.get(1));

	if (it != names_map_.end()) {
		handler({}, names_event{
			shared_from_this(),
			msg.get(1),
			std::vector<std::string>(it->second.begin(), it->second.end())
		});

		names_map_.erase(it);
	}

	return true;
}

auto server::dispatch_endofwhois(const irc::message& msg, const recv_handler& handler) -> bool
{
	/*
	 * Called when whois is finished.
	 *
	 * params[0] == originator
	 * params[1] == nickname
	 * params[2] == End of WHOIS list
	 */
	const auto it = whois_map_.find(msg.get(1));

	if (it != whois_map_.end()) {
		handler({}, whois_event{shared_from_this(), it->second});
		whois_map_.erase(it);
	}

	return true;
}

auto server::dispatch_invite(const irc::message& msg, const recv_handler& handler) -> bool
{
	// If join-invite is set, join the channel.
	if ((options_ & options::join_invite) == options::join_invite && is_self(msg.get(0)))
		join(msg.get(1));

	handler({}, invite_event{shared_from_this(), msg.prefix, msg.get(1), msg.get(0)});

	return true;
}

auto server::dispatch_isupport(const irc::message& msg) -> bool
{
	for (unsigned int i = 0; i < msg.args.size(); ++i) {
		if (msg.get(i).compare(0, 6, "PREFIX") == 0) {
			modes_ = isupport_extract_prefixes(msg.get(i));
			break;
		}
	}

	return false;
}

auto server::dispatch_join(const irc::message& msg, const recv_handler& handler) -> bool
{
	if (is_self(msg.prefix))
		jchannels_.insert(msg.get(0));

	handler({}, join_event{shared_from_this(), msg.prefix, msg.get(0)});

	return true;
}

auto server::dispatch_kick(const irc::message& msg, const recv_handler& handler) -> bool
{
	if (is_self(msg.get(1))) {
		// Remove the channel from the joined list.
		jchannels_.erase(msg.get(0));

		// Rejoin the channel if the option has been set and I was kicked.
		if ((options_ & options::auto_rejoin) == options::auto_rejoin)
			join(msg.get(0));
	}

	handler({}, kick_event{shared_from_this(), msg.prefix, msg.get(0), msg.get(1), msg.get(2)});

	return true;
}

auto server::dispatch_mode(const irc::message& msg, const recv_handler& handler) -> bool
{
	handler({}, mode_event{
		shared_from_this(),
		msg.prefix,
		msg.get(0),
		msg.get(1),
		msg.get(2),
		msg.get(3),
		msg.get(4)
	});

	return true;
}

auto server::dispatch_namreply(const irc::message& msg) -> bool
{
	/*
	 * Called multiple times to list clients on a channel.
	 *
	 * params[0] == originator
	 * params[1] == '='
	 * params[2] == channel
	 * params[3] == list of users with their prefixes
	 *
	 * IDEA for the future: maybe give the appropriate mode as a second
	 * parameter in onNames.
	 */
	if (msg.args.size() < 4 || msg.get(2) == "" || msg.get(3) == "")
		return false;

	auto users = string_util::split(msg.get(3), " \t");

	// The listing may add some prefixes, remove them if needed.
	for (const auto& u : users)
		names_map_[msg.get(2)].insert(clean_prefix(modes_, u));

	return false;
}

auto server::dispatch_nick(const irc::message& msg, const recv_handler& handler) -> bool
{
	// Update our nickname.
	if (is_self(msg.prefix))
		nickname_ = msg.get(0);

	handler({}, nick_event{shared_from_this(), msg.prefix, msg.get(0)});

	return true;
}

auto server::dispatch_notice(const irc::message& msg, const recv_handler& handler) -> bool
{
	handler({}, notice_event{shared_from_this(), msg.prefix, msg.get(0), msg.get(1)});

	return true;
}

auto server::dispatch_part(const irc::message& msg, const recv_handler& handler) -> bool
{
	// Remove the channel from the joined list if I left a channel.
	if (is_self(msg.prefix))
		jchannels_.erase(msg.get(1));

	handler({}, part_event{shared_from_this(), msg.prefix, msg.get(0), msg.get(1)});

	return true;
}

auto server::dispatch_ping(const irc::message& msg) -> bool
{
	assert(msg.command == "PING");

	send(str(format("PONG %1%") % msg.get(0)));

	return false;
}

auto server::dispatch_privmsg(const irc::message& msg, const recv_handler& handler) -> bool
{
	assert(msg.command == "PRIVMSG");

	if (msg.is_ctcp(1)) {
		const auto cmd = msg.ctcp(1);

		if (cmd.compare(0, 6, "ACTION") == 0)
			handler({}, me_event{shared_from_this(), msg.prefix, msg.get(0), cmd.substr(7)});
		else if (cmd.compare(0, 7, "VERSION") == 0 && !ctcpversion_.empty()) {
			send(str(format("NOTICE %s :\x01VERSION %s\x01") % msg.prefix % ctcpversion_));
			return false;
		} else
			return false;
	} else
		handler({}, message_event{shared_from_this(), msg.prefix, msg.get(0), msg.get(1)});

	return true;
}

auto server::dispatch_topic(const irc::message& msg, const recv_handler& handler) -> bool
{
	assert(msg.command == "TOPIC");

	handler({}, topic_event{shared_from_this(), msg.get(0), msg.get(1), msg.get(2)});

	return true;
}

auto server::dispatch_whoischannels(const irc::message& msg) -> bool
{
	/*
	 * Called when we have received channels for one user.
	 *
	 * params[0] == originator
	 * params[1] == nickname
	 * params[2] == list of channels with their prefixes
	 */
	if (msg.args.size() < 3 || msg.get(1) == "" || msg.get(2) == "")
		return false;

	auto it = whois_map_.find(msg.get(1));

	if (it != whois_map_.end()) {
		auto channels = string_util::split(msg.get(2), " \t");

		// Clean their prefixes.
		for (auto& s : channels)
			s = clean_prefix(modes_, s);

		it->second.channels = std::move(channels);
	}

	return false;
}

auto server::dispatch_whoisuser(const irc::message& msg) -> bool
{
	/*
	 * Called when whois information has been partially received.
	 *
	 * params[0] == originator
	 * params[1] == nickname
	 * params[2] == username
	 * params[3] == hostname
	 * params[4] == * (no idea what is that)
	 * params[5] == realname
	 */
	if (msg.args.size() < 6 || msg.get(1) == "" || msg.get(2) == "" || msg.get(3) == "" || msg.get(5) == "")
		return false;

	whois_info info;

	info.nick = msg.get(1);
	info.user = msg.get(2);
	info.hostname = msg.get(3);
	info.realname = msg.get(5);

	whois_map_.emplace(info.nick, info);

	return false;
}

auto server::dispatch(const irc::message& message, const recv_handler& handler) -> bool
{
	bool handled = false;

	if (message.is(5))
		handled = dispatch_isupport(message);
	else if (message.is(irc::err::nomotd) || message.is(irc::rpl::endofmotd))
		handled = dispatch_connect(message, handler);
	else if (message.command == "INVITE")
		handled = dispatch_invite(message, handler);
	else if (message.command == "JOIN")
		handled = dispatch_join(message, handler);
	else if (message.command == "KICK")
		handled = dispatch_kick(message, handler);
	else if (message.command == "MODE")
		handled = dispatch_mode(message, handler);
	else if (message.command == "NICK")
		handled = dispatch_nick(message, handler);
	else if (message.command == "NOTICE")
		handled = dispatch_notice(message, handler);
	else if (message.command == "TOPIC")
		handled = dispatch_topic(message, handler);
	else if (message.command == "PART")
		handled = dispatch_part(message, handler);
	else if (message.command == "PING")
		handled = dispatch_ping(message);
	else if (message.command == "PRIVMSG")
		handled = dispatch_privmsg(message, handler);
	else if (message.is(irc::rpl::namreply))
		handled = dispatch_namreply(message);
	else if (message.is(irc::rpl::endofnames))
		handled = dispatch_endofnames(message, handler);
	else if (message.is(irc::rpl::endofwhois))
		handled = dispatch_endofwhois(message, handler);
	else if (message.is(irc::rpl::whoischannels))
		handled = dispatch_whoischannels(message);
	else if (message.is(irc::rpl::whoisuser))
		handled = dispatch_whoisuser(message);

	return handled;
}

void server::handle_send(const std::error_code& code)
{
	/*
	 * We don't notify server_service in case of error because in any case the
	 * pending recv() will complete with an error.
	 */
	queue_.pop_front();

	if (!code)
		flush();
}

void server::handle_recv(const std::error_code& code,
                         const irc::message& message,
                         const recv_handler& handler)
{
	/*
	 * Once a message is received, dispatch it to individual dispatch_*
	 * functions. If the function calls handler by itself it returns true
	 * otherwise we call handler with no event to tell the caller the message
	 * has arrived and allowed to call recv() again.
	 */
	if (code) {
		disconnect();
		handler(std::move(code), std::monostate{});
	} else if (!dispatch(message, handler))
		handler({}, std::monostate{});
}

void server::recv(recv_handler handler) noexcept
{
	assert(state_ == state::identifying || state_ == state::connected);

	const auto self = shared_from_this();

	timer_.expires_from_now(boost::posix_time::seconds(timeout_));
	timer_.async_wait([this, handler, self, c = conn_] (auto code) {
		if (c != conn_)
			return;

		if (!code) {
			disconnect();
			handler(make_error_code(std::errc::timed_out), std::monostate{});
		}
	});

	conn_->recv([this, handler, self, c = conn_] (auto code, auto message) {
		if (c != conn_)
			return;

		handle_recv(std::move(code), message, handler);
	});
}

void server::flush()
{
	if (!conn_ || queue_.empty())
		return;

	const auto self = shared_from_this();

	conn_->send(queue_.front(), [this, self, c = conn_] (auto code) {
		if (c != conn_)
			return;

		handle_send(std::move(code));
	});
}

void server::identify()
{
	state_ = state::identifying;

	if (!password_.empty())
		send(str(format("PASS %1%") % password_));

	send(str(format("NICK %1%") % nickname_));
	send(str(format("USER %1% unknown unknown :%2%") % username_ % realname_));
}

void server::handle_connect(const std::error_code& code, const connect_handler& handler)
{
	timer_.cancel();

	if (code)
		disconnect();
	else
		identify();

	handler(code);
}

server::server(boost::asio::io_service& service, std::string id, std::string hostname)
	: id_(std::move(id))
	, hostname_(std::move(hostname))
	, options_(options::ipv4 | options::ipv6)
	, service_(service)
	, timer_(service)
{
	assert(!hostname_.empty());
}

server::~server()
{
	conn_ = nullptr;
	state_ = state::disconnected;
}

auto server::get_state() const noexcept -> state
{
	return state_;
}

auto server::get_id() const noexcept -> const std::string&
{
	return id_;
}

auto server::get_hostname() const noexcept -> const std::string&
{
	return hostname_;
}

auto server::get_password() const noexcept -> const std::string&
{
	return password_;
}

void server::set_password(std::string password) noexcept
{
	password_ = std::move(password);
}

auto server::get_port() const noexcept -> std::uint16_t
{
	return port_;
}

void server::set_port(std::uint16_t port) noexcept
{
	port_ = port;
}

auto server::get_options() const noexcept -> options
{
	return options_;
}

void server::set_options(options flags) noexcept
{
#if !defined(IRCCD_HAVE_SSL)
	assert((flags & options::ssl) != options::ssl);
#endif

	options_ = flags;
}

auto server::get_nickname() const noexcept -> const std::string&
{
	return nickname_;
}

void server::set_nickname(std::string nickname)
{
	if (state_ == state::connected)
		send(str(format("NICK %1%") % nickname));
	else
		nickname_ = std::move(nickname);
}

auto server::get_username() const noexcept -> const std::string&
{
	return username_;
}

void server::set_username(std::string name) noexcept
{
	username_ = std::move(name);
}

auto server::get_realname() const noexcept -> const std::string&
{
	return realname_;
}

void server::set_realname(std::string realname) noexcept
{
	realname_ = std::move(realname);
}

auto server::get_ctcp_version() const noexcept -> const std::string&
{
	return ctcpversion_;
}

void server::set_ctcp_version(std::string ctcpversion)
{
	ctcpversion_ = std::move(ctcpversion);
}

auto server::get_command_char() const noexcept -> const std::string&
{
	return command_char_;
}

void server::set_command_char(std::string command_char) noexcept
{
	assert(!command_char.empty());

	command_char_ = std::move(command_char);
}

auto server::get_reconnect_delay() const noexcept -> std::uint16_t
{
	return recodelay_;
}

void server::set_reconnect_delay(std::uint16_t reconnect_delay) noexcept
{
	recodelay_ = reconnect_delay;
}

auto server::get_ping_timeout() const noexcept -> std::uint16_t
{
	return timeout_;
}

void server::set_ping_timeout(std::uint16_t ping_timeout) noexcept
{
	timeout_ = ping_timeout;
}

auto server::get_channels() const noexcept -> const std::set<std::string>&
{
	return jchannels_;
}

auto server::is_self(std::string_view target) const noexcept -> bool
{
	return nickname_ == irc::user::parse(target).nick;
}

void server::connect(connect_handler handler) noexcept
{
	assert(state_ == state::disconnected);
	assert((options_ & options::ipv4) == options::ipv4 || (options_ & options::ipv6) == options::ipv6);

	/*
	 * This is needed if irccd is started before DHCP or if DNS cache is
	 * outdated.
	 */
#if !BOOST_OS_WINDOWS
	(void)res_init();
#endif

	conn_ = std::make_unique<irc::connection>(service_);
	conn_->use_ssl((options_ & options::ssl) == options::ssl);
	conn_->use_ipv4((options_ & options::ipv4) == options::ipv4);
	conn_->use_ipv6((options_ & options::ipv6) == options::ipv6);

	jchannels_.clear();
	state_ = state::connecting;

	timer_.expires_from_now(boost::posix_time::seconds(timeout_));
	timer_.async_wait([this, handler, c = conn_] (auto code) {
		if (c != conn_)
			return;

		if (!code) {
			disconnect();
			handler(make_error_code(std::errc::timed_out));
		}
	});

	const auto self = shared_from_this();

	conn_->connect(hostname_, std::to_string(port_), [this, handler, c = conn_] (auto code) {
		if (c != conn_)
			return;

		handle_connect(code, handler);
	});
}

void server::disconnect()
{
	state_ = state::disconnected;

	if (conn_) {
		conn_->disconnect();
		conn_ = nullptr;
	}

	timer_.cancel();
	queue_.clear();
}

void server::wait(connect_handler handler)
{
	assert(state_ == state::disconnected);

	const auto self = shared_from_this();

	timer_.expires_from_now(boost::posix_time::seconds(recodelay_));
	timer_.async_wait([this, handler, self, c = conn_] (auto code) {
		if (code != boost::asio::error::operation_aborted)
			handler({});
	});
}

void server::invite(std::string_view target, std::string_view channel)
{
	assert(!target.empty());
	assert(!channel.empty());

	send(str(format("INVITE %1% %2%") % target % channel));
}

void server::join(std::string_view channel, std::string_view password)
{
	assert(!channel.empty());

	auto it = std::find_if(rchannels_.begin(), rchannels_.end(), [&] (const auto& c) {
		return c.name == channel;
	});

	if (it == rchannels_.end())
		rchannels_.push_back({ std::string(channel), std::string(password) });
	else
		*it = { std::string(channel), std::string(password) };

	if (state_ == state::connected) {
		if (password.empty())
			send(str(format("JOIN %1%") % channel));
		else
			send(str(format("JOIN %1% :%2%") % channel % password));
	}
}

void server::kick(std::string_view target, std::string_view channel, std::string_view reason)
{
	assert(!target.empty());
	assert(!channel.empty());

	if (!reason.empty())
		send(str(format("KICK %1% %2% :%3%") % channel % target % reason));
	else
		send(str(format("KICK %1% %2%") % channel % target));
}

void server::me(std::string_view target, std::string_view message)
{
	assert(!target.empty());
	assert(!message.empty());

	send(str(format("PRIVMSG %1% :\x01" "ACTION %2%\x01") % target % message));
}

void server::message(std::string_view target, std::string_view message)
{
	assert(!target.empty());
	assert(!message.empty());

	send(str(format("PRIVMSG %1% :%2%") % target % message));
}

void server::mode(std::string_view channel,
                  std::string_view mode,
                  std::string_view limit,
                  std::string_view user,
                  std::string_view mask)
{
	assert(!channel.empty());
	assert(!mode.empty());

	std::ostringstream oss;

	oss << "MODE " << channel << " " << mode;

	if (!limit.empty())
		oss << " " << limit;
	if (!user.empty())
		oss << " " << user;
	if (!mask.empty())
		oss << " " << mask;

	send(oss.str());
}

void server::names(std::string_view channel)
{
	assert(!channel.empty());

	send(str(format("NAMES %1%") % channel));
}

void server::notice(std::string_view target, std::string_view message)
{
	assert(!target.empty());
	assert(!message.empty());

	send(str(format("NOTICE %1% :%2%") % target % message));
}

void server::part(std::string_view channel, std::string_view reason)
{
	assert(!channel.empty());

	if (!reason.empty())
		send(str(format("PART %1% :%2%") % channel % reason));
	else
		send(str(format("PART %1%") % channel));
}

void server::send(std::string_view raw)
{
	assert(!raw.empty());

	if (state_ == state::identifying || state_ == state::connected) {
		const auto in_progress = queue_.size() > 0;

		queue_.push_back(std::string(raw));

		if (!in_progress)
			flush();
	} else
		queue_.push_back(std::string(raw));
}

void server::topic(std::string_view channel, std::string_view topic)
{
	assert(!channel.empty());

	if (!topic.empty())
		send(str(format("TOPIC %1% :%2%") % channel % topic));
	else
		send(str(format("TOPIC %1%") % channel));
}

void server::whois(std::string_view target)
{
	assert(!target.empty());

	send(str(format("WHOIS %1% %2%") % target % target));
}

server_error::server_error(error code) noexcept
	: system_error(make_error_code(code))
{
}

auto server_category() -> const std::error_category&
{
	static const class category : public std::error_category {
	public:
		auto name() const noexcept -> const char* override
		{
			return "server";
		}

		auto message(int e) const -> std::string override
		{
			switch (static_cast<server_error::error>(e)) {
			case server_error::not_found:
				return "server not found";
			case server_error::invalid_identifier:
				return "invalid server identifier";
			case server_error::not_connected:
				return "server is not connected";
			case server_error::already_connected:
				return "server is already connected";
			case server_error::already_exists:
				return "server already exists";
			case server_error::invalid_port:
				return "invalid port number specified";
			case server_error::invalid_reconnect_delay:
				return "invalid reconnect delay number";
			case server_error::invalid_hostname:
				return "invalid hostname";
			case server_error::invalid_channel:
				return "invalid or empty channel";
			case server_error::invalid_mode:
				return "invalid or empty mode";
			case server_error::invalid_nickname:
				return "invalid nickname";
			case server_error::invalid_username:
				return "invalid username";
			case server_error::invalid_realname:
				return "invalid realname";
			case server_error::invalid_password:
				return "invalid password";
			case server_error::invalid_ping_timeout:
				return "invalid ping timeout";
			case server_error::invalid_ctcp_version:
				return "invalid CTCP VERSION";
			case server_error::invalid_command_char:
				return "invalid character command";
			case server_error::invalid_message:
				return "invalid message";
			case server_error::ssl_disabled:
				return "ssl is not enabled";
			case server_error::invalid_family:
				return "invalid family";
			default:
				return "no error";
			}
		}
	} category;

	return category;
}

auto make_error_code(server_error::error e) -> std::error_code
{
	return { static_cast<int>(e), server_category() };
}

} // !irccd::daemon
