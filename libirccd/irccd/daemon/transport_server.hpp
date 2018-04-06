/*
 * transport_server.hpp -- server side transports
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

#ifndef IRCCD_DAEMON_TRANSPORT_SERVER_HPP
#define IRCCD_DAEMON_TRANSPORT_SERVER_HPP

#include <irccd/sysconfig.hpp>

#include <cassert>
#include <functional>
#include <memory>
#include <unordered_set>
#include <type_traits>

#include <boost/asio.hpp>

#include "transport_client.hpp"

namespace irccd {

/**
 * \brief Abstract transport server class.
 *
 * # Synopsis
 *
 * The transport_server class is an abstract interface that waits for clients to
 * connect and store them locally. It does not know the underlying
 * implementation so derived classes may be implemented in any shape of form.
 *
 * As only constraint the implementation must provide an asynchronous operation
 * to avoid blocking the daemon.
 *
 * The derived class only have to implement do_accept function which is only
 * responsible of getting a client ready for I/O (receiving and sending), the
 * transport_server does authentication and greeting by itself.
 *
 * # Accept procedure
 *
 * The connection procedure looks like this:
 *
 * ```
 *            o (transport_server::accept is called)
 *            |
 *            v                          [error]
 *   +-----------------------------+                  +---------------------+
 *   | asynchronous accept process |----------------->| client is discarded |
 *   +-----------------------------+                  +---------------------+
 *            |                                                          ^
 *            | [success]                                                |
 *            v                                                          |
 *   +-----------------------------------------+  [error while sending]  |
 *   | sending irccd information to the client |------------------------>+
 *   +-----------------------------------------+                         |
 *     |              |                                                  |
 *     |              | [authentication required]                        |
 *     |              |                                                  |
 *     |              v                    [error or invalid password]   |
 *     |      +-------------------------+         +------------+         |
 *     |      | wait for authentication |-------->| send error |-------->+
 *     |      +-------------------------+         +------------+         ^
 *     |              |                                                  |
 *     |              | [correct password]                               |
 *     v              v                                                  |
 *   +---------------------------------------+  [incorrect]              |
 *   | client is added to the list and ready ]-------------------------- +
 *   +---------------------------------------+
 * ```
 *
 * # I/O procedures
 *
 * Each client has a reference to its parent, since operations are asynchronous,
 * they maintain their lifetime by themselve to update the parent list on
 * errors.
 *
 * See the following diagram:
 *
 * ```
 *       o (transport_client::recv or send is called) o
 *       |                                            |
 *       | [no operations in queue]                   | [operation in progress]
 *       |                                            v
 *       |                                    +---------------+
 *       |                                    | push in queue |
 *       |                                    +---------------+
 *       |
 *       |
 *       |                                [pending operations in queue]
 *       |<-----------------------------------------------+
 *       |                                                ^
 *       |                                                |
 *       v                             [success]          |
 *   +-------------------------------+           +-------------------+
 *   | asynchronous operation starts |---------->| handler is called |
 *   +-------------------------------+           +-------------------+
 *       |
 *       v [error]
 *   +--------------------------------------+
 *   | handler is called with an error code |
 *   +--------------------------------------+
 *       |
 *       v
 *   +----------------------------------+
 *   | client delete itself from parent |
 *   +----------------------------------+
 * ```
 *
 * \see transport_client
 * \see transport_service
 */
class transport_server {
public:
    /**
     * Set of clients.
     */
    using client_set = std::unordered_set<std::shared_ptr<transport_client>>;

    /**
     * Callback when a new client should be accepted.
     */
    using accept_handler = std::function<void (
        boost::system::error_code,
        std::shared_ptr<transport_client>
    )>;

private:
    boost::asio::io_service& service_;
    client_set clients_;
    std::string password_;

    void do_auth(std::shared_ptr<transport_client>, accept_handler);
    void do_greetings(std::shared_ptr<transport_client>, accept_handler);

protected:
    /**
     * Start accept operation, the implementation should not block and call
     * the handler function on error or completion.
     *
     * \pre handler must not be null
     * \param handler the handler function
     */
    virtual void do_accept(accept_handler handler) = 0;

public:
    /**
     * Default constructor.
     */
    inline transport_server(boost::asio::io_service& service) noexcept
        : service_(service)
    {
    }

    /**
     * Virtual destructor defaulted.
     */
    virtual ~transport_server() noexcept = default;

    /**
     * Accept a new client into the transport server.
     *
     * Also perform greetings and authentication under the hood. On success, the
     * client is added into the server and is ready to use.
     *
     * \pre accept != nullptr
     * \param handler the handler
     */
    void accept(accept_handler handler);

    /**
     * Get the io service.
     *
     * \return the service
     */
    inline const boost::asio::io_service& get_service() const noexcept
    {
        return service_;
    }

    /**
     * Overloaded function.
     *
     * \return the service
     */
    inline boost::asio::io_service& get_service() noexcept
    {
        return service_;
    }

    /**
     * Get the clients.
     *
     * \return the clients
     */
    inline const client_set& get_clients() const noexcept
    {
        return clients_;
    }

    /**
     * Overloaded function.
     *
     * \return the clients
     */
    inline client_set& get_clients() noexcept
    {
        return clients_;
    }

    /**
     * Get the current password, empty string means no password.
     *
     * \return the password
     */
    inline const std::string& get_password() const noexcept
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
 * \brief Transport error.
 */
class transport_error : public boost::system::system_error {
public:
    /**
     * \brief Transport related errors.
     */
    enum error {
        //!< No error.
        no_error = 0,

        //!< Authentication is required.
        auth_required,

        //!< Authentication was invalid.
        invalid_auth,

        //!< Invalid TCP/IP port.
        invalid_port,

        //!< Invalid TCP/IP address.
        invalid_address,

        //!< The specified host was invalid.
        invalid_hostname,

        //!< Invalid unix local path.
        invalid_path,

        //!< Invalid IPv4/IPv6 family.
        invalid_family,

        //!< Invalid certificate given.
        invalid_certificate,

        //!< Invalid private key given.
        invalid_private_key,

        //!< SSL was requested but is disabled.
        ssl_disabled,

        //!< Kind of transport not supported on this platform.
        not_supported
    };

    /**
     * Constructor.
     *
     * \param code the error code
     */
    transport_error(error code) noexcept;
};

/**
 * Get the transport error category singleton.
 *
 * \return the singleton
 */
const boost::system::error_category& transport_category() noexcept;

/**
 * Create a boost::system::error_code from server_error::error enum.
 *
 * \param e the error code
 */
boost::system::error_code make_error_code(transport_error::error e) noexcept;

} // !irccd

namespace boost {

namespace system {

template <>
struct is_error_code_enum<irccd::transport_error::error> : public std::true_type {
};

} // !system

} // !boost

#endif // !IRCCD_DAEMON_TRANSPORT_SERVER_HPP
