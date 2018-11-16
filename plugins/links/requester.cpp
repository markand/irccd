/*
 * requester.cpp -- convenient HTTP get requester
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

#include <regex>

#include <irccd/daemon/server.hpp>

#include <irccd/string_util.hpp>

#include "links.hpp"
#include "requester.hpp"
#include "uri.hpp"

using std::error_code;
using std::get;
using std::monostate;
using std::move;
using std::regex;
using std::regex_match;
using std::regex_search;
using std::shared_ptr;
using std::smatch;
using std::string;
using std::variant;

using boost::beast::http::async_read;
using boost::beast::http::async_write;
using boost::beast::http::field;
using boost::beast::http::status;
using boost::beast::http::verb;

using boost::asio::async_connect;
using boost::asio::deadline_timer;
using boost::asio::io_context;
using boost::asio::ip::tcp;

#if defined(IRCCD_HAVE_SSL)

using boost::asio::ssl::context;
using boost::asio::ssl::stream;
using boost::asio::ssl::stream_base;

#endif

using boost::posix_time::seconds;

using irccd::string_util::subst;
using irccd::string_util::format;

using irccd::daemon::irc::user;
using irccd::daemon::server;

namespace irccd {

void requester::notify(const string& title)
{
	subst subst;

	subst.keywords.emplace("channel", channel_);
	subst.keywords.emplace("nickname", user::parse(origin_).nick);
	subst.keywords.emplace("origin", origin_);
	subst.keywords.emplace("server", server_->get_id());
	subst.keywords.emplace("title", title);

	server_->message(channel_, format(links_plugin::format_info, subst));
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
	if (const auto it = res_.find(field::location); it != res_.end() && level_ < 32U) {
		const string location(it->value().data(), it->value().size());
		auto& io = timer_.get_io_service();
		auto uri = uri::parse(location);

		if (!uri)
			return;

		shared_ptr<requester>(new requester(io, server_, channel_, origin_, move(*uri), level_ + 1))->start();
	} else if (res_.result() == status::ok)
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
#if defined(IRCCD_HAVE_SSL)
	case 2:
		async_read(get<2>(socket_), buffer_, res_, wrap);
		break;
#endif
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
	req_.target(uri_.path);
	req_.set(field::host, uri_.hostname);
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
#if defined(IRCCD_HAVE_SSL)
	case 2:
		async_write(get<2>(socket_), req_, wrap);
		break;
#endif
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
#if defined(IRCCD_HAVE_SSL)
	case 2:
		get<2>(socket_).async_handshake(stream_base::client, [self] (auto code) {
			self->handle_handshake(code);
		});
		break;
#endif
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
#if defined(IRCCD_HAVE_SSL)
	case 2:
		async_connect(get<2>(socket_).lowest_layer(), eps, wrap);
		break;
#endif
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
	resolver_.async_resolve(uri_.hostname, uri_.port, [self] (auto code, auto eps) {
		self->handle_resolve(code, eps);
	});
}

void requester::handle_timer(const error_code& code)
{
	// Force close sockets to cancel all pending operations.
	if (code && code != std::errc::operation_canceled)
		socket_.emplace<monostate>();
}

void requester::timer()
{
	const auto self = shared_from_this();

	timer_.expires_from_now(seconds(links_plugin::conf_timeout));
	timer_.async_wait([self] (auto code) {
		self->handle_timer(code);
	});
}

void requester::start()
{
	if (uri_.scheme == "http")
		socket_.emplace<tcp::socket>(resolver_.get_io_service());
#if defined(IRCCD_HAVE_SSL)
	else if (uri_.scheme == "https")
		socket_.emplace<stream<tcp::socket>>(resolver_.get_io_service(), ctx_);
#endif

	// Only do the resolve if scheme is correct.
	if (socket_.index() != 0)
		resolve();
}

requester::requester(io_context& io,
                     shared_ptr<server> server,
                     string channel,
                     string origin,
                     uri uri,
                     size_t level)
	: level_(level)
	, server_(move(server))
	, channel_(move(channel))
	, origin_(move(origin))
	, uri_(move(uri))
	, timer_(io)
	, resolver_(io)
{
}

void requester::run(io_context& io, shared_ptr<server> server, string origin, string channel, string link)
{
	auto uri = uri::parse(link);

	if (!uri)
		return;

	shared_ptr<requester>(new requester(io, server, channel, origin, move(*uri), 0))->start();
}

} // !irccd
