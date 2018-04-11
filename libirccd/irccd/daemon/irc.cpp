/*
 * irc.cpp -- low level IRC functions
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

#include <cassert>
#include <iterator>
#include <sstream>

#include "irc.hpp"

using boost::asio::async_connect;
using boost::asio::async_read_until;
using boost::asio::async_write;

namespace irccd {

namespace irc {

message message::parse(const std::string& line)
{
    std::istringstream iss(line);
    std::string prefix;

    if (line.empty())
        return {};

    // Prefix.
    if (line[0] == ':') {
        iss.ignore(1);
        iss >> prefix;
        iss.ignore(1);
    }

    // Command.
    std::string command;
    iss >> command;
    iss.ignore(1);

    // Arguments.
    std::vector<std::string> args;
    std::istreambuf_iterator<char> it(iss), end;

    while (it != end) {
        std::string arg;

        if (*it == ':')
            arg = std::string(++it, end);
        else {
            while (!isspace(*it) && it != end)
                arg.push_back(*it++);

            // Skip space after param.
            if (it != end)
                ++it;
        }

        args.push_back(std::move(arg));
    }

    return {std::move(prefix), std::move(command), std::move(args)};
}

bool message::is_ctcp(unsigned index) const noexcept
{
    const auto a = arg(index);

    if (a.empty())
        return false;

    return a.front() == 0x01 && a.back() == 0x01;
}

std::string message::ctcp(unsigned index) const
{
    assert(is_ctcp(index));

    return args_[index].substr(1, args_[index].size() - 1);
}

user user::parse(const std::string& line)
{
    if (line.empty())
        return {"", ""};

    const auto pos = line.find("!");

    if (pos == std::string::npos)
        return {line, ""};

    return {line.substr(0, pos), line.substr(pos + 1)};
}

template <typename Socket>
void connection::wrap_connect(Socket& socket,
                              const std::string& host,
                              const std::string& service,
                              const connect_handler& handler) noexcept
{
    using boost::asio::ip::tcp;

    tcp::resolver::query query(host, service);

    resolver_.async_resolve(query, [&socket, handler] (auto code, auto it) {
        if (code) {
            handler(code);
        } else {
            async_connect(socket, it, [handler] (auto code, auto) {
                handler(code);
            });
        }
    });
}

template <typename Socket>
void connection::wrap_recv(Socket& socket, const recv_handler& handler) noexcept
{
    async_read_until(socket, input_, "\r\n", [this, &socket, handler] (auto code, auto xfer) {
        if (xfer == 0U)
            return handler(make_error_code(boost::asio::error::eof), message());
        else if (code)
            return handler(std::move(code), message());

        std::string data;

        try {
            data = std::string(
                boost::asio::buffers_begin(input_.data()),
                boost::asio::buffers_begin(input_.data()) + xfer - 2
            );

            input_.consume(xfer);
        } catch (...) {
            code = make_error_code(boost::system::errc::not_enough_memory);
        }

        handler(code, code ? message() : message::parse(data));
    });
}

template <typename Socket>
void connection::wrap_send(Socket& socket, const send_handler& handler) noexcept
{
    boost::asio::async_write(socket, output_, [handler] (auto code, auto xfer) {
        if (xfer == 0U)
            return handler(make_error_code(boost::asio::error::eof));

        handler(code);
    });
}

void connection::connect(const std::string& host,
                         const std::string& service,
                         const connect_handler& handler) noexcept
{
#if !defined(NDEBUG)
    assert(handler);
    assert(!is_connecting_);

    is_connecting_ = true;
#endif

    do_connect(host, service, [this, handler] (auto code) {
#if !defined(NDEBUG)
        is_connecting_ = false;
#endif
        handler(std::move(code));
    });
}

void connection::recv(const recv_handler& handler) noexcept
{
#if !defined(NDEBUG)
    assert(handler);
    assert(!is_receiving_);

    is_receiving_ = true;
#endif

    do_recv([this, handler] (auto code, auto message) {
#if !defined(NDEBUG)
        is_receiving_ = false;
#endif

        handler(std::move(code), std::move(message));
    });
}

void connection::send(std::string message, const send_handler& handler)
{
#if !defined(NDEBUG)
    assert(handler);
    assert(!is_sending_);

    is_sending_ = true;
#endif

    std::ostream out(&output_);

    out << std::move(message);
    out << "\r\n";

    do_send([this, handler] (auto code) {
#if !defined(NDEBUG)
        is_sending_ = false;
#endif

        handler(std::move(code));
    });
}

void ip_connection::do_connect(const std::string& host,
                               const std::string& service,
                               const connect_handler& handler) noexcept
{
    wrap_connect(socket_, host, service, handler);
}

void ip_connection::do_recv(const recv_handler& handler) noexcept
{
    wrap_recv(socket_, handler);
}

void ip_connection::do_send(const send_handler& handler) noexcept
{
    wrap_send(socket_, handler);
}

#if defined(HAVE_SSL)

void tls_connection::do_connect(const std::string& host,
                                const std::string& service,
                                const connect_handler& handler) noexcept
{
    using boost::asio::ssl::stream_base;

    wrap_connect(socket_.lowest_layer(), host, service, [this, handler] (auto code) {
        if (code) {
            handler(code);
        } else {
            socket_.async_handshake(stream_base::client, [this, handler] (auto code) {
                handler(code);
            });
        }
    });
}

void tls_connection::do_recv(const recv_handler& handler) noexcept
{
    wrap_recv(socket_, handler);
}

void tls_connection::do_send(const send_handler& handler) noexcept
{
    wrap_send(socket_, handler);
}

#endif // !HAVE_SSL

} // !irc

} // !irccd
