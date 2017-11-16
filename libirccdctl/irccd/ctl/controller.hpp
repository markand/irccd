/*
 * controller.hpp -- main irccdctl interface
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

#ifndef IRCCD_CTL_CONTROLLER_HPP
#define IRCCD_CTL_CONTROLLER_HPP

/**
 * \file controller.hpp
 * \brief Main irccdctl interface.
 */

#include <boost/system/error_code.hpp>

#include <deque>
#include <functional>
#include <string>

#include <json.hpp>

namespace irccd {

namespace ctl {

class connection;

/**
 * \brief Main irccdctl interface.
 *
 * This class is an easy frontend to issue commands to irccd, it uses an
 * independant connection to perform the requests.
 *
 * This class is responsible of doing initial connection, performing checks and
 * optional authentication.
 *
 * It is implemented in mind that connection are asynchronous even though this
 * is not necessary.
 *
 * \see connection
 * \see network_connection
 * \see local_connection
 * \see ip_connection
 * \see tls_connection
 */
class controller {
public:
    /**
     * Connection handler.
     *
     * This callback is called when connection has been completed or failed. In
     * both case, the error code is set and the JSON object may contain the
     * irccd program information.
     */
    using connect_t = std::function<void (boost::system::error_code, nlohmann::json)>;

    /**
     * Receive handler.
     *
     * This callback is called when a message has been received. If an error
     * occured the error_code is set and the JSON object is null, otherwise it
     * contains the received message.
     */
    using recv_t = std::function<void (boost::system::error_code, nlohmann::json)>;

    /**
     * Send handler.
     *
     * This callback is optional and is called when a message has been sent, it
     * is also called if an error occured.
     */
    using send_t = std::function<void (boost::system::error_code, nlohmann::json)>;

private:
    using recv_queue_t = std::deque<recv_t>;
    using send_queue_t = std::deque<std::pair<nlohmann::json, send_t>>;

    connection& conn_;
    recv_queue_t rqueue_;
    send_queue_t squeue_;
    std::string password_;

    void flush_recv();
    void flush_send();
    void authenticate(connect_t, nlohmann::json);
    void verify(connect_t);

public:
    /**
     * Construct the controller with its connection.
     *
     * \note no connect attempt is done
     */
    inline controller(connection& conn) noexcept
        : conn_(conn)
    {
    }

    /**
     * Tells if receive requests are pending.
     *
     * \return true if receive queue is not empty
     */
    inline bool has_recv_pending() const noexcept
    {
        return !rqueue_.empty();
    }

    /**
     * Tells if send requests are pending.
     *
     * \return true if send queue is not empty
     */
    inline bool has_send_pending() const noexcept
    {
        return !squeue_.empty();
    }

    /**
     * Tells if receive or send requests are pending.
     *
     * \return true if one of receive/send queue is not empty
     */
    inline bool has_pending() const noexcept
    {
        return has_recv_pending() || has_send_pending();
    }

    /**
     * Get the optional password set.
     *
     * \return the password
     */
    inline const std::string& password() const noexcept
    {
        return password_;
    }

    /**
     * Set an optional password.
     *
     * An empty password means no authentication (default).
     *
     * \param password the password
     * \note this must be called before connect
     */
    inline void set_password(std::string password) noexcept
    {
        password_ = std::move(password);
    }

    /**
     * Attempt to connect to the irccd daemon.
     *
     * \pre handler != nullptr
     * \param handler the handler
     */
    void connect(connect_t handler);

    /**
     * Queue a receive operation, if receive operations are already running, it
     * is queued and ran once ready.
     *
     * \pre handler != nullptr
     * \param handler the recv handler
     */
    void recv(recv_t handler);

    /**
     * Queue a send operation, if receive operations are already running, it is
     * queued and ran once ready.
     *
     * \pre message.is_object()
     * \param message the JSON message
     * \param handler the optional completion handler
     */
    void send(nlohmann::json message, send_t handler);
};

} // !ctl

} // !irccd

#endif // !IRCCD_CTL_CONTROLLER_HPP
