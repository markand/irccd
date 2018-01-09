/*
 * ip_connection.hpp -- TCP/IP and SSL connections
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

#ifndef IRCCD_CTL_IP_CONNECTION_HPP
#define IRCCD_CTL_IP_CONNECTION_HPP

/**
 * \file ip_connection.hpp
 * \brief TCP/IP and SSL connections.
 */

#include <irccd/sysconfig.hpp>

#include <string>
#include <cstdint>

#include "basic_connection.hpp"

namespace irccd {

namespace ctl {

/**
 * \brief Common class for both ip and tls connections.
 */
template <typename Socket>
class basic_ip_connection : public basic_connection<Socket> {
protected:
    boost::asio::ip::tcp::resolver resolver_;
    std::string host_;
    std::uint16_t port_;

public:
    /**
     * Construct the ip connection.
     *
     * The socket is created as invoked like this:
     *
     *     socket_(service_, std::forward<Args>(args)...)
     *
     * \param service the io service
     * \param host the host
     * \param port the port number
     * \param args extra arguments (except service) to pass to the socket constructor
     */
    template <typename... Args>
    inline basic_ip_connection(boost::asio::io_service& service, std::string host, std::uint16_t port, Args&&... args)
        : basic_connection<Socket>(service, std::forward<Args>(args)...)
        , resolver_(service)
        , host_(std::move(host))
        , port_(std::move(port))
    {
    }
};

/**
 * \brief Raw TCP/IP connection.
 */
class ip_connection : public basic_ip_connection<boost::asio::ip::tcp::socket> {
public:
    /**
     * Inherited constructor.
     */
    using basic_ip_connection::basic_ip_connection;

    /**
     * \copydoc connection::connect
     */
    void connect(connect_t handler);
};

#if defined(HAVE_SSL)

/**
 * \brief Secure layer connection.
 */
class tls_connection : public basic_ip_connection<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> {
private:
    void handshake(connect_t);

public:
    /**
     * Construct the TLS connection.
     *
     * \param service the io service
     * \param context the context
     * \param host the host
     * \param port the port number
     */
    inline tls_connection(boost::asio::io_service& service,
                          boost::asio::ssl::context& context,
                          std::string host,
                          std::uint16_t port)
        : basic_ip_connection(service, std::move(host), port, context)
    {
    }

    /**
     * \copydoc connection::connect
     */
    void connect(connect_t handler);
};

#endif // !HAVE_SSL

} // !ctl

} // !irccd

#endif // !IRCCD_CTL_IP_CONNECTION_HPP
