/*
 * server.cpp -- an IRC server
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

#include <boost/predef/os.h>

#include <irccd/sysconfig.hpp>

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <stdexcept>

#if !BOOST_OS_WINDOWS
#  include <sys/types.h>
#  include <netinet/in.h>
#  include <arpa/nameser.h>
#  include <resolv.h>
#endif

#include <irccd/json_util.hpp>
#include <irccd/string_util.hpp>
#include <irccd/system.hpp>

#include "server.hpp"

namespace irccd {

namespace {

/*
 * clean_prefix
 * ------------------------------------------------------------------
 *
 * Remove the user prefix only if it is present in the mode table, for example
 * removes @ from @irccd if and only if @ is a character mode (e.g. operator).
 */
std::string clean_prefix(const std::map<channel_mode, char>& modes, std::string nickname)
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
std::map<channel_mode, char> isupport_extract_prefixes(const std::string& line)
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

void server::remove_joined_channel(const std::string& channel)
{
    jchannels_.erase(std::remove(jchannels_.begin(), jchannels_.end(), channel), jchannels_.end());
}

server::server(boost::asio::io_service& service, std::string name, std::string host)
    : name_(std::move(name))
    , host_(std::move(host))
    , service_(service)
    , timer_(service)
{
    assert(!host_.empty());

    // Initialize nickname and username.
    auto user = sys::username();

    nickname_ = user.empty() ? "irccd" : user;
    username_ = user.empty() ? "irccd" : user;
}

void server::dispatch_connect(const irc::message&)
{
    state_ = state::connected;
    on_connect({shared_from_this()});

    for (const auto& channel : rchannels_)
        join(channel.name, channel.password);
}

void server::dispatch_endofnames(const irc::message& msg)
{
    /*
     * Called when end of name listing has finished on a channel.
     *
     * params[0] == originator
     * params[1] == channel
     * params[2] == End of NAMES list
     */
    if (msg.args().size() < 3 || msg.arg(1) == "")
        return;

    const auto it = names_map_.find(msg.arg(1));

    if (it != names_map_.end()) {
        std::vector<std::string> list(it->second.begin(), it->second.end());

        on_names({shared_from_this(), msg.arg(1), std::move(list)});

        // Don't forget to remove the list.
        names_map_.erase(it);
    }
}

void server::dispatch_endofwhois(const irc::message& msg)
{
    /*
     * Called when whois is finished.
     *
     * params[0] == originator
     * params[1] == nickname
     * params[2] == End of WHOIS list
     */
    const auto it = whois_map_.find(msg.arg(1));

    if (it != whois_map_.end()) {
        on_whois({shared_from_this(), it->second});

        // Don't forget to remove.
        whois_map_.erase(it);
    }
}

void server::dispatch_invite(const irc::message& msg)
{
    // If join-invite is set, join the channel.
    if ((flags_ & join_invite) && is_self(msg.arg(0)))
        join(msg.arg(1));

    on_invite({shared_from_this(), msg.prefix(), msg.arg(1), msg.arg(0)});
}

void server::dispatch_isupport(const irc::message& msg)
{
    for (unsigned int i = 0; i < msg.args().size(); ++i) {
        if (msg.arg(i).compare(0, 6, "PREFIX") == 0) {
            modes_ = isupport_extract_prefixes(msg.arg(i));
            break;
        }
    }
}

void server::dispatch_join(const irc::message& msg)
{
    if (is_self(msg.prefix()))
        jchannels_.push_back(msg.arg(0));

    on_join({shared_from_this(), msg.prefix(), msg.arg(0)});
}

void server::dispatch_kick(const irc::message& msg)
{
    if (is_self(msg.arg(1))) {
        // Remove the channel from the joined list.
        remove_joined_channel(msg.arg(0));

        // Rejoin the channel if the option has been set and I was kicked.
        if (flags_ & auto_rejoin)
            join(msg.arg(0));
    }

    on_kick({shared_from_this(), msg.prefix(), msg.arg(0), msg.arg(1), msg.arg(2)});
}

