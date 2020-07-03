/*
 * irc.cpp -- low level IRC functions
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

#include <irccd/sysconfig.hpp>

#include <cassert>
#include <iterator>
#include <sstream>

#include "irc.hpp"

using std::errc;
using std::flush;
using std::isspace;
using std::istreambuf_iterator;
using std::istringstream;
using std::move;
using std::ostream;
using std::string;
using std::string_view;
using std::vector;

using boost::asio::async_connect;
using boost::asio::async_read_until;
using boost::asio::async_write;
using boost::asio::io_service;
using boost::asio::ip::tcp;

#if defined(IRCCD_HAVE_SSL)

using boost::asio::ssl::stream_base;

#endif

namespace irccd::daemon::irc {

auto message::get(unsigned short index) const noexcept -> const string&
{
	static const string dummy;

	return (index >= args.size()) ? dummy : args[index];
}

auto message::is_ctcp(unsigned short index) const noexcept -> bool
{
	const auto a = get(index);

	if (a.empty())
		return false;

	return a.front() == 0x01 && a.back() == 0x01;
}

auto message::ctcp(unsigned short index) const -> string
{
	assert(is_ctcp(index));

	return args[index].substr(1, args[index].size() - 2);
}

auto message::parse(const string& line) -> message
{
	istringstream iss(line);
	string prefix;

	if (line.empty())
		return {};

	// Prefix.
	if (line[0] == ':') {
		iss.ignore(1);
		iss >> prefix;
		iss.ignore(1);
	}

	// Command.
	string command;
	iss >> command;
	iss.ignore(1);

	// Arguments.
	vector<std::string> args;
	istreambuf_iterator<char> it(iss), end;

	while (it != end) {
		std::string arg;

		if (*it == ':')
			arg = string(++it, end);
		else {
			while (it != end && !isspace(*it))
				arg.push_back(*it++);

			// Skip space after param.
			if (it != end)
				++it;
		}

		args.push_back(move(arg));
	}

	return { move(prefix), move(command), move(args) };
}

auto user::parse(string_view line) -> user
{
	if (line.empty())
		return { "", "" };

	const auto pos = line.find("!");

	if (pos == string::npos)
		return { string(line), "" };

	return { string(line.substr(0, pos)), string(line.substr(pos + 1)) };
}

void connection::handshake(const connect_handler& handler)
{
	if (!ssl_) {
		handler({});
		return;
	}

#if defined(IRCCD_HAVE_SSL)
	ssl_socket_.async_handshake(stream_base::client, [handler] (auto code) {
		handler(move(code));
	});
#endif
}

void connection::connect(const tcp::resolver::results_type& endpoints, const connect_handler& handler)
{
	async_connect(socket_, endpoints, [this, handler] (auto code, auto) {
		if (code) {
			handler(move(code));
			return;
		}

		handshake(handler);
	});
}

void connection::resolve(string_view hostname, string_view port, const connect_handler& handler)
{
	auto chain = [this, handler] (auto code, auto eps) {
		if (code)
			handler(std::move(code));
		else
			connect(eps, std::move(handler));
	};

	if (ipv6_ && ipv4_)
		resolver_.async_resolve(hostname, port, move(chain));
	else if (ipv6_)
		resolver_.async_resolve(tcp::v6(), hostname, port, move(chain));
	else
		resolver_.async_resolve(tcp::v4(), hostname, port, move(chain));
}

connection::connection(io_service& service)
	: service_(service)
	, resolver_(service)
{
}

void connection::use_ipv4(bool enable) noexcept
{
	ipv4_ = enable;
}

void connection::use_ipv6(bool enable) noexcept
{
	ipv6_ = enable;
}

void connection::use_ssl(bool enable) noexcept
{
	ssl_ = enable;
}

void connection::connect(string_view hostname, string_view service, connect_handler handler)
{
#if !defined(IRCCD_HAVE_SSL)
	assert(!ssl_);
#endif
#if !defined(NDEBUG)
	assert(!is_connecting_);

	is_connecting_ = true;
#endif
	assert(handler);
	assert(ipv4_ || ipv6_);

	auto chain = [this, handler] (auto code) {
#if !defined(NDEBUG)
		is_connecting_ = false;
#endif
		(void)this;

		handler(move(code));
	};

	resolve(hostname, service, move(chain));
}

void connection::disconnect()
{
	socket_.close();
}

void connection::recv(recv_handler handler)
{
#if !defined(NDEBUG)
	assert(!is_receiving_);

	is_receiving_ = true;
#endif

	auto chain = [this, handler] (auto code, auto xfer) {
#if !defined(NDEBUG)
		is_receiving_ = false;
#endif
		(void)this;

		if (code == boost::asio::error::not_found)
			return handler(make_error_code(errc::argument_list_too_long), message());
		if (code == boost::asio::error::eof || xfer == 0)
			return handler(make_error_code(errc::connection_reset), message());
		if (code)
			return handler(move(code), message());

		string data;

		// 1. Convert the buffer safely.
		try {
			data = string(
				buffers_begin(input_.data()),
				buffers_begin(input_.data()) + xfer - 2
			);

			input_.consume(xfer);
		} catch (...) {
			return handler(make_error_code(errc::not_enough_memory), message());
		}

		handler(move(code), message::parse(data));
	};

#if defined(IRCCD_HAVE_SSL)
	if (ssl_)
		async_read_until(ssl_socket_, input_, "\r\n", move(chain));
	else
#endif
		async_read_until(socket_, input_, "\r\n", move(chain));
}

void connection::send(string_view message, send_handler handler)
{
#if !defined(NDEBUG)
	assert(!is_sending_);

	is_sending_ = true;
#endif

	auto chain = [this, handler] (auto code, auto xfer) {
#if !defined(NDEBUG)
		is_sending_ = false;
#endif
		(void)this;

		if (code == boost::asio::error::eof || xfer == 0)
			return handler(make_error_code(errc::connection_reset));

		handler(move(code));
	};

	ostream out(&output_);

	out << message;
	out << "\r\n";
	out << flush;

#if defined(IRCCD_HAVE_SSL)
	if (ssl_)
		async_write(ssl_socket_, output_, move(chain));
	else
#endif
		async_write(socket_, output_, move(chain));
}

} // !irccd::daemon::irc
