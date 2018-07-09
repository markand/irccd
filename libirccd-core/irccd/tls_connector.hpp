/*
 * tls_connector.hpp -- TLS/SSL connectors
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

#ifndef IRCCD_COMMON_TLS_CONNECTOR_HPP
#define IRCCD_COMMON_TLS_CONNECTOR_HPP

/**
 * \file tls_connector.hpp
 * \brief TLS/SSL connectors.
 */

#include <irccd/sysconfig.hpp>

#if defined(IRCCD_HAVE_SSL)

#include "socket_connector.hpp"
#include "tls_stream.hpp"

namespace irccd {

namespace io {

/**
 * \brief TLS/SSL connectors.
 * \tparam Protocol a Boost.Asio compatible protocol (e.g. ip::tcp)
 */
template <typename Protocol = boost::asio::ip::tcp>
class tls_connector : public socket_connector<Protocol> {
private:
    boost::asio::ssl::context context_;

public:
    /**
     * Construct a secure layer transport server.
     *
     * \param context the SSL context
     * \param args the arguments to socket_connector<Socket> constructor
     */
    template <typename... Args>
    inline tls_connector(boost::asio::ssl::context context, Args&&... args)
        : socket_connector<Protocol>(std::forward<Args>(args)...)
        , context_(std::move(context))
    {
    }

    /**
     * \copydoc socket_connector::connect
     */
    void connect(connect_handler handler) override;
};

template <typename Protocol>
void tls_connector<Protocol>::connect(connect_handler handler)
{
    using boost::asio::ssl::stream_base;
    using socket = typename Protocol::socket;

    assert(handler);

    const auto stream = std::make_shared<tls_stream<socket>>(this->get_io_service(), context_);

    socket_connector<Protocol>::do_connect(stream->get_socket().lowest_layer(), [handler, stream] (auto code) {
        if (code) {
            handler(code, nullptr);
            return;
        }

        stream->get_socket().async_handshake(stream_base::client, [handler, stream] (auto code) {
            handler(detail::convert(code), code ? nullptr : std::move(stream));
        });
    });
}

} // !io

} // !irccd

#endif // !IRCCD_HAVE_SSL

#endif // !IRCCD_COMMON_TLS_CONNECTOR_HPP