void server::dispatch_mode(const irc::message& msg)
{
    on_mode({
        shared_from_this(),
        msg.prefix(),
        msg.arg(0),
        msg.arg(1),
        msg.arg(2),
        msg.arg(3),
        msg.arg(4)
    });
}

void server::dispatch_namreply(const irc::message& msg)
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
    if (msg.args().size() < 4 || msg.arg(2) == "" || msg.arg(3) == "")
        return;

    auto users = string_util::split(msg.arg(3), " \t");

    // The listing may add some prefixes, remove them if needed.
    for (auto u : users)
        names_map_[msg.arg(2)].insert(clean_prefix(modes_, u));
}

void server::dispatch_nick(const irc::message& msg)
{
    // Update our nickname.
    if (is_self(msg.prefix()))
        nickname_ = msg.arg(0);

    on_nick({shared_from_this(), msg.prefix(), msg.arg(0)});
}

void server::dispatch_notice(const irc::message& msg)
{
    on_notice({shared_from_this(), msg.prefix(), msg.arg(0), msg.arg(1)});
}

void server::dispatch_part(const irc::message& msg)
{
    // Remove the channel from the joined list if I left a channel.
    if (is_self(msg.prefix()))
        remove_joined_channel(msg.arg(1));

    on_part({shared_from_this(), msg.prefix(), msg.arg(0), msg.arg(1)});
}

void server::dispatch_ping(const irc::message& msg)
{
    assert(msg.command() == "PING");

    send(string_util::sprintf("PONG %s", msg.arg(0)));
}

void server::dispatch_privmsg(const irc::message& msg)
{
    assert(msg.command() == "PRIVMSG");

    if (msg.is_ctcp(1)) {
        auto cmd = msg.ctcp(1);

        if (cmd.compare(0, 6, "ACTION") == 0)
            on_me({shared_from_this(), msg.prefix(), msg.arg(0), cmd.substr(7)});
    } else if (is_self(msg.arg(0)))
        on_query({shared_from_this(), msg.prefix(), msg.arg(1)});
    else
        on_message({shared_from_this(), msg.prefix(), msg.arg(0), msg.arg(1)});
}

void server::dispatch_topic(const irc::message& msg)
{
    assert(msg.command() == "TOPIC");

    on_topic({shared_from_this(), msg.arg(0), msg.arg(1), msg.arg(2)});
}

void server::dispatch_whoischannels(const irc::message& msg)
{
    /*
     * Called when we have received channels for one user.
     *
     * params[0] == originator
     * params[1] == nickname
     * params[2] == list of channels with their prefixes
     */
    if (msg.args().size() < 3 || msg.arg(1) == "" || msg.arg(2) == "")
        return;

    auto it = whois_map_.find(msg.arg(1));

    if (it != whois_map_.end()) {
        auto channels = string_util::split(msg.arg(2), " \t");

        // Clean their prefixes.
        for (auto& s : channels)
            s = clean_prefix(modes_, s);

        it->second.channels = std::move(channels);
    }
}

void server::dispatch_whoisuser(const irc::message& msg)
{
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
    if (msg.args().size() < 6 || msg.arg(1) == "" || msg.arg(2) == "" || msg.arg(3) == "" || msg.arg(5) == "")
        return;

    whois_info info;

    info.nick = msg.arg(1);
    info.user = msg.arg(2);
    info.host = msg.arg(3);
    info.realname = msg.arg(5);

    whois_map_.emplace(info.nick, info);
}

