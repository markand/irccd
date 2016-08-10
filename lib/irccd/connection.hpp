/*
 * connection.hpp -- value wrapper for connecting to irccd
 *
 * Copyright (c) 2013-2016 David Demelier <markand@malikania.fr>
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

#ifndef IRCCD_CONNECTION_HPP
#define IRCCD_CONNECTION_HPP

/**
 * \file connection.hpp
 * \brief Connection to irccd instance.
 */

#include <cassert>
#include <memory>
#include <string>

#include "net.hpp"
#include "signals.hpp"
#include "service.hpp"

namespace irccd {

/**
 * \brief Low level connection to irccd instance.
 *
 * This class is an event-based connection to an irccd instance. You can use
 * it directly if you want to issue commands to irccd in an asynchronous way.
 *
 * Being asynchronous makes mixing the event loop with this connection easier.
 *
 * It is implemented as a finite state machine as it may requires several
 * roundtrips between the controller and irccd.
 *
 * Be aware that there are no namespaces for commands, if you plan to use
 * Irccdctl class and you also connect the onMessage signal, irccdctl will also
 * use it. Do not use irccdctl directly if this is a concern.
 *
 * The state may change as following.
 *
 *   [o]
 *    |       +----------------------------+
 *    v       v                            |
 * +--------------+   +----------+     +----------------+
 * | Disconnected |-->| Checking |---->| Authenticating |
 * +--------------+   +----------+     +----------------+
 *     ^       |            ^              |
 *     |       |            |              v
 *     |       |      +------------+   +-------+
 *     |       +----->| Connecting |<--| Ready |
 *     |              +------------+   +-------+
 *     |                                   |
 *     ------------------------------------+
 *
 * Note: authenticating state is not implemented yet.
 */
class Connection : public Service {
public:
    /**
     * \brief The current connection state.
     */
    enum Status {
        Disconnected,       //!< Socket is closed
        Connecting,         //!< Connection is in progress
        Checking,           //!< Connection is checking irccd daemon
        Authenticating,     //!< Connection is authenticating
        Ready               //!< Socket is ready for I/O
    };

    /**
     * \brief Irccd information.
     */
    class Info {
    public:
        unsigned short major;
        unsigned short minor;
        unsigned short patch;
    };

    /**
     * onConnect
     * --------------------------------------------------------------
     *
     * Connection was successfull.
     */
    Signal<const Info &> onConnect;

    /**
     * onMessage
     * ---------------------------------------------------------------
     *
     * Upon message.
     */
    Signal<const nlohmann::json &> onMessage;

    /**
     * onDisconnect
     * --------------------------------------------------------------
     *
     * A fatal error occured resulting in disconnection.
     */
    Signal<const std::string &> onDisconnect;

private:
    std::string m_input;
    std::string m_output;

public:
    /**
     * \brief TEST
     */
    class State;
    class DisconnectedState;
    class ConnectingState;
    /**
     * \brief TEST
     */
    class CheckingState;
    class ReadyState;

private:
    std::unique_ptr<State> m_state;
    std::unique_ptr<State> m_stateNext;

protected:
    net::TcpSocket m_socket{net::Invalid};

    void syncInput();
    void syncOutput();

public:
    /**
     * Default constructor.
     */
    Connection();

    /**
     * Default destructor.
     */
    virtual ~Connection();

    /**
     * Send an asynchronous request to irccd.
     *
     * \pre json.is_object
     * \param json the JSON object
     */
    inline void request(const nlohmann::json &json)
    {
        assert(json.is_object());

        m_output += json.dump();
        m_output += "\r\n\r\n";
    }

    /**
     * Get the underlying socket handle.
     *
     * \return the handle
     */
    inline net::Handle handle() const noexcept
    {
        return m_socket.handle();
    }

    /**
     * Shorthand for state() != Disconnected.
     *
     * \return true if state() != Disconnected
     */
    inline bool isConnected() const noexcept
    {
        return status() != Disconnected;
    }

    /**
     * Get the current state.
     *
     * \return the state
     */
    Status status() const noexcept;

    /**
     * Initiate connection to irccd.
     *
     * \pre state() == Disconnected
     * \param address the address
     */
    virtual void connect(const net::Address &address);

    /**
     * \copydoc Service::prepare
     */
    void prepare(fd_set &in, fd_set &out, net::Handle &max) override;

    /**
     * \copydoc Service::sync
     */
    void sync(fd_set &in, fd_set &out) override;
};

} // !irccd

#endif // !IRCCD_CONNECTION_HPP
