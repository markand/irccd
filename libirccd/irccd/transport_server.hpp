/*
 * transport_server.hpp -- server side transports
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

#ifndef IRCCD_TRANSPORT_SERVER_HPP
#define IRCCD_TRANSPORT_SERVER_HPP

#include "sysconfig.hpp"

#include <cassert>
#include <functional>
#include <memory>
#include <unordered_set>
#include <type_traits>

#include <boost/asio.hpp>

#if defined(HAVE_SSL)
#   include <boost/asio/ssl.hpp>
#endif

#include "transport_client.hpp"

namespace irccd {

/**
 * \brief Abstract transport server class.
 *
 * This class create asynchronous operation to accept new clients.
 */
class transport_server {
protected:
    /**
     * Set of clients.
     */
    using client_set_t = std::unordered_set<std::shared_ptr<transport_client>>;

    /**
     * Callback when a new client should be accepted.
     */
    using accept_t = std::function<void (std::shared_ptr<transport_client>, boost::system::error_code)>;

private:
    client_set_t clients_;
    std::string password_;

    bool do_auth_check(nlohmann::json, accept_t);
    void do_auth(std::shared_ptr<transport_client>, accept_t);
    void do_greetings(std::shared_ptr<transport_client>, accept_t);

protected:
    /**
     * Start accept operation, the implementation should not block and call
     * the handler function on error or completion.
     *
     * \pre handler must not be null
     * \param handler the handler function
     */
    virtual void do_accept(accept_t handler) = 0;

public:
    /**
     * Default constructor.
     */
    transport_server() noexcept = default;

    /**
     * Virtual destructor defaulted.
     */
    virtual ~transport_server() noexcept = default;

    /**
     * Wrapper that automatically add the new client into the list.
     *
     * If handler is not null it is called on error or on successful accept
     * operation.
     *
     * \param handler the handler
     */
    void accept(accept_t handler);

    /**
     * Get the clients.
     *
     * \return the clients
     */
    inline const client_set_t& clients() const noexcept
    {
        return clients_;
    }

    /**
     * Overloaded function.
     *
     * \return the clients
     */
    inline client_set_t& clients() noexcept
    {
        return clients_;
    }

    /**
     * Get the current password, empty string means no password.
     *
     * \return the password
     */
    inline const std::string& password() const noexcept
    {
        return password_;
    }

    /**
     * Set an optional password, empty string means no password.
     *
     * \param password the password
     */
    inline void set_password(std::string password) noexcept
    {
        password_ = std::move(password);
    }
};

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
    using client_type = basic_transport_client<socket_t>;

    auto client = std::make_shared<client_type>(*this, acceptor_.get_io_service());

    acceptor_.async_accept(client->socket(), [client, handler] (auto code) {
        if (!code)
            handler(std::move(client), std::move(code));
        else
            handler(nullptr, code);
    });
}

/**
 * Convenient type for IP/TCP
 */
using tcp_transport_server = basic_transport_server<boost::asio::ip::tcp>;

#if !defined(_WIN32)

/**
 * Convenient type for UNIX local sockets.
 */
using local_transport_server = basic_transport_server<boost::asio::local::stream_protocol>;

#endif // !_WIN32

#if defined(HAVE_SSL)

/**
 * \brief Secure layer implementation.
 */
class tls_transport_server : public tcp_transport_server {
public:
    using context_t = boost::asio::ssl::context;

private:
    context_t context_;

    void do_handshake(std::shared_ptr<tls_transport_client>, accept_t);

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
     * \param acceptor the acceptor
     * \param context the SSL context
     */
    tls_transport_server(acceptor_t acceptor, context_t context);
};

#endif // !HAVE_SSL

} // !irccd

#endif // !IRCCD_TRANSPORT_SERVER_HPP
