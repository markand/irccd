/*
 * transport_client.hpp -- server side transport clients
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

#ifndef IRCCD_DAEMON_TRANSPORT_CLIENT_HPP
#define IRCCD_DAEMON_TRANSPORT_CLIENT_HPP

#include <irccd/network_stream.hpp>

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
     * Virtual destructor defaulted.
     */
    virtual ~transport_client() = default;

    /**
     * Get the transport server parent.
     *
     * \return the parent
     */
    inline const transport_server& get_parent() const noexcept
    {
        return parent_;
    }

    /**
     * Overloaded function.
     *
     * \return the parent
     */
    inline transport_server& get_parent() noexcept
    {
        return parent_;
    }

    /**
     * Get the current client state.
     *
     * \return the state
     */
    inline state_t get_state() const noexcept
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
     * Possible error codes:
     *
     *   - boost::system::errc::network_down in case of errors,
     *   - boost::system::errc::invalid_argument if the JSON message is invalid.
     *
     * \pre handler != nullptr
     * \param handler the handler
     */
    void recv(network_recv_handler handler);

    /**
     * Start sending if not closed.
     *
     * Possible error codes:
     *
     *   - boost::system::errc::network_down in case of errors,
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
     * Send an error code to the client.
     *
     * \pre code is not 0
     * \param code the error code
     * \param handler the optional handler
     */
    void error(boost::system::error_code code, network_send_handler handler = nullptr);

    /**
     * Send an error code to the client.
     *
     * \pre code is not 0
     * \param code the error code
     * \param cname the command name
     * \param handler the optional handler
     */
    void error(boost::system::error_code code,
               std::string cname,
               network_send_handler handler = nullptr);
};

} // !irccd

#endif // !IRCCD_DAEMON_TRANSPORT_CLIENT_HPP
