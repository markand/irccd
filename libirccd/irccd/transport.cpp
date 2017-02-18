/*
 * transport.cpp -- irccd transports
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

#include <cassert>
#include <cstdio>

#include "transport.hpp"

namespace irccd {

/*
 * TransportClient
 * ------------------------------------------------------------------
 */

void TransportClient::error(const std::string &msg)
{
    m_state = Closing;

    send({{ "error", msg }});
}

void TransportClient::flush() noexcept
{
    for (std::size_t pos; (pos = m_input.find("\r\n\r\n")) != std::string::npos; ) {
        auto message = m_input.substr(0, pos);

        m_input.erase(m_input.begin(), m_input.begin() + pos + 4);

        try {
            auto document = nlohmann::json::parse(message);

            if (!document.is_object())
                error("invalid argument");
            else
                onCommand(document);
        } catch (const std::exception &ex) {
            error(ex.what());
        }
    }
}

void TransportClient::authenticate() noexcept
{
    auto pos = m_input.find("\r\n\r\n");

    if (pos == std::string::npos)
        return;

    auto msg = m_input.substr(0, pos);

    m_input.erase(m_input.begin(), m_input.begin() + pos + 4);

    try {
        auto doc = nlohmann::json::parse(msg);

        if (!doc.is_object())
            error("invalid argument");

        auto cmd = doc.find("command");

        if (cmd == doc.end() || !cmd->is_string() || *cmd != "auth")
            error("authentication required");

        auto pw = doc.find("password");
        auto result = true;

        if (pw == doc.end() || !pw->is_string() || *pw != m_parent.password()) {
            m_state = Closing;
            result = false;
        } else
            m_state = Ready;

        send({
            { "response", "auth" },
            { "result", result }
        });
    } catch (const std::exception &ex) {
        error(ex.what());
    }
}

void TransportClient::recv() noexcept
{
    try {
        std::string buffer;

        buffer.resize(512);
        buffer.resize(recv(&buffer[0], buffer.size()));

        if (buffer.empty())
            onDie();

        m_input += std::move(buffer);
    } catch (const std::exception &) {
        onDie();
    }
}

void TransportClient::send() noexcept
{
    try {
        auto ns = send(&m_output[0], m_output.size());

        if (ns == 0)
            onDie();

        m_output.erase(0, ns);
    } catch (const std::exception &ex) {
        onDie();
    }
}

unsigned TransportClient::recv(void *buffer, unsigned length)
{
    return m_socket.recv(buffer, length);
}

unsigned TransportClient::send(const void *buffer, unsigned length)
{
    return m_socket.send(buffer, length);
}

TransportClient::TransportClient(TransportServer &parent, net::TcpSocket socket)
    : m_parent(parent)
    , m_socket(std::move(socket))
{
    assert(m_socket.isOpen());

    m_socket.set(net::option::SockBlockMode(false));

    // Send some information.
    auto object = nlohmann::json::object({
        { "program",    "irccd"                 },
        { "major",      IRCCD_VERSION_MAJOR     },
        { "minor",      IRCCD_VERSION_MINOR     },
        { "patch",      IRCCD_VERSION_PATCH     }
    });

#if defined(WITH_JS)
    object.push_back({"javascript", true});
#endif
#if defined(WITH_SSL)
    object.push_back({"ssl", true});
#endif

    send(object);
}

void TransportClient::prepare(fd_set &in, fd_set &out, net::Handle &max)
{
    if (m_socket.handle() > max)
        max = m_socket.handle();

    switch (m_state) {
    case Greeting:
        FD_SET(m_socket.handle(), &in);
        FD_SET(m_socket.handle(), &out);
        break;
    case Authenticating:
        FD_SET(m_socket.handle(), &in);
        break;
    case Ready:
        FD_SET(m_socket.handle(), &in);

        if (!m_output.empty())
            FD_SET(m_socket.handle(), &out);
        break;
    case Closing:
        if (!m_output.empty())
            FD_SET(m_socket.handle(), &out);
        else
            onDie();
        break;
    default:
        break;
    }
}

void TransportClient::sync(fd_set &in, fd_set &out)
{
    switch (m_state) {
    case Greeting:
        if (FD_ISSET(m_socket.handle(), &in))
            recv();
        else if (FD_ISSET(m_socket.handle(), &out))
            send();

        if (m_output.empty())
            m_state = m_parent.password().empty() ? Ready : Authenticating;

        break;
    case Authenticating:
        if (FD_ISSET(m_socket.handle(), &in))
            recv();

        authenticate();
        break;
    case Ready:
        if (FD_ISSET(m_socket.handle(), &in))
            recv();
        if (FD_ISSET(m_socket.handle(), &out))
            send();

        flush();
        break;
    case Closing:
        if (FD_ISSET(m_socket.handle(), &out))
            send();
        break;
    default:
        break;
    }
}

void TransportClient::send(const nlohmann::json &json)
{
    assert(json.is_object());

    m_output += json.dump();
    m_output += "\r\n\r\n";
}

void TransportClient::success(const std::string &cmd, nlohmann::json extra)
{
    assert(extra.is_object() || extra.is_null());

    if (!extra.is_object())
        extra = nlohmann::json::object();

    extra["command"] = cmd;
    extra["status"] = true;

    m_output += extra.dump();
    m_output += "\r\n\r\n";
}

