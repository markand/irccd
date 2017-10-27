/*
 * server.cpp -- an IRC server
 *
 * Copyright (c) 2013-2017 David Demelier <markand@malikania.fr>
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

#include <libircclient.h>
#include <libirc_rfcnumeric.h>

#include "sysconfig.hpp"

#if !defined(IRCCD_SYSTEM_WINDOWS)
#  include <sys/types.h>
#  include <netinet/in.h>
#  include <arpa/nameser.h>
#  include <resolv.h>
#endif

#include "json_util.hpp"
#include "logger.hpp"
#include "server.hpp"
#include "string_util.hpp"
#include "system.hpp"

namespace irccd {

/*
 * server::session declaration.
 * ------------------------------------------------------------------
 */

class server::session {
public:
    std::unique_ptr<irc_session_t, void (*)(irc_session_t *)> handle_{nullptr, nullptr};

    inline operator const irc_session_t*() const noexcept
    {
        return handle_.get();
    }

    inline operator irc_session_t*() noexcept
    {
        return handle_.get();
    }

    inline bool is_connected() const noexcept
    {
        return irc_is_connected(handle_.get());
    }
};

/*
 * server::state declaration.
 * ------------------------------------------------------------------
 */

class server::state {
public:
    state() = default;
    virtual ~state() = default;
    virtual void prepare(server&, fd_set&, fd_set&, net::Handle&) = 0;
    virtual std::string ident() const = 0;
};

/*
 * server::disconnected_state declaration.
 * ------------------------------------------------------------------
 */

class server::disconnected_state : public server::state {
private:
    boost::timer::cpu_timer timer_;

public:
    void prepare(server&, fd_set&, fd_set&, net::Handle&) override;
    std::string ident() const override;
};

/*
 * server::connecting_state declaration.
 * ------------------------------------------------------------------
 */

class server::connecting_state : public state {
private:
    enum {
        disconnected,
        connecting
    } state_{disconnected};

    boost::timer::cpu_timer timer_;

    bool connect(server& server);

public:
    void prepare(server&, fd_set&, fd_set&, net::Handle&) override;
    std::string ident() const override;
};

/*
 * server::connected_state declaration.
 * ------------------------------------------------------------------
 */

class server::connected_state : public state {
public:
    void prepare(server&, fd_set&, fd_set&, net::Handle&) override;
    std::string ident() const override;
};

