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

namespace irccd {

namespace irc {

namespace {

using boost::asio::ip::tcp;

template <typename Socket>
void wrap_connect(Socket& socket, tcp::resolver::iterator it, connection::connect_t handler)
{
    assert(handler);

    socket.close();
    socket.async_connect(*it, [&socket, it, handler] (auto code) mutable {
        if (code && ++it != tcp::resolver::iterator())
            wrap_connect(socket, it, std::move(handler));
        else
            handler(code);
    });
}

template <typename Socket>
void wrap_resolve(Socket& socket,
                tcp::resolver& resolver,
                const std::string& host,
                const std::string& port,
                connection::connect_t handler)
{
    assert(handler);

    tcp::resolver::query query(host, port);

    resolver.async_resolve(query, [&socket, handler] (auto code, auto it) {
        if (code)
            handler(code);
        else
            wrap_connect(socket, it, std::move(handler));
    });
}

template <typename Socket>
void wrap_recv(Socket& socket, boost::asio::streambuf& buffer, connection::recv_t handler)
{
    assert(handler);

    boost::asio::async_read_until(socket, buffer, "\r\n", [&socket, &buffer, handler] (auto code, auto xfer) noexcept {
        if (code || xfer == 0U) {
            handler(std::move(code), message());
            return;
        }

        std::string data;

        try {
            data = std::string(
                boost::asio::buffers_begin(buffer.data()),
                boost::asio::buffers_begin(buffer.data()) + xfer - 2
            );

            buffer.consume(xfer);
        } catch (...) {
            code = make_error_code(boost::system::errc::not_enough_memory);
        }

        handler(code, code ? message() : message::parse(data));
    });
}

template <typename Socket>
void wrap_send(Socket& socket, const std::string& message, connection::send_t handler)
{
    assert(handler);

    boost::asio::async_write(socket, boost::asio::buffer(message), [handler, message] (auto code, auto) noexcept {
        handler(code);
    });
}

} // !namespace

void connection::rflush()
{
    if (input_.empty())
        return;

    do_recv(buffer_, [this] (auto code, auto message) {
        if (input_.front())
            input_.front()(code, std::move(message));

        input_.pop_front();

        if (!code)
            rflush();
    });
}

void connection::sflush()
{
    if (output_.empty())
        return;

    do_send(output_.front().first, [this] (auto code) {
        if (output_.front().second)
            output_.front().second(code);

        output_.pop_front();

        if (!code)
            sflush();
    });
}

void connection::connect(const std::string& host, const std::string& service, connect_t handler)
{
    assert(handler);

    do_connect(host, service, std::move(handler));
}

void connection::recv(recv_t handler)
{
    auto in_progress = !input_.empty();

    input_.push_back(std::move(handler));

    if (!in_progress)
        rflush();
}

void connection::send(std::string message, send_t handler)
{
    auto in_progress = !output_.empty();

    output_.emplace_back(message + "\r\n", std::move(handler));

    if (!in_progress)
        sflush();
}

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
    auto a = arg(index);

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

    auto pos = line.find("!");

    if (pos == std::string::npos)
        return {line, ""};

    return {line.substr(0, pos), line.substr(pos + 1)};
}

void ip_connection::do_connect(const std::string& host, const std::string& service, connect_t handler) noexcept
{
    wrap_resolve(socket_, resolver_, host, service, std::move(handler));
}

void ip_connection::do_recv(boost::asio::streambuf& buffer, recv_t handler) noexcept
{
    wrap_recv(socket_, buffer, std::move(handler));
}

void ip_connection::do_send(const std::string& data, send_t handler) noexcept
{
    wrap_send(socket_, data, std::move(handler));
}

#if defined(HAVE_SSL)

void tls_connection::do_connect(const std::string& host, const std::string& service, connect_t handler) noexcept
{
    using boost::asio::ssl::stream_base;

    wrap_resolve(socket_.lowest_layer(), resolver_, host, service, [this, handler] (auto code) {
        if (code)
            handler(code);
        else
            socket_.async_handshake(stream_base::client, [this, handler] (auto code) {
                handler(code);
            });
    });
}

void tls_connection::do_recv(boost::asio::streambuf& buffer, recv_t handler) noexcept
{
    wrap_recv(socket_, buffer, std::move(handler));
}

void tls_connection::do_send(const std::string& data, send_t handler) noexcept
{
    wrap_send(socket_, data, std::move(handler));
}

#endif // !HAVE_SSL

} // !irc

} // !irccd
