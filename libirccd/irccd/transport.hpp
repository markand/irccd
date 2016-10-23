/*
 * transport.hpp -- irccd transports
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

#ifndef IRCCD_TRANSPORT_HPP
#define IRCCD_TRANSPORT_HPP

/**
 * \file transport.hpp
 * \brief Irccd transports.
 */

#include <cstdint>
#include <memory>
#include <string>

#include <json.hpp>

#include "net.hpp"
#include "signals.hpp"
#include "sysconfig.hpp"

namespace irccd {

class TransportServer;

/**
 * \class TransportClient
 * \brief Client connected to irccd.
 *
 * This class emits a warning upon clients request through onCommand signal.
 */
class TransportClient {
public:
    /**
     * \brief Client state
     */
    enum State {
        Greeting,               //!< client is getting irccd info
        Authenticating,         //!< client requires authentication
        Ready,                  //!< client is ready to use
        Closing                 //!< client must disconnect
    };

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

private:
    void error(const std::string &msg);
    void flush() noexcept;
    void authenticate() noexcept;

protected:
    State m_state{Greeting};    //!< current client state
    TransportServer &m_parent;  //!< parent transport server
    net::TcpSocket m_socket;    //!< socket
    std::string m_input;        //!< input buffer
    std::string m_output;       //!< output buffer

    /**
     * Fill the input buffer with available data.
     */
    void recv() noexcept;

    /**
     * Flush the output buffer from available pending data.
     */
    void send() noexcept;

    /**
     * Try to receive some data into the given buffer.
     *
     * \param buffer the destination buffer
     * \param length the buffer length
     * \return the number of bytes received
     */
    IRCCD_EXPORT virtual unsigned recv(void *buffer, unsigned length);

    /**
     * Try to send some data into the given buffer.
     *
     * \param buffer the source buffer
     * \param length the buffer length
     * \return the number of bytes sent
     */
    IRCCD_EXPORT virtual unsigned send(const void *buffer, unsigned length);

public:
    /**
     * Create a transport client from the socket.
     *
     * \pre socket must be valid
     * \param parent the parent server
     * \param socket the new socket
     */
    IRCCD_EXPORT TransportClient(TransportServer &parent, net::TcpSocket socket);

    /**
     * Virtual destructor defaulted.
     */
    virtual ~TransportClient() = default;

    /**
     * Get the client state.
     *
     * \return the client state
     */
    inline State state() const noexcept
    {
        return m_state;
    }

    /**
     * Append some data to the output queue.
     *
     * \pre json.is_object()
     * \param json the json object
     */
    IRCCD_EXPORT void send(const nlohmann::json &json);

    /**
     * \copydoc Service::prepare
     */
    IRCCD_EXPORT virtual void prepare(fd_set &in, fd_set &out, net::Handle &max);

    /**
     * \copydoc Service::sync
     */
    IRCCD_EXPORT virtual void sync(fd_set &in, fd_set &out);

    /**
     * Send a successful command to the client with optional extra data
     *
     * \pre extra must be null or object
     * \param cmd the command name
     * \param extra the optional extra data
     */
    IRCCD_EXPORT void success(const std::string &cmd, nlohmann::json extra = nullptr);

    /**
     * Send an error status to the client.
     *
     * \pre extra must be null or object
     * \param cmd the command name
     * \param error the error string
     * \param extra the optional extra data
     */
    IRCCD_EXPORT void error(const std::string &cmd,
                            const std::string &error,
                            nlohmann::json extra = nullptr);
};

/*
 * TransportClientTls
 * ------------------------------------------------------------------
 */

/**
 * \brief TLS version of transport client.
 */
class TransportClientTls : public TransportClient {
private:
    enum {
        HandshakeWrite,
        HandshakeRead,
        HandshakeReady
    } m_handshake{HandshakeReady};

    net::TlsSocket m_ssl;

    void handshake();

protected:
    /**
     * \copydoc TransportClient::recv
     */
    unsigned recv(void *buffer, unsigned length) override;

    /**
     * \copydoc TransportClient::send
     */
    unsigned send(const void *buffer, unsigned length) override;

public:
    /**
     * Create the transport client.
     *
     * \pre socket.isOpen()
     * \param pkey the private key
     * \param cert the certificate file
     * \param socket the accepted socket
     * \param parent the parent server
     * \param socket the new socket
     */
    IRCCD_EXPORT TransportClientTls(const std::string &pkey,
                                    const std::string &cert,
                                    TransportServer &server,
                                    net::TcpSocket socket);

    /**
     * \copydoc TransportClient::prepare
     */
    IRCCD_EXPORT virtual void prepare(fd_set &in, fd_set &out, net::Handle &max);

    /**
     * \copydoc TransportClient::sync
     */
    IRCCD_EXPORT virtual void sync(fd_set &in, fd_set &out);
};

/*
 * TransportServer
 * ------------------------------------------------------------------
 */

/**
 * \brief Bring networking between irccd and irccdctl.
 *
 * This class contains a master sockets for listening to TCP connections, it is
 * then processed by irccd.
 *
 * The transport class supports the following domains:
 *
 * | Domain                | Class                 |
 * |-----------------------|-----------------------|
 * | IPv4, IPv6            | TransportServerIp     |
 * | Unix (not on Windows) | TransportServerUnix   |
 *
 * Note: IPv4 and IPv6 can be combined, using TransportServer::IPv6 and its
 * option.
 */
class TransportServer {
private:
    TransportServer(const TransportServer &) = delete;
    TransportServer(TransportServer &&) = delete;

    TransportServer &operator=(const TransportServer &) = delete;
    TransportServer &operator=(TransportServer &&) = delete;

protected:
    net::TcpSocket m_socket;
    std::string m_password;

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
     * Get the password.
     *
     * \return the password
     */
    inline const std::string &password() const noexcept
    {
        return m_password;
    }

    /**
     * Set an optional password.
     *
     * \return the password
     */
    inline void setPassword(std::string password) noexcept
    {
        m_password = std::move(password);
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
        return std::make_unique<TransportClient>(*this, m_socket.accept());
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

/**
 * \brief TLS over IP transport.
 */
class TransportServerTls : public TransportServerIp {
private:
    std::string m_privatekey;
    std::string m_cert;

public:
    /**
     * Constructor.
     * \pre mode > 0
     * \param pkey the private key file
     * \param cert the certificate file
     * \param address the address (* for any)
     * \param port the port number
     * \param mode the domains to use (can be OR'ed)
     */
    IRCCD_EXPORT TransportServerTls(const std::string &pkey,
                                    const std::string &cert,
                                    const std::string &address,
                                    std::uint16_t port,
                                    std::uint8_t mode = v4);

    /**
     * \copydoc TransportServer::accept
     */
    IRCCD_EXPORT std::unique_ptr<TransportClient> accept() override;
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

#endif // !IRCCD_SYSTEM_WINDOWS

} // !irccd

#endif // !IRCCD_TRANSPORT_HPP
