/*
 * transport-server.hpp -- I/O for irccd clients (acceptors)
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

#ifndef IRCCD_TRANSPORT_SERVER_HPP
#define IRCCD_TRANSPORT_SERVER_HPP

/**
 * \file transport-server.hpp
 * \brief Transports for irccd
 */

#include <memory>
#include <string>

#include "net.hpp"
#include "sysconfig.hpp"
#include "transport-client.hpp"

namespace irccd {

/**
 * \brief Bring networking between irccd and irccdctl.
 *
 * This class contains a master sockets for listening to TCP connections, it is then processed by irccd.
 *
 * The transport class supports the following domains:
 *
 * | Domain                | Class                 |
 * |-----------------------|-----------------------|
 * | IPv4, IPv6            | TransportServerIp     |
 * | Unix (not on Windows) | TransportServerUnix   |
 *
 * Note: IPv4 and IPv6 can be combined, using TransportServer::IPv6 and its option.
 */
class TransportServer {
private:
    TransportServer(const TransportServer &) = delete;
    TransportServer(TransportServer &&) = delete;

    TransportServer &operator=(const TransportServer &) = delete;
    TransportServer &operator=(TransportServer &&) = delete;

protected:
    /**
     * The socket handle.
     */
    net::TcpSocket m_socket;

public:
    /**
     * Default constructor.
     */
    inline TransportServer(net::TcpSocket socket)
        : m_socket(std::move(socket))
    {
    }

    /**
     * Get the socket handle for this transport.
     *
     * \return the handle
     */
    inline net::Handle handle() const noexcept
    {
        return m_socket.handle();
    }

    /**
     * Destructor defaulted.
     */
    virtual ~TransportServer() = default;

    /**
     * Accept a new client depending on the domain.
     *
     * \return the new client
     */
    virtual std::unique_ptr<TransportClient> accept()
    {
        return std::make_unique<TransportClient>(m_socket.accept());
    }
};

/**
 * \brief Create IP transport.
 */
class TransportServerIp : public TransportServer {
public:
    /**
     * \brief Domain to use.
     */
    enum Mode {
        v4 = (1 << 0),      //!< IPv6
        v6 = (1 << 1)       //!< IPv4
    };

    /**
     * Constructor.
     * \pre mode > 0
     * \param address the address (* for any)
     * \param port the port number
     * \param mode the domains to use (can be OR'ed)
     */
    IRCCD_EXPORT TransportServerIp(const std::string &address,
                                   std::uint16_t port,
                                   std::uint8_t mode = v4);
};

#if !defined(IRCCD_SYSTEM_WINDOWS)

/**
 * \brief Implementation of transports for Unix sockets.
 */
class TransportServerLocal : public TransportServer {
private:
    std::string m_path;

public:
    /**
     * Create a Unix transport.
     *
     * \param path the path
     */
    IRCCD_EXPORT TransportServerLocal(std::string path);

    /**
     * Destroy the transport and remove the file.
     */
    IRCCD_EXPORT ~TransportServerLocal();
};

#endif // !_WIN32

} // !irccd

#endif // !IRCCD_TRANSPORT_SERVER_HPP