void TransportClient::error(const std::string &cmd, const std::string &error, nlohmann::json extra)
{
    assert(extra.is_object() || extra.is_null());

    if (!extra.is_object())
        extra = nlohmann::json::object();

    extra["command"] = cmd;
    extra["status"] = false;
    extra["error"] = error;

    m_output += extra.dump();
    m_output += "\r\n\r\n";
}

/*
 * TransportClientTls
 * ------------------------------------------------------------------
 */

#if defined(WITH_SSL)

void TransportClientTls::handshake()
{
    try {
        m_ssl.handshake();
        m_handshake = HandshakeReady;
    } catch (const net::WantReadError &) {
        m_handshake = HandshakeRead;
    } catch (const net::WantWriteError &) {
        m_handshake = HandshakeWrite;
    } catch (const std::exception &) {
        onDie();
    }
}

TransportClientTls::TransportClientTls(const std::string &pkey,
                                       const std::string &cert,
                                       TransportServer &server,
                                       net::TcpSocket socket)
    : TransportClient(server, std::move(socket))
    , m_ssl(m_socket)
{
    m_ssl.setPrivateKey(pkey);
    m_ssl.setCertificate(cert);

    handshake();
}

unsigned TransportClientTls::recv(void *buffer, unsigned length)
{
    unsigned nread = 0;

    try {
        nread = m_ssl.recv(buffer, length);
    } catch (const net::WantReadError &) {
        m_handshake = HandshakeRead;
    } catch (const net::WantWriteError &) {
        m_handshake = HandshakeWrite;
    }

    return nread;
}

unsigned TransportClientTls::send(const void *buffer, unsigned length)
{
    unsigned nsent = 0;

    try {
        nsent = m_ssl.send(buffer, length);
    } catch (const net::WantReadError &) {
        m_handshake = HandshakeRead;
    } catch (const net::WantWriteError &) {
        m_handshake = HandshakeWrite;
    }

    return nsent;
}

void TransportClientTls::prepare(fd_set &in, fd_set &out, net::Handle &max)
{
    if (m_socket.handle() > max)
        max = m_socket.handle();

    switch (m_handshake) {
    case HandshakeRead:
        FD_SET(m_socket.handle(), &in);
        break;
    case HandshakeWrite:
        FD_SET(m_socket.handle(), &out);
        break;
    default:
        TransportClient::prepare(in, out, max);
        break;
    }
}

void TransportClientTls::sync(fd_set &in, fd_set &out)
{
    switch (m_handshake) {
    case HandshakeRead:
    case HandshakeWrite:
        handshake();
        break;
    default:
        TransportClient::sync(in, out);
    }
}

#endif  // !WITH_SSL

/*
 * TransportServerIp
 * ------------------------------------------------------------------
 */

TransportServerIp::TransportServerIp(const std::string &address,
                                     std::uint16_t port,
                                     std::uint8_t mode)
    : TransportServer(net::TcpSocket((mode & v6) ? AF_INET6 : AF_INET, 0))
{
    assert((mode & v6) || (mode & v4));

    m_socket.set(net::option::SockReuseAddress(true));

    if (mode & v6) {
        // Disable or enable IPv4 when using IPv6.
        if (!(mode & v4))
            m_socket.set(net::option::Ipv6Only(true));

        if (address == "*")
            m_socket.bind(net::ipv6::any(port));
        else
            m_socket.bind(net::ipv6::pton(address, port));
    } else {
        if (address == "*")
            m_socket.bind(net::ipv4::any(port));
        else
            m_socket.bind(net::ipv4::pton(address, port));
    }

    m_socket.listen();
}

std::uint16_t TransportServerIp::port() const
{
    auto addr = m_socket.getsockname();

    return addr.domain() == AF_INET
        ? ntohs(addr.as<sockaddr_in>().sin_port)
        : ntohs(addr.as<sockaddr_in6>().sin6_port);
}

/*
 * TransportServerTls
 * ------------------------------------------------------------------
 */

#if defined(WITH_SSL)

TransportServerTls::TransportServerTls(const std::string &pkey,
                                       const std::string &cert,
                                       const std::string &address,
                                       std::uint16_t port,
                                       std::uint8_t mode)
    : TransportServerIp(address, port, mode)
    , m_privatekey(pkey)
    , m_cert(cert)
{
}

std::unique_ptr<TransportClient> TransportServerTls::accept()
{
    return std::make_unique<TransportClientTls>(m_privatekey, m_cert, *this, m_socket.accept());
}

#endif  // !WITH_SSL

/*
 * TransportServerLocal
 * ------------------------------------------------------------------
 */

#if !defined(IRCCD_SYSTEM_WINDOWS)

TransportServerLocal::TransportServerLocal(std::string path)
    : TransportServer(net::TcpSocket(AF_LOCAL, 0))
    , m_path(std::move(path))
{
    m_socket.bind(net::local::create(m_path, true));
    m_socket.listen();
}

TransportServerLocal::~TransportServerLocal()
{
    ::remove(m_path.c_str());
}

#endif

} // !irccd