namespace {

/*
 * strify
 * ------------------------------------------------------------------
 *
 * Make sure to build a C++ string with a not-null C string.
 */
inline std::string strify(const char* s)
{
    return (s == nullptr) ? "" : std::string(s);
}

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
 * extract_prefixes
 * ------------------------------------------------------------------
 *
 * Read modes from the IRC event numeric.
 */
std::map<channel_mode, char> extract_prefixes(const std::string& line)
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

void server::handle_connect(const char*, const char**) noexcept
{
    // Reset the number of tried reconnection.
    recocur_ = 1;

    // Reset the timer.
    timer_.start();

    // Reset joined channels.
    jchannels_.clear();

    // Don't forget to change state and notify.
    next(std::make_unique<connected_state>());
    on_connect(connect_event{shared_from_this()});

    // Auto join listed channels.
    for (const auto& channel : rchannels_) {
        log::info() << "server " << name_ << ": auto joining " << channel.name << std::endl;
        join(channel.name, channel.password);
    }
}

void server::handle_channel(const char* orig, const char** params) noexcept
{
    on_message({shared_from_this(), strify(orig), strify(params[0]), strify(params[1])});
}

void server::handle_channel_mode(const char* orig, const char** params) noexcept
{
    on_channel_mode({
        shared_from_this(),
        strify(orig),
        strify(params[0]),
        strify(params[1]),
        strify(params[2])
    });
}

void server::handle_channel_notice(const char* orig, const char** params) noexcept
{
    on_channel_notice({
        shared_from_this(),
        strify(orig),
        strify(params[0]),
        strify(params[1])
    });
}

void server::handle_ctcp_action(const char* orig, const char** params) noexcept
{
    on_me({shared_from_this(), strify(orig), strify(params[0]), strify(params[1])});
}

void server::handle_invite(const char* orig, const char** params) noexcept
{
    // If joininvite is set, join the channel.
    if ((flags_ & join_invite) && is_self(strify(params[0])))
        join(strify(params[1]));

    /*
     * The libircclient says that invite contains the target nickname, it's
     * quit uncommon to need it so it is passed as the last argument to be
     * optional in the plugin.
     */
    on_invite({shared_from_this(), strify(orig), strify(params[1]), strify(params[0])});
}

void server::handle_join(const char* orig, const char** params) noexcept
{
    if (is_self(strify(orig)))
        jchannels_.push_back(strify(params[0]));

    on_join({shared_from_this(), strify(orig), strify(params[0])});
}

void server::handle_kick(const char* orig, const char** params) noexcept
{
    if (is_self(strify(params[1]))) {
        // Remove the channel from the joined list.
        remove_joined_channel(strify(params[0]));

        // Rejoin the channel if the option has been set and I was kicked.
        if (flags_ & auto_rejoin)
            join(strify(params[0]));
    }

    on_kick({
        shared_from_this(),
        strify(orig),
        strify(params[0]),
        strify(params[1]),
        strify(params[2])
    });
}

void server::handle_mode(const char* orig, const char** params) noexcept
{
    on_mode({shared_from_this(), strify(orig), strify(params[1])});
}

void server::handle_nick(const char* orig, const char** params) noexcept
{
    // Update our nickname.
    if (is_self(strify(orig)))
        nickname_ = strify(params[0]);

    on_nick({shared_from_this(), strify(orig), strify(params[0])});
}

void server::handle_notice(const char* orig, const char** params) noexcept
{
    /*
     * Like handleInvite, the notice provides the target nickname, we discard
     * it.
     */
    on_notice({shared_from_this(), strify(orig), strify(params[1])});
}

void server::handle_numeric(unsigned int event, const char** params, unsigned int c) noexcept
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
         * IDEA for the future: maybe give the appropriate mode as a second
         * parameter in onNames.
         */
        if (c < 4 || params[2] == nullptr || params[3] == nullptr)
            return;

        auto users = string_util::split(params[3], " \t");

        // The listing may add some prefixes, remove them if needed.
        for (auto u : users)
            names_map_[params[2]].insert(clean_prefix(modes_, u));
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

