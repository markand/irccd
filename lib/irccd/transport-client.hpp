/*
 * transport-client.hpp -- client connected to irccd
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

#ifndef IRCCD_TRANSPORT_CLIENT_HPP
#define IRCCD_TRANSPORT_CLIENT_HPP

/**
 * \file transport-client.hpp
 * \brief Client connected to irccd
 */

#include <functional>
#include <memory>
#include <stdexcept>
#include <string>

#include <json.hpp>

#include "net.hpp"
#include "server.hpp"
#include "signals.hpp"
#include "sysconfig.hpp"

namespace irccd {

/**
 * \class TransportClient
 * \brief Client connected to irccd.
 *
 * This class emits a warning upon clients request through onCommand signal.
 */
class TransportClient {
public:
    /**
     * Signal: onCommand
     * ----------------------------------------------------------
     *
     * Arguments:
     *   - the command
     */
    Signal<const nlohmann::json &> onCommand;

    /**
     * Signal: onDie
     * ----------------------------------------------------------
     *
     * The client has disconnected.
     */
    Signal<> onDie;

protected:
    net::TcpSocket m_socket;    //!< socket
    std::string m_input;        //!< input buffer
    std::string m_output;       //!< output buffer

    /**
     * Parse input buffer.
     *
     * \param buffer the buffer.
     */
    void parse(const std::string &buffer);

private:
    void receive();
    void send();

public:
    inline TransportClient(net::TcpSocket socket)
        : m_socket(std::move(socket))
    {
    }

    /**
     * Virtual destructor defaulted.
     */
    virtual ~TransportClient() = default;

    IRCCD_EXPORT void send(const nlohmann::json &json);

    IRCCD_EXPORT virtual void prepare(fd_set &in, fd_set &out, net::Handle &max);

    IRCCD_EXPORT virtual void sync(fd_set &in, fd_set &out);
};

} // !irccd

#endif // !IRCCD_TRANSPORT_CLIENT_HPP
