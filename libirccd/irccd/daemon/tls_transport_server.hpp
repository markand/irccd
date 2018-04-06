/*
 * tls_transport_server.hpp -- server side transports (SSL support)
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

#ifndef IRCCD_DAEMON_TLS_TRANSPORT_SERVER_HPP
#define IRCCD_DAEMON_TLS_TRANSPORT_SERVER_HPP

/**
 * \file tls_transport_server.hpp
 * \brief Server side transports (SSL support).
 */

#include <irccd/sysconfig.hpp>

#if defined(HAVE_SSL)

#include <boost/asio/ssl.hpp>

#include "ip_transport_server.hpp"

namespace irccd {

/**
 * \brief Secure layer implementation.
 */
class tls_transport_server : public ip_transport_server {
private:
    using context_t = boost::asio::ssl::context;
    using client_t = basic_transport_client<boost::asio::ssl::stream<socket_t>>;

    context_t context_;

    void do_handshake(std::shared_ptr<client_t>, accept_t);

protected:
    /**
     * \copydoc tcp_transport_server::do_accept
     *
     * This function does the same as tcp_transport_server::do_accept but it
     * also perform a SSL handshake after a successful accept operation.
     */
    void do_accept(accept_t handler) override;

public:
    /**
     * Construct a secure layer transport server.
     *
     * \param service the io service
     * \param acceptor the acceptor
     * \param context the SSL context
     */
    tls_transport_server(boost::asio::io_service& service, acceptor_t acceptor, context_t context);
};

} // !irccd

#endif // !HAVE_SSL

#endif // !IRCCD_DAEMON_TLS_TRANSPORT_SERVER_HPP
