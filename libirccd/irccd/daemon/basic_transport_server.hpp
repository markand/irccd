/*
 * basic_transport_server.hpp -- simple socket transport servers
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

#ifndef IRCCD_DAEMON_BASIC_TRANSPORT_SERVER_HPP
#define IRCCD_DAEMON_BASIC_TRANSPORT_SERVER_HPP

/**
 * \file basic_transport_server.hpp
 * \brief Simple socket transport servers.
 */

#include "basic_transport_client.hpp"
#include "transport_server.hpp"

namespace irccd {

/**
 * \brief Basic implementation for IP/TCP and local sockets
 *
 * This class implements an accept function for:
 *
 *   - boost::asio::ip::tcp
 *   - boost::asio::local::stream_protocol
 */
template <typename Protocol>
class basic_transport_server : public transport_server {
public:
    /**
     * Type for underlying socket.
     */
    using socket_t = typename Protocol::socket;

    /**
     * Type for underlying acceptor.
     */
    using acceptor_t = typename Protocol::acceptor;

protected:
    /**
     * The acceptor object.
     */
    acceptor_t acceptor_;

protected:
    /**
     * \copydoc transport_server::accept
     */
    void do_accept(accept_t handler) override;

public:
    /**
     * Constructor with an acceptor in parameter.
     *
     * \pre acceptor.is_open()
     * \param acceptor the already bound acceptor
     */
    basic_transport_server(acceptor_t acceptor);
};

template <typename Protocol>
basic_transport_server<Protocol>::basic_transport_server(acceptor_t acceptor)
    : acceptor_(std::move(acceptor))
{
    assert(acceptor_.is_open());
}

template <typename Protocol>
void basic_transport_server<Protocol>::do_accept(accept_t handler)
{
    auto client = std::make_shared<basic_transport_client<socket_t>>(*this, acceptor_.get_io_service());

    acceptor_.async_accept(client->stream().socket(), [this, client, handler] (auto code) {
        if (code)
            handler(std::move(code), nullptr);
        else
            handler(std::move(code), std::move(client));
    });
}

} // !irccd

#endif // !IRCCD_DAEMON_BASIC_TRANSPORT_SERVER_HPP
