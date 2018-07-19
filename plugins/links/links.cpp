/*
 * links.cpp -- links plugin
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

#include <memory>
#include <regex>
#include <sstream>
#include <string>
#include <variant>

#include <boost/dll.hpp>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

#include <boost/beast.hpp>

#include <irccd/string_util.hpp>

#include <irccd/daemon/irc.hpp>
#include <irccd/daemon/irccd.hpp>
#include <irccd/daemon/plugin.hpp>
#include <irccd/daemon/server.hpp>

using boost::asio::async_connect;
using boost::asio::deadline_timer;
using boost::asio::io_context;
using boost::asio::ip::tcp;
using boost::asio::ssl::context;
using boost::asio::ssl::stream;
using boost::asio::ssl::stream_base;

using boost::beast::flat_buffer;
using boost::beast::http::async_read;
using boost::beast::http::async_write;
using boost::beast::http::empty_body;
using boost::beast::http::field;
using boost::beast::http::request;
using boost::beast::http::response;
using boost::beast::http::string_body;
using boost::beast::http::verb;
using boost::beast::http::status;

using boost::posix_time::seconds;
using boost::system::error_code;

using std::get;
using std::enable_shared_from_this;
using std::monostate;
using std::move;
using std::regex;
using std::regex_match;
using std::regex_search;
using std::shared_ptr;
using std::smatch;
using std::string;
using std::variant;

namespace irccd {

namespace {

// {{{ globals

// User options.
struct config {
    static inline unsigned timeout{30U};
};

// User formats.
struct formats {
    static inline string info{"#{title}"};
};

// }}}

// {{{ url

struct url {
    string protocol;
    string host;
    string path{"/"};

    static auto parse(const string&) -> url;
};

auto url::parse(const string& link) -> url
{
    static const regex regex("^(https?):\\/\\/([^\\/\\?]+)(.*)$");

    url ret;

    if (smatch match; regex_match(link, match, regex)) {
        ret.protocol = match[1];
        ret.host = match[2];

        if (match.length(3) > 0)
            ret.path = match[3];
        if (ret.path[0] != '/')
            ret.path.insert(ret.path.begin(), '/');
    }

    return ret;
}

// }}}

// {{{ requester

class requester : public enable_shared_from_this<requester> {
private:
    using socket = variant<monostate, tcp::socket, stream<tcp::socket>>;

    shared_ptr<server> server_;
    string channel_;
    string origin_;

    url url_;
    context ctx_{context::sslv23};
    socket socket_;
    flat_buffer buffer_;
    request<empty_body> req_;
    response<string_body> res_;
    deadline_timer timer_;
    tcp::resolver resolver_;

    void notify(const string&);
    void parse();
    void handle_read(const error_code&);
    void read();
    void handle_write(const error_code&);
    void write();
    void handle_handshake(const error_code&);
    void handshake();
    void handle_connect(const error_code&);
    void connect(const tcp::resolver::results_type&);
    void handle_resolve(const error_code&, const tcp::resolver::results_type&);
    void resolve();
    void handle_timer(const error_code&);
    void timer();
    void start();

    requester(io_context&, shared_ptr<server>, string, string, url);

public:
    static void run(io_context&, shared_ptr<server>, string, string, string);
};

void requester::notify(const string& title)
{
    string_util::subst subst;

    subst.keywords.emplace("channel", channel_);
    subst.keywords.emplace("nickname", irc::user::parse(origin_).nick());
    subst.keywords.emplace("origin", origin_);
    subst.keywords.emplace("server", server_->get_id());
    subst.keywords.emplace("title", title);

    server_->message(channel_, format(formats::info, subst));
}

void requester::parse()
{
    /*
     * Use a regex because Boost's XML parser is strict and many web pages may
     * have invalid or broken tags.
     */
    static const regex regex("<title>([^<]+)<\\/title>");

    string data(res_.body().data());
    smatch match;

    if (regex_search(data, match, regex))
        notify(match[1]);
}

void requester::handle_read(const error_code& code)
{
    timer_.cancel();

    if (code)
        return;

    // Request again in case of relocation.
    if (const auto it = res_.find(field::location); it != res_.end()) {
        const string location(it->value().data(), it->value().size());

        run(timer_.get_io_service(), server_, origin_, channel_, location);
    } else
        parse();
}

void requester::read()
{
    const auto self = shared_from_this();
    const auto wrap = [self] (auto code, auto) {
        self->handle_read(code);
    };

    timer();

    switch (socket_.index()) {
    case 1:
        async_read(get<1>(socket_), buffer_, res_, wrap);
        break;
    case 2:
        async_read(get<2>(socket_), buffer_, res_, wrap);
        break;
    default:
        break;
    }
}

void requester::handle_write(const error_code& code)
{
    timer_.cancel();

    if (!code)
        read();
}