        auto it = names_map_.find(params[1]);
        if (it != names_map_.end()) {
            on_names({
                shared_from_this(),
                params[1],
                std::vector<std::string>(it->second.begin(), it->second.end())
            });

            // Don't forget to remove the list.
            names_map_.erase(it);
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

        class whois info;

        info.nick = strify(params[1]);
        info.user = strify(params[2]);
        info.host = strify(params[3]);
        info.realname = strify(params[5]);

        whois_map_.emplace(info.nick, info);
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

        auto it = whois_map_.find(params[1]);
        if (it != whois_map_.end()) {
            auto channels = string_util::split(params[2], " \t");

            // Clean their prefixes.
            for (auto &s : channels)
                s = clean_prefix(modes_, s);

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
        auto it = whois_map_.find(params[1]);
        if (it != whois_map_.end()) {
            on_whois({shared_from_this(), it->second});

            // Don't forget to remove.
            whois_map_.erase(it);
        }
    } else if (event == /* RPL_BOUNCE */ 5) {
        /*
         * The event 5 is usually RPL_BOUNCE, but we always see it as ISUPPORT.
         */
        for (unsigned int i = 0; i < c; ++i) {
            if (strncmp(params[i], "PREFIX", 6) == 0) {
                modes_ = extract_prefixes(params[i]);
                break;
            }
        }
    }
}

void server::handle_part(const char* orig, const char** params) noexcept
{
    // Remove the channel from the joined list if I left a channel.
    if (is_self(strify(orig)))
        remove_joined_channel(strify(params[0]));

    on_part({shared_from_this(), strify(orig), strify(params[0]), strify(params[1])});
}

void server::handle_ping(const char*, const char**) noexcept
{
    // Reset the timer to detect disconnection.
    timer_.start();
}

void server::handle_query(const char* orig, const char** params) noexcept
{
    on_query({shared_from_this(), strify(orig), strify(params[1])});
}

void server::handle_topic(const char* orig, const char** params) noexcept
{
    on_topic({shared_from_this(), strify(orig), strify(params[0]), strify(params[1])});
}

std::shared_ptr<server> server::from_json(const nlohmann::json& object)
{
    auto sv = std::make_shared<server>(json_util::require_identifier(object, "name"));

    sv->set_host(json_util::require_string(object, "host"));
    sv->set_password(json_util::get_string(object, "password"));
    sv->set_nickname(json_util::get_string(object, "nickname", sv->nickname()));
    sv->set_realname(json_util::get_string(object, "realname", sv->realname()));
    sv->set_username(json_util::get_string(object, "username", sv->username()));
    sv->set_ctcp_version(json_util::get_string(object, "ctcpVersion", sv->ctcp_version()));
    sv->set_command_char(json_util::get_string(object, "commandChar", sv->command_char()));

    if (object.find("port") != object.end())
        sv->set_port(json_util::get_uint(object, "port"));
    if (json_util::get_bool(object, "ipv6"))
        sv->set_flags(sv->flags() | server::ipv6);
    if (json_util::get_bool(object, "ssl"))
        sv->set_flags(sv->flags() | server::ssl);
    if (json_util::get_bool(object, "sslVerify"))
        sv->set_flags(sv->flags() | server::ssl_verify);
    if (json_util::get_bool(object, "autoRejoin"))
        sv->set_flags(sv->flags() | server::auto_rejoin);
    if (json_util::get_bool(object, "joinInvite"))
        sv->set_flags(sv->flags() | server::join_invite);

    return sv;
}

channel server::split_channel(const std::string& value)
{
    auto pos = value.find(':');

    if (pos != std::string::npos)
        return {value.substr(0, pos), value.substr(pos + 1)};

    return {value, ""};
}

server::server(std::string name)
    : name_(std::move(name))
    , session_(std::make_unique<session>())
    , state_(std::make_unique<connecting_state>())
{
    // Initialize nickname and username.
    auto user = sys::username();

    nickname_ = user.empty() ? "irccd" : user;
    username_ = user.empty() ? "irccd" : user;

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

    callbacks.event_channel = [] (irc_session_t* session, const char*, const char* orig, const char** params, unsigned int) {
        static_cast<server*>(irc_get_ctx(session))->handle_channel(orig, params);
    };
    callbacks.event_channel_notice = [] (irc_session_t* session, const char*, const char* orig, const char** params, unsigned int) {
        static_cast<server*>(irc_get_ctx(session))->handle_channel_notice(orig, params);
    };
    callbacks.event_connect = [] (irc_session_t* session, const char*, const char* orig, const char** params, unsigned int) {
        static_cast<server*>(irc_get_ctx(session))->handle_connect(orig, params);
    };
    callbacks.event_ctcp_action = [] (irc_session_t* session, const char*, const char* orig, const char** params, unsigned int) {
        static_cast<server*>(irc_get_ctx(session))->handle_ctcp_action(orig, params);
    };
    callbacks.event_invite = [] (irc_session_t* session, const char*, const char* orig, const char** params, unsigned int) {
        static_cast<server*>(irc_get_ctx(session))->handle_invite(orig, params);
    };
    callbacks.event_join = [] (irc_session_t* session, const char*, const char* orig, const char** params, unsigned int) {
        static_cast<server*>(irc_get_ctx(session))->handle_join(orig, params);
    };
    callbacks.event_kick = [] (irc_session_t* session, const char*, const char* orig, const char** params, unsigned int) {
        static_cast<server*>(irc_get_ctx(session))->handle_kick(orig, params);
    };
    callbacks.event_mode = [] (irc_session_t* session, const char*, const char* orig, const char** params, unsigned int) {
        static_cast<server*>(irc_get_ctx(session))->handle_channel_mode(orig, params);
    };
    callbacks.event_nick = [] (irc_session_t* session, const char*, const char* orig, const char** params, unsigned int) {
        static_cast<server*>(irc_get_ctx(session))->handle_nick(orig, params);
    };
    callbacks.event_notice = [] (irc_session_t* session, const char*, const char* orig, const char** params, unsigned int) {
        static_cast<server*>(irc_get_ctx(session))->handle_notice(orig, params);
    };
    callbacks.event_numeric = [] (irc_session_t* session, unsigned int event, const char*, const char** params, unsigned int count) {
        static_cast<server*>(irc_get_ctx(session))->handle_numeric(event, params, count);
    };
    callbacks.event_part = [] (irc_session_t* session, const char*, const char* orig, const char** params, unsigned int) {
        static_cast<server*>(irc_get_ctx(session))->handle_part(orig, params);
    };
    callbacks.event_ping = [] (irc_session_t* session, const char*, const char* orig, const char** params, unsigned int) {
        static_cast<server*>(irc_get_ctx(session))->handle_ping(orig, params);
    };
    callbacks.event_privmsg = [] (irc_session_t* session, const char*, const char* orig, const char** params, unsigned int) {
        static_cast<server*>(irc_get_ctx(session))->handle_query(orig, params);
    };
    callbacks.event_topic = [] (irc_session_t* session, const char*, const char* orig, const char** params, unsigned int) {
        static_cast<server*>(irc_get_ctx(session))->handle_topic(orig, params);
    };
    callbacks.event_umode = [] (irc_session_t* session, const char*, const char* orig, const char** params, unsigned int) {
        static_cast<server*>(irc_get_ctx(session))->handle_mode(orig, params);
    };

    session_->handle_ = {irc_create_session(&callbacks), irc_destroy_session};

    // Save this to the session.
    irc_set_ctx(*session_, this);
    irc_set_ctcp_version(*session_, ctcpversion_.c_str());
}

server::~server()
{
    irc_disconnect(*session_);
}

void server::set_nickname(std::string nickname)
{
    if (session_->is_connected())
        queue_.push([=] () {
            return irc_cmd_nick(*session_, nickname.c_str()) == 0;
        });
    else
        nickname_ = std::move(nickname);
}

void server::set_ctcp_version(std::string ctcpversion)
{
    ctcpversion_ = std::move(ctcpversion);
    irc_set_ctcp_version(*session_, ctcpversion_.c_str());
}

void server::next(std::unique_ptr<state> state) noexcept
{
    state_next_ = std::move(state);
}

std::string server::status() const noexcept
{
    return state_ ? state_->ident() : "null";
}

void server::update() noexcept
{
    if (state_next_) {
        log::debug(string_util::sprintf("server %s: switch state %s -> %s",
            name_, state_->ident(), state_next_->ident()));

        state_ = std::move(state_next_);
        state_next_ = nullptr;

        // Reset channels.
        jchannels_.clear();
    }
}

void server::disconnect() noexcept
{
    using namespace std::placeholders;

    irc_disconnect(*session_);
    on_die();
}

void server::reconnect() noexcept
{
    irc_disconnect(*session_);
    next(std::make_unique<connecting_state>());
}

void server::prepare(fd_set& setinput, fd_set& setoutput, net::Handle& maxfd) noexcept
{
    state_->prepare(*this, setinput, setoutput, maxfd);
}

void server::sync(fd_set &setinput, fd_set &setoutput)
{
    /*
     * 1. Send maximum of command possible if available for write
     *
     * Break on the first failure to avoid changing the order of the
     * commands if any of them fails.
     */
    bool done = false;

    while (!queue_.empty() && !done) {
        if (queue_.front()())
            queue_.pop();
        else
            done = true;
    }

    // 2. Read data.
    irc_process_select_descriptors(*session_, &setinput, &setoutput);
}

bool server::is_self(const std::string& nick) const noexcept
{
    char target[32]{0};

    irc_target_get_nick(nick.c_str(), target, sizeof (target));

    return nickname_ == target;
}

void server::cmode(std::string channel, std::string mode)
{
    queue_.push([=] () {
        return irc_cmd_channel_mode(*session_, channel.c_str(), mode.c_str()) == 0;
    });
}

void server::cnotice(std::string channel, std::string message)
{
    queue_.push([=] () {
        return irc_cmd_notice(*session_, channel.c_str(), message.c_str()) == 0;
    });
}

void server::invite(std::string target, std::string channel)
{
    queue_.push([=] () {
        return irc_cmd_invite(*session_, target.c_str(), channel.c_str()) == 0;
    });
}

void server::join(std::string channel, std::string password)
{
    // 1. Add the channel or update it to the requested channels.
    auto it = std::find_if(rchannels_.begin(), rchannels_.end(), [&] (const auto& c) {
        return c.name == channel;
    });

    if (it == rchannels_.end())
        rchannels_.push_back({ channel, password });
    else
        *it = { channel, password };

    // 2. Join if not found and connected.
    if (session_->is_connected())
        irc_cmd_join(*session_, channel.c_str(), password.empty() ? nullptr : password.c_str());
}

void server::kick(std::string target, std::string channel, std::string reason)
{
    queue_.push([=] () {
        return irc_cmd_kick(*session_, target.c_str(), channel.c_str(), reason.c_str()) == 0;
    });
}

void server::me(std::string target, std::string message)
{
    queue_.push([=] () {
        return irc_cmd_me(*session_, target.c_str(), message.c_str()) == 0;
    });
}

void server::message(std::string target, std::string message)
{
    queue_.push([=] () {
        return irc_cmd_msg(*session_, target.c_str(), message.c_str()) == 0;
    });
}

void server::mode(std::string mode)
{
    queue_.push([=] () {
        return irc_cmd_user_mode(*session_, mode.c_str()) == 0;
    });
}

void server::names(std::string channel)
{
    queue_.push([=] () {
        return irc_cmd_names(*session_, channel.c_str()) == 0;
    });
}

void server::notice(std::string target, std::string message)
{
    queue_.push([=] () {
        return irc_cmd_notice(*session_, target.c_str(), message.c_str()) == 0;
    });
}

void server::part(std::string channel, std::string reason)
{
    queue_.push([=] () -> bool {
        if (reason.empty())
            return irc_cmd_part(*session_, channel.c_str()) == 0;

        return irc_send_raw(*session_, "PART %s :%s", channel.c_str(), reason.c_str());
    });
}

void server::send(std::string raw)
{
    queue_.push([=] () {
        return irc_send_raw(*session_, raw.c_str()) == 0;
    });
}

void server::topic(std::string channel, std::string topic)
{
    queue_.push([=] () {
        return irc_cmd_topic(*session_, channel.c_str(), topic.c_str()) == 0;
    });
}

void server::whois(std::string target)
{
    queue_.push([=] () {
        return irc_cmd_whois(*session_, target.c_str()) == 0;
    });
}

/*
 * server::disconnected_state implementation
 * ------------------------------------------------------------------
 */

void server::disconnected_state::prepare(server& server, fd_set&, fd_set&, net::Handle&)
{
    if (server.recotries_ == 0) {
        log::warning() << "server " << server.name_ << ": reconnection disabled, skipping" << std::endl;
        server.on_die();
    } else if (server.recotries_ > 0 && server.recocur_ > server.recotries_) {
        log::warning() << "server " << server.name_ << ": giving up" << std::endl;
        server.on_die();
    } else {
        if (timer_.elapsed().wall / 1000000LL > static_cast<unsigned>(server.recodelay_ * 1000)) {
            irc_disconnect(*server.session_);

            server.recocur_ ++;
            server.next(std::make_unique<connecting_state>());
        }
    }
}

std::string server::disconnected_state::ident() const
{
    return "Disconnected";
}

/*
 * server::connecting_state implementation
 * ------------------------------------------------------------------
 */

bool server::connecting_state::connect(server& server)
{
    auto password = server.password_.empty() ? nullptr : server.password_.c_str();
    auto host = server.host_;

    // libircclient requires # for SSL connection.
#if defined(HAVE_SSL)
    if (server.flags_ & server::ssl)
        host.insert(0, 1, '#');
    if (!(server.flags_ & server::ssl_verify))
        irc_option_set(*server.session_, LIBIRC_OPTION_SSL_NO_VERIFY);
#endif

    int code;
    if (server.flags() & server::ipv6) {
        code = irc_connect6(*server.session_, host.c_str(), server.port_, password,
                            server.nickname_.c_str(),
                            server.username_.c_str(),
                            server.realname_.c_str());
    } else {
        code = irc_connect(*server.session_, host.c_str(), server.port_, password,
                           server.nickname_.c_str(),
                           server.username_.c_str(),
                           server.realname_.c_str());
    }

    return code == 0;
}

void server::connecting_state::prepare(server& server, fd_set& setinput, fd_set& setoutput, net::Handle& maxfd)
{
    /*
     * The connect function will either fail if the hostname wasn't resolved or
     * if any of the internal functions fail.
     *
     * It returns success if the connection was successful but it does not mean
     * that connection is established.
     *
     * Because this function will be called repeatidly, the connection was
     * started and we're still not connected in the specified timeout time, we
     * mark the server as disconnected.
     *
     * Otherwise, the libircclient event_connect will change the state.
     */
    if (state_ == connecting) {
        if (timer_.elapsed().wall / 1000000LL > static_cast<unsigned>(server.recodelay_ * 1000)) {
            log::warning() << "server " << server.name() << ": timeout while connecting" << std::endl;
            server.next(std::make_unique<disconnected_state>());
        } else if (!irc_is_connected(*server.session_)) {
            log::warning() << "server " << server.name_ << ": error while connecting: ";
            log::warning() << irc_strerror(irc_errno(*server.session_)) << std::endl;

            if (server.recotries_ != 0)
                log::warning(string_util::sprintf("server %s: retrying in %hu seconds", server.name_, server.recodelay_));

            server.next(std::make_unique<disconnected_state>());
        } else
            irc_add_select_descriptors(*server.session_, &setinput, &setoutput, reinterpret_cast<int*>(&maxfd));
    } else {
        /*
         * This is needed if irccd is started before DHCP or if DNS cache is
         * outdated.
         */
#if !defined(IRCCD_SYSTEM_WINDOWS)
        (void)res_init();
#endif
        log::info(string_util::sprintf("server %s: trying to connect to %s, port %hu", server.name_, server.host_, server.port_));

        if (!connect(server)) {
            log::warning() << "server " << server.name_ << ": disconnected while connecting: ";
            log::warning() << irc_strerror(irc_errno(*server.session_)) << std::endl;
            server.next(std::make_unique<disconnected_state>());
        } else {
            state_ = connecting;

            if (irc_is_connected(*server.session_))
                irc_add_select_descriptors(*server.session_, &setinput, &setoutput, reinterpret_cast<int*>(&maxfd));
        }
    }
}

std::string server::connecting_state::ident() const
{
    return "Connecting";
}

/*
 * server::connected_state implementation
 * ------------------------------------------------------------------
 */

void server::connected_state::prepare(server& server, fd_set& setinput, fd_set& setoutput, net::Handle& maxfd)
{
    if (!irc_is_connected(*server.session_)) {
        log::warning() << "server " << server.name_ << ": disconnected" << std::endl;

        if (server.recodelay_ > 0)
            log::warning(string_util::sprintf("server %s: retrying in %hu seconds", server.name_, server.recodelay_));

        server.next(std::make_unique<disconnected_state>());
    } else if (server.timer_.elapsed().wall / 1000000LL >= server.timeout_ * 1000) {
        log::warning() << "server " << server.name_ << ": ping timeout after "
                       << (server.timer_.elapsed().wall / 1000000000LL) << " seconds" << std::endl;
        server.next(std::make_unique<disconnected_state>());
    } else
        irc_add_select_descriptors(*server.session_, &setinput, &setoutput, reinterpret_cast<int*>(&maxfd));
}

std::string server::connected_state::ident() const
{
    return "Connected";
}

} // !irccd
