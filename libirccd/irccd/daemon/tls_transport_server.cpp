/*
 * tls_transport_server.cpp -- server side transports (SSL support)
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

#include "tls_transport_server.hpp"

#if defined(HAVE_SSL)

namespace irccd {

void tls_transport_server::do_handshake(std::shared_ptr<client_t> client, accept_t handler)
{
    client->stream().socket().async_handshake(boost::asio::ssl::stream_base::server, [client, handler] (auto code) {
        handler(std::move(code), std::move(client));
    });
}

tls_transport_server::tls_transport_server(acceptor_t acceptor, context_t context)
    : ip_transport_server(std::move(acceptor))
    , context_(std::move(context))
{
}

void tls_transport_server::do_accept(accept_t handler)
{
    auto client = std::make_shared<client_t>(*this, acceptor_.get_io_service(), context_);

    acceptor_.async_accept(client->stream().socket().lowest_layer(), [this, client, handler] (auto code) {
        if (code)
            handler(std::move(code), nullptr);
        else
            do_handshake(std::move(client), std::move(handler));
    });
}

} // !irccd

#endif // !HAVE_SSL