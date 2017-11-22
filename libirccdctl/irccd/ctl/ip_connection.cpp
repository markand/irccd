/*
 * ip_connection.cpp -- TCP/IP and SSL connections
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

#include "ip_connection.hpp"

using resolver = boost::asio::ip::tcp::resolver;

namespace irccd {

namespace ctl {

namespace {

template <typename Socket>
void do_connect(Socket& socket, resolver::iterator it, connection::connect_t handler)
{
    socket.close();
    socket.async_connect(*it, [&socket, it, handler] (auto code) mutable {
        if (code && it != resolver::iterator())
            do_connect(socket, ++it, std::move(handler));
        else
            handler(code);
    });
}

template <typename Socket>
void do_resolve(const std::string& host,
                const std::string& port,
                Socket& socket,
                resolver& resolver,
                connection::connect_t handler)
{
    resolver::query query(host, port);

    resolver.async_resolve(query, [&socket, handler] (auto code, auto it) {
        if (code)
            handler(code);
        else
            do_connect(socket, it, std::move(handler));
    });
}

} // !namespace

void ip_connection::connect(connect_t handler)
{
    do_resolve(host_, std::to_string(port_), stream_.socket(), resolver_, std::move(handler));
}

#if defined(HAVE_SSL)

void tls_connection::handshake(connect_t handler)
{
    stream_.socket().async_handshake(boost::asio::ssl::stream_base::client, [handler] (auto code) {
        handler(code);
    });
}

void tls_connection::connect(connect_t handler)
{
    do_resolve(host_, std::to_string(port_), stream_.socket().lowest_layer(), resolver_, [handler, this] (auto code) {
        if (code)
            handler(code);
        else
            handshake(std::move(handler));
    });
}

#endif // !HAVE_SSL

} // !ctl

} // !irccd