void server::dispatch(const irc::message& message)
{
    if (message.is(5))
        dispatch_isupport(message);
    else if (message.is(irc::err::nomotd) || message.is(irc::rpl::endofmotd))
        dispatch_connect(message);
    else if (message.command() == "INVITE")
        dispatch_invite(message);
    else if (message.command() == "JOIN")
        dispatch_join(message);
    else if (message.command() == "KICK")
        dispatch_kick(message);
    else if (message.command() == "MODE")
        dispatch_mode(message);
    else if (message.command() == "NICK")
        dispatch_nick(message);
    else if (message.command() == "NOTICE")
        dispatch_notice(message);
    else if (message.command() == "TOPIC")
        dispatch_topic(message);
    else if (message.command() == "PART")
        dispatch_part(message);
    else if (message.command() == "PING")
        dispatch_ping(message);
    else if (message.command() == "PRIVMSG")
        dispatch_privmsg(message);
    else if (message.is(irc::rpl::namreply))
        dispatch_namreply(message);
    else if (message.is(irc::rpl::endofnames))
        dispatch_endofnames(message);
    else if (message.is(irc::rpl::endofwhois))
        dispatch_endofwhois(message);
    else if (message.is(irc::rpl::whoischannels))
        dispatch_whoischannels(message);
    else if (message.is(irc::rpl::whoisuser))
        dispatch_whoisuser(message);
}

void server::recv()
{
    conn_->recv([this, conn = conn_] (auto code, auto message) {
        if (code)
            wait();
        else {
            dispatch(message);
            recv();
        }
    });
}

void server::flush()
{
    if (queue_.empty())
        return;

    conn_->send(queue_.front(), [this, conn = conn_] (auto code) {
        queue_.pop_front();

        if (code)
            wait();
        else
            flush();
    });
}

void server::identify()
{
    state_ = state::identifying;
    recocur_ = 0U;
    jchannels_.clear();

    if (!password_.empty())
        send(string_util::sprintf("PASS %s", password_));

    send(string_util::sprintf("NICK %s", nickname_));
    send(string_util::sprintf("USER %s unknown unknown :%s", username_, realname_));
}

void server::wait()
{
    /*
     * This function maybe called from a recv(), send() or even connect() call
     * so be sure to only wait one at a time.
     */
    if (state_ == state::waiting)
        return;

    state_ = state::waiting;
    timer_.expires_from_now(boost::posix_time::seconds(recodelay_));
    timer_.async_wait([this] (auto code) {
        if (code == boost::asio::error::operation_aborted)
            return;

        recocur_ ++;
        connect();
    });
}

void server::handle_connect(boost::system::error_code code)
{
    // Cancel connect timer.
    timer_.cancel();

    if (code) {
        // Wait before reconnecting.
        if (recotries_ != 0) {
            if (recotries_ > 0 && recocur_ >= recotries_) {
                disconnect();
            } else {
                state_ = state::waiting;
                wait();
            }
        } else
            disconnect();
    } else {
        identify();
        recv();
    }
}

server::~server()
{
    conn_ = nullptr;
    state_ = state::disconnected;
}

void server::set_nickname(std::string nickname)
{
    if (state_ == state::connected)
        send(string_util::sprintf("NICK %s", nickname));
    else
        nickname_ = std::move(nickname);
}

void server::set_ctcp_version(std::string ctcpversion)
{
    ctcpversion_ = std::move(ctcpversion);
}

void server::connect() noexcept
{
    assert(state_ == state::disconnected || state_ == state::waiting);

    /*
     * This is needed if irccd is started before DHCP or if DNS cache is
     * outdated.
     */
#if !BOOST_OS_WINDOWS
    (void)res_init();
#endif

    if (flags_ & ssl) {
#if defined(IRCCD_HAVE_SSL)
        conn_ = std::make_shared<irc::tls_connection>(service_);
#else
        /*
         * If SSL is not compiled in, the caller is responsible of not setting
         * the flag.
         */
        assert(!(flags_ & ssl));
#endif
    } else
        conn_ = std::make_shared<irc::ip_connection>(service_);

    state_ = state::connecting;
    conn_->connect(host_, std::to_string(port_), [this, conn = conn_] (auto code) {
        handle_connect(std::move(code));
    });
}

void server::disconnect() noexcept
{
    conn_ = nullptr;
    state_ = state::disconnected;
    on_disconnect({shared_from_this()});
}

void server::reconnect() noexcept
{
    disconnect();
    connect();
}

