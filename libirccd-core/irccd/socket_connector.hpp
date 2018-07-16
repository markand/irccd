/*
 * socket_connector.hpp -- socket connection interface
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

#ifndef IRCCD_COMMON_SOCKET_CONNECTOR_HPP
#define IRCCD_COMMON_SOCKET_CONNECTOR_HPP

/**
 * \file socket_connector.hpp
 * \brief Socket connection interface.
 */

#include <irccd/sysconfig.hpp>

#include <vector>

#include "connector.hpp"
#include "socket_stream.hpp"

namespace irccd::io {

/**
 * \brief Socket connection interface.
 * \tparam Protocol a Boost.Asio compatible protocol (e.g. ip::tcp)
 */
template <typename Protocol>
class socket_connector : public connector {
public:
    /**
     * Convenient endpoint alias.
     */
    using endpoint = typename Protocol::endpoint;

    /**
     * Convenient socket alias.
     */
    using socket = typename Protocol::socket;

private:
    boost::asio::io_service& service_;
    std::vector<endpoint> endpoints_;

#if !defined(NDEBUG)
    bool is_connecting_{false};
#endif

protected:
    /**
     * Start trying to connect to all endpoints.
     *
     * \param socket the underlying socket
     * \param handler handler with `void f(std::error_code)` signature
     */
    template <typename Socket, typename Handler>
    void do_connect(Socket& socket, Handler handler);

public:
    /**
     * Construct the socket connector with only one endpoint.
     *
     * \param service the service
     * \param endpoint the unique endpoint
     */
    inline socket_connector(boost::asio::io_service& service, endpoint endpoint) noexcept
        : service_(service)
        , endpoints_{std::move(endpoint)}
    {
    }

    /**
     * Construct the socket connection.
     *
     * \param service the service
     * \param eps the endpoints
     */
    inline socket_connector(boost::asio::io_service& service, std::vector<endpoint> eps) noexcept
        : service_(service)
        , endpoints_(std::move(eps))
    {
    }

    /**
     * Get the underlying I/O service.
     *
     * \return the I/O service
     */
    inline const boost::asio::io_service& get_io_service() const noexcept
    {
        return service_;
    }

    /**
     * Overloaded function.
     *
     * \return the I/O service
     */
    inline boost::asio::io_service& get_io_service() noexcept
    {
        return service_;
    }

    /**
     * \copydoc connector::connect
     */
    void connect(connect_handler handler);
};

template <typename Protocol>
template <typename Socket, typename Handler>
void socket_connector<Protocol>::do_connect(Socket& socket, Handler handler)
{
#if !defined(NDEBUG)
    assert(!is_connecting_);

    is_connecting_ = true;
#endif

    boost::asio::async_connect(socket, endpoints_.begin(), endpoints_.end(), [this, handler] (auto code, auto ep) {
#if !defined(NDEBUG)
        is_connecting_ = false;
#endif

        if (ep == endpoints_.end())
            handler(make_error_code(std::errc::host_unreachable));
        else
            handler(detail::convert(code));
    });
}

template <typename Protocol>
void socket_connector<Protocol>::connect(connect_handler handler)
{
    assert(handler);

    const auto stream = std::make_shared<socket_stream<socket>>(service_);

    do_connect(stream->get_socket(), [handler, stream] (auto code) {
        handler(code, code ? nullptr : std::move(stream));
    });
}

/**
 * Convenient TCP/IP connector type.
 */
using ip_connector = socket_connector<boost::asio::ip::tcp>;

#if !BOOST_OS_WINDOWS

/**
 * Convenient Unix conncetor type.
 */
using local_connector = socket_connector<boost::asio::local::stream_protocol>;

#endif

} // !irccd::io

#endif // !IRCCD_COMMON_SOCKET_CONNECTOR_HPP
