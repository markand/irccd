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
    using accept_t = std::function<void (boost::system::error_code, std::shared_ptr<transport_client>)>;

private:
    client_set_t clients_;
    std::string password_;

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

class transport_error : public std::system_error {
public:
    enum error {
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

    transport_error(error err) noexcept;
};

/**
 * Get the transport error category singleton.
 *
 * \return the singleton
 */
const std::error_category& transport_category() noexcept;

/**
 * Create a boost::system::error_code from server_error::error enum.
 *
 * \param e the error code
 */
std::error_code make_error_code(transport_error::error e) noexcept;

} // !irccd

namespace std {

template <>
struct is_error_code_enum<irccd::transport_error::error> : public std::true_type {
};

} // !std

#endif // !IRCCD_DAEMON_TRANSPORT_SERVER_HPP