bool server::is_self(const std::string& target) const noexcept
{
    return nickname_ == irc::user::parse(target).nick();
}

void server::invite(std::string target, std::string channel)
{
    assert(!target.empty());
    assert(!channel.empty());

    send(string_util::sprintf("INVITE %s %s", target, channel));
}

void server::join(std::string channel, std::string password)
{
    assert(!channel.empty());

    auto it = std::find_if(rchannels_.begin(), rchannels_.end(), [&] (const auto& c) {
        return c.name == channel;
    });

    if (it == rchannels_.end())
        rchannels_.push_back({ channel, password });
    else
        *it = { channel, password };

    if (state_ == state::connected) {
        if (password.empty())
            send(string_util::sprintf("JOIN %s", channel));
        else
            send(string_util::sprintf("JOIN %s :%s", channel, password));
    }
}

void server::kick(std::string target, std::string channel, std::string reason)
{
    assert(!target.empty());
    assert(!channel.empty());

    if (!reason.empty())
        send(string_util::sprintf("KICK %s %s :%s", channel, target, reason));
    else
        send(string_util::sprintf("KICK %s %s", channel, target));
}

void server::me(std::string target, std::string message)
{
    assert(!target.empty());
    assert(!message.empty());

    send(string_util::sprintf("PRIVMSG %s :\x01" "ACTION %s\x01", target, message));
}

void server::message(std::string target, std::string message)
{
    assert(!target.empty());
    assert(!message.empty());

    send(string_util::sprintf("PRIVMSG %s :%s", target, message));
}

void server::mode(std::string channel,
                  std::string mode,
                  std::string limit,
                  std::string user,
                  std::string mask)
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

void server::names(std::string channel)
{
    assert(channel.c_str());

    send(string_util::sprintf("NAMES %s", channel));
}

void server::notice(std::string target, std::string message)
{
    assert(!target.empty());
    assert(!message.empty());

    send(string_util::sprintf("NOTICE %s :%s", target, message));
}

void server::part(std::string channel, std::string reason)
{
    assert(!channel.empty());

    if (!reason.empty())
        send(string_util::sprintf("PART %s :%s", channel, reason));
    else
        send(string_util::sprintf("PART %s", channel));
}

void server::send(std::string raw)
{
    assert(!raw.empty());

    if (state_ == state::identifying || state_ == state::connected) {
        const auto in_progress = queue_.size() > 0;

        queue_.push_back(std::move(raw));

        if (!in_progress)
            flush();
    } else
        queue_.push_back(std::move(raw));
}

void server::topic(std::string channel, std::string topic)
{
    assert(!channel.empty());

    if (!topic.empty())
        send(string_util::sprintf("TOPIC %s :%s", channel, topic));
    else
        send(string_util::sprintf("TOPIC %s", channel));
}

void server::whois(std::string target)
{
    assert(!target.empty());

    send(string_util::sprintf("WHOIS %s %s", target, target));
}

server_error::server_error(error code) noexcept
    : system_error(make_error_code(code))
{
}

const std::error_category& server_category()
{
    static const class category : public std::error_category {
    public:
        const char* name() const noexcept override
        {
            return "server";
        }

        std::string message(int e) const override
        {
            switch (static_cast<server_error::error>(e)) {
            case server_error::not_found:
                return "server not found";
            case server_error::invalid_identifier:
                return "invalid identifier";
            case server_error::not_connected:
                return "server is not connected";
            case server_error::already_connected:
                return "server is already connected";
            case server_error::already_exists:
                return "server already exists";
            case server_error::invalid_port:
                return "invalid port number specified";
            case server_error::invalid_reconnect_tries:
                return "invalid number of reconnection tries";
            case server_error::invalid_reconnect_timeout:
                return "invalid reconnect timeout number";
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
            default:
                return "no error";
            }
        }
    } category;

    return category;
}

std::error_code make_error_code(server_error::error e)
{
    return {static_cast<int>(e), server_category()};
}

} // !irccd
