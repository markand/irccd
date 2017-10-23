/*
 * transport.hpp -- irccd transports
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

#ifndef IRCCD_TRANSPORT_HPP
#define IRCCD_TRANSPORT_HPP

/**
 * \file transport.hpp
 * \brief Irccd transports.
 */

#include <cstdint>
#include <memory>
#include <string>

#include <boost/signals2/signal.hpp>

#include <json.hpp>

#include "net.hpp"
#include "sysconfig.hpp"

namespace irccd {

class transport_server;

/**
 * \brief Client connected to irccd.
 *
 * This class emits a warning upon clients request through onCommand signal.
 */
class transport_client {
public:
    /**
     * \brief Client state
     */
    enum class state {
        greeting,               //!< client is getting irccd info
        authenticating,         //!< client requires authentication
        ready,                  //!< client is ready to use
        closing                 //!< client must disconnect
    };

    /**
     * Signal: on_command
     * ----------------------------------------------------------
     *
     * Arguments:
     *   - the command
     */
    boost::signals2::signal<void (const nlohmann::json&)> on_command;

    /**
     * Signal: on_die
     * ----------------------------------------------------------
     *
     * The client has disconnected.
     */
    boost::signals2::signal<void ()> on_die;

private:
    void error(const std::string& msg);
    void flush() noexcept;
    void authenticate() noexcept;

protected:
    state state_{state::greeting};      //!< current client state
    transport_server& parent_;          //!< parent transport server
    net::TcpSocket socket_;             //!< socket
    std::string input_;                 //!< input buffer
    std::string output_;                //!< output buffer

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
    virtual unsigned recv(void* buffer, unsigned length);

    /**
     * Try to send some data into the given buffer.
     *
     * \param buffer the source buffer
     * \param length the buffer length
     * \return the number of bytes sent
     */
    virtual unsigned send(const void* buffer, unsigned length);

public:
    /**
     * Create a transport client from the socket.
     *
     * \pre socket must be valid
     * \param parent the parent server
     * \param socket the new socket
     */
    transport_client(transport_server& parent, net::TcpSocket socket);

    /**
     * Virtual destructor defaulted.
     */
    virtual ~transport_client() = default;

    /**
     * Get the client state.
     *
     * \return the client state
     */
    inline enum state state() const noexcept
    {
        return state_;
    }

    /**
     * Append some data to the output queue.
     *
     * \pre json.is_object()
     * \param json the json object
     */
    void send(const nlohmann::json& json);

    /**
     * \copydoc Service::prepare
     */
    virtual void prepare(fd_set& in, fd_set& out, net::Handle& max);

    /**
     * \copydoc Service::sync
     */
    virtual void sync(fd_set& in, fd_set& out);

    /**
     * Send a successful command to the client with optional extra data
     *
     * \pre extra must be null or object
     * \param cmd the command name
     * \param extra the optional extra data
     */
    void success(const std::string& cmd, nlohmann::json extra = nullptr);

    /**
     * Send an error status to the client.
     *
     * \pre extra must be null or object
     * \param cmd the command name
     * \param error the error string
     * \param extra the optional extra data
     */
    void error(const std::string& cmd,
               const std::string& error,
               nlohmann::json extra = nullptr);
};

/*
 * TransportClientTls
 * ------------------------------------------------------------------
 */

#if defined(HAVE_SSL)

/**
 * \brief TLS version of transport client.
 */
class transport_client_tls : public transport_client {
private:
    enum class handshake {
        write,
        read,
        ready
    } handshake_{handshake::ready};

    net::TlsSocket ssl_;

    void handshake();

protected:
    /**
     * \copydoc TransportClient::recv
     */
    unsigned recv(void* buffer, unsigned length) override;

    /**
     * \copydoc TransportClient::send
     */
    unsigned send(const void* buffer, unsigned length) override;

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
    transport_client_tls(const std::string& pkey,
                         const std::string& cert,
                         transport_server& server,
                         net::TcpSocket socket);

    /**
     * \copydoc TransportClient::prepare
     */
    void prepare(fd_set& in, fd_set& out, net::Handle& max) override;

    /**
     * \copydoc TransportClient::sync
     */
    void sync(fd_set& in, fd_set& out) override;
};

#endif  // !HAVE_SSL

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
 * | Domain                | Class                  |
 * |-----------------------|------------------------|
 * | IPv4, IPv6            | transport_server_ip    |
 * | Unix (not on Windows) | transport_server_local |
 *
 * Note: IPv4 and IPv6 can be combined, using TransportServer::IPv6 and its
 * option.
 */
class transport_server {
private:
    transport_server(const transport_server&) = delete;
    transport_server(transport_server&&) = delete;

    transport_server& operator=(const transport_server&) = delete;
    transport_server& operator=(transport_server&&) = delete;

protected:
    net::TcpSocket socket_;
    std::string password_;

public:
    /**
     * Default constructor.
     */
    inline transport_server(net::TcpSocket socket)
        : socket_(std::move(socket))
    {
    }

    /**
     * Get the socket handle for this transport.
     *
     * \return the handle
     */
    inline net::Handle handle() const noexcept
    {
        return socket_.handle();
    }

    /**
     * Get the password.
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
     * \return the password
     */
    inline void set_password(std::string password) noexcept
    {
        password_ = std::move(password);
    }

    /**
     * Destructor defaulted.
     */
    virtual ~transport_server() = default;

    /**
     * Accept a new client depending on the domain.
     *
     * \return the new client
     */
    virtual std::unique_ptr<transport_client> accept()
    {
        return std::make_unique<transport_client>(*this, socket_.accept());
    }
};

/**
 * \brief Create IP transport.
 */
class transport_server_ip : public transport_server {
public:
    /**
     * \brief Domain to use.
     */
    enum mode {
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
    transport_server_ip(const std::string& address,
                        std::uint16_t port,
                        std::uint8_t mode = v4);

    /**
     * Get the associated port.
     *
     * \return the port
     */
    std::uint16_t port() const;
};

#if defined(HAVE_SSL)

/**
 * \brief TLS over IP transport.
 */
class transport_server_tls : public transport_server_ip {
private:
    std::string privatekey_;
    std::string cert_;

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
    transport_server_tls(const std::string& pkey,
                         const std::string& cert,
                         const std::string& address,
                         std::uint16_t port,
                         std::uint8_t mode = mode::v4);

    /**
     * \copydoc TransportServer::accept
     */
    std::unique_ptr<transport_client> accept() override;
};

#endif // !HAVE_SSL

#if !defined(IRCCD_SYSTEM_WINDOWS)

/**
 * \brief Implementation of transports for Unix sockets.
 */
class transport_server_local : public transport_server {
private:
    std::string path_;

public:
    /**
     * Create a Unix transport.
     *
     * \param path the path
     */
    transport_server_local(std::string path);

    /**
     * Destroy the transport and remove the file.
     */
    ~transport_server_local();
};

#endif // !IRCCD_SYSTEM_WINDOWS

} // !irccd

#endif // !IRCCD_TRANSPORT_HPP