void requester::write()
{
    req_.version(11);
    req_.method(verb::get);
    req_.target(url_.path);
    req_.set(field::host, url_.host);
    req_.set(field::user_agent, BOOST_BEAST_VERSION_STRING);

    const auto self = shared_from_this();
    const auto wrap = [self] (auto code, auto) {
        self->handle_write(code);
    };

    timer();

    switch (socket_.index()) {
    case 1:
        async_write(get<1>(socket_), req_, wrap);
        break;
    case 2:
        async_write(get<2>(socket_), req_, wrap);
        break;
    default:
        break;
    }
}

void requester::handle_handshake(const error_code& code)
{
    timer_.cancel();

    if (!code)
        write();
}

void requester::handshake()
{
    const auto self = shared_from_this();

    timer();

    switch (socket_.index()) {
    case 1:
        handle_handshake(error_code());
        break;
    case 2:
        get<2>(socket_).async_handshake(stream_base::client, [self] (auto code) {
            self->handle_handshake(code);
        });
        break;
    default:
        break;
    }
}

void requester::handle_connect(const error_code& code)
{
    timer_.cancel();

    if (!code)
        handshake();
}

void requester::connect(const tcp::resolver::results_type& eps)
{
    const auto self = shared_from_this();
    const auto wrap = [self] (auto code, auto) {
        self->handle_connect(code);
    };

    timer();

    switch (socket_.index()) {
    case 1:
        async_connect(get<1>(socket_), eps, wrap);
        break;
    case 2:
        async_connect(get<2>(socket_).lowest_layer(), eps, wrap);
        break;
    default:
        break;
    }
}

void requester::handle_resolve(const error_code& code, const tcp::resolver::results_type& eps)
{
    timer_.cancel();

    if (!code)
        connect(eps);
}

void requester::resolve()
{
    auto self = shared_from_this();

    timer();
    resolver_.async_resolve(url_.host, url_.protocol, [self] (auto code, auto eps) {
        self->handle_resolve(code, eps);
    });
}

void requester::handle_timer(const error_code& code)
{
    // Force close sockets to cancel all pending operations.
    if (code && code != boost::asio::error::operation_aborted)
        socket_.emplace<monostate>();
}

void requester::timer()
{
    const auto self = shared_from_this();

    timer_.expires_from_now(seconds(config::timeout));
    timer_.async_wait([self] (auto code) {
        self->handle_timer(code);
    });
}

void requester::start()
{
    if (url_.protocol == "http")
        socket_.emplace<tcp::socket>(resolver_.get_io_service());
    else
        socket_.emplace<stream<tcp::socket>>(resolver_.get_io_service(), ctx_);

    resolve();
}

requester::requester(io_context& io,
                     shared_ptr<server> server,
                     string channel,
                     string origin,
                     url url)
    : server_(move(server))
    , channel_(move(channel))
    , origin_(move(origin))
    , url_(move(url))
    , timer_(io)
    , resolver_(io)
{
}

void requester::run(io_context& io, shared_ptr<server> server, string origin, string channel, string link)
{
    auto url = url::parse(link);

    if (url.protocol.empty() || url.host.empty())
        return;

    shared_ptr<requester>(new requester(io, server, channel, origin, move(url)))->start();
}

// }}}

// {{{ links_plugin

class links_plugin : public plugin {
public:
    using plugin::plugin;

    auto get_name() const noexcept -> std::string_view override;

    auto get_author() const noexcept -> std::string_view override;

    auto get_license() const noexcept -> std::string_view override;

    auto get_summary() const noexcept -> std::string_view override;

    auto get_version() const noexcept -> std::string_view override;

    void set_options(const map&) override;

    void set_formats(const map&) override;

    void handle_message(irccd&, const message_event&) override;

    static auto abi() -> version;

    static auto init(std::string) -> std::unique_ptr<plugin>;
};

auto links_plugin::get_name() const noexcept -> std::string_view
{
    return "links";
}

auto links_plugin::get_author() const noexcept -> std::string_view
{
    return "David Demelier <markand@malikania.fr>";
}

auto links_plugin::get_license() const noexcept -> std::string_view
{
    return "ISC";
}

auto links_plugin::get_summary() const noexcept -> std::string_view
{
    return "show webpage title";
}

auto links_plugin::get_version() const noexcept -> std::string_view
{
    return IRCCD_VERSION;
}

void links_plugin::set_options(const map& conf)
{
    if (const auto it = conf.find("timeout"); it != conf.end())
        if (const auto v = string_util::to_uint(it->second); v)
            config::timeout = *v;
}

void links_plugin::set_formats(const map& formats)
{
    if (const auto it = formats.find("info"); it != formats.end())
        formats::info = it->second;
}

void links_plugin::handle_message(irccd& irccd, const message_event& ev)
{
    requester::run(irccd.get_service(), ev.server, ev.origin, ev.channel, ev.message);
}

auto links_plugin::abi() -> version
{
    return version();
}

auto links_plugin::init(std::string id) -> std::unique_ptr<plugin>
{
    return std::make_unique<links_plugin>(std::move(id));
}

BOOST_DLL_ALIAS(links_plugin::abi, irccd_abi_links)
BOOST_DLL_ALIAS(links_plugin::init, irccd_init_links)

// }}}

} // !namespace

} // !irccd
