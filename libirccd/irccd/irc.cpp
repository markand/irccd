/*
 * irc.cpp -- low level IRC functions
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

#include <cassert>
#include <iterator>
#include <sstream>

#include "irc.hpp"

namespace irccd {

namespace irc {

namespace {

using boost::asio::ip::tcp;

template <typename Socket>
void do_connect(Socket& socket, tcp::resolver::iterator it, connection::connect_t handler)
{
    socket.close();
    socket.async_connect(*it, [&socket, it, handler] (auto code) mutable {
        if (code && it != tcp::resolver::iterator())
            do_connect(socket, ++it, std::move(handler));
        else
            handler(code);
    });
}

template <typename Socket>
void do_resolve(const std::string& host,
                const std::string& port,
                Socket& socket,
                connection::connect_t handler)
{
    auto resolver = std::make_shared<tcp::resolver>(socket.get_io_service());

    resolver->async_resolve(tcp::resolver::query(host, port), [&socket, handler, resolver] (auto code, auto it) {
        if (code)
            handler(code);
        else
            do_connect(socket, it, std::move(handler));
    });
}

} // !namespace

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

void ip_connection::connect(const std::string& host, const std::string& service, connect_t handler)
{
    do_resolve(host, service, socket_, std::move(handler));
}

} // !irc

} // !irccd
