/*
 * transport_client.hpp -- server side transport clients
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

#ifndef IRCCD_TRANSPORT_CLIENT_HPP
#define IRCCD_TRANSPORT_CLIENT_HPP

#include "network_stream.hpp"

namespace irccd {

class transport_server;

/**
 * \brief Abstract transport client class.
 *
 * This class is responsible of receiving/sending data.
 */
class transport_client : public std::enable_shared_from_this<transport_client> {
public:
    /**
     * Client state.
     */
    enum class state_t {
        authenticating,                     //!< client is authenticating
        ready,                              //!< client is ready
        closing                             //!< client is closing
    };

private:
    state_t state_{state_t::authenticating};
    transport_server& parent_;

    void close();

protected:
    /**
     * Request a receive operation.
     *
     * The implementation must call the handler once the operation has finished
     * even in case of errors.
     *
     * \param handler the non-null handler
     */
    virtual void do_recv(network_recv_handler handler) = 0;

    /**
     * Request a send operation.
     *
     * The implementation must call the handler once the operation has finished
     * even in case of errors.
     *
     * \param json the json message to send
     * \param handler the non-null handler
     */
    virtual void do_send(nlohmann::json json, network_send_handler handler) = 0;

public:
    /**
     * Constructor.
     *
     * \param server the parent
     */
    inline transport_client(transport_server& server) noexcept
        : parent_(server)
    {
    }

    /**
     * Get the transport server parent.
     *
     * \return the parent
     */
    inline const transport_server& parent() const noexcept
    {
        return parent_;
    }

    /**
     * Overloaded function.
     *
     * \return the parent
     */
    inline transport_server& parent() noexcept
    {
        return parent_;
    }

    /**
     * Get the current client state.
     *
     * \return the state
     */
    inline state_t state() const noexcept
    {
        return state_;
    }

    /**
     * Set the client state.
     *
     * \param state the new state
     */
    inline void set_state(state_t state) noexcept
    {
        state_ = state;
    }

    /**
     * Start receiving if not closed.
     *
     * \pre handler != nullptr
     * \param handler the handler
     */
    void recv(network_recv_handler handler);

    /**
     * Start sending if not closed.
     *
     * \param json the json message
     * \param handler the optional handler
     */
    void send(nlohmann::json json, network_send_handler handler = nullptr);

    /**
     * Convenient success message.
     *
     * \param cname the command name
     * \param handler the optional handler
     */
    void success(const std::string& cname, network_send_handler handler = nullptr);

    /**
     * Send a error message, the state is set to closing.
     *
     * The invocation is similar to:
     *
     * ````cpp
     * set_state(state_t::closing);
     * send(message, handler);
     * ````
     *
     * \pre message is not null
     * \pre data.is_object()
     * \param message the error message
     * \param handler the handler
     */
    void error(const nlohmann::json& data, network_send_handler handler = nullptr);

    /**
     * Convenient error overload.
     *
     * \param cname the command name
     * \pre !reason.empty()
     * \param reason the reason string
     * \param handler the optional handler
     */
    void error(const std::string& cname, const std::string& reason, network_send_handler handler = nullptr);

    /**
     * Convenient error overload.
     *
     * \pre !reason.empty()
     * \param reason the reason string
     * \param handler the handler
     */
    void error(const std::string& reason, network_send_handler handler = nullptr);

    /**
     * Convenient error overload.
     *
     * \param cname the command name
     * \param reason the error code
     * \param handler the optional handler
     */
    void error(const std::string& cname, network_errc reason, network_send_handler handler = nullptr);

    /**
     * Convenient error overload.
     *
     * \pre reason != network_errc::no_error
     * \param reason the reason string
     * \param handler the handler
     */
    void error(network_errc reason, network_send_handler handler = nullptr);
};

} // !irccd

#endif // !IRCCD_TRANSPORT_CLIENT_HPP
