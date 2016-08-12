/*
 * transport.cpp -- irccd transports
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

#include <cassert>
#include <cstdio>

#include "transport.hpp"

namespace irccd {

/*
 * TransportClient
 * ------------------------------------------------------------------
 */

void TransportClient::parse(const std::string &message)
{
    auto document = nlohmann::json::parse(message);

    if (document.is_object())
        onCommand(document);
}

unsigned TransportClient::recv(char *buffer, unsigned length)
{
    return m_socket.recv(buffer, length);
}

unsigned TransportClient::send(const char *buffer, unsigned length)
{
    return m_socket.send(buffer, length);
}

void TransportClient::syncInput()
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

void TransportClient::syncOutput()
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

void TransportClient::prepare(fd_set &in, fd_set &out, net::Handle &max)
{
    if (m_socket.handle() > max)
        max = m_socket.handle();

    FD_SET(m_socket.handle(), &in);

    if (!m_output.empty())
        FD_SET(m_socket.handle(), &out);
}

void TransportClient::sync(fd_set &in, fd_set &out)
{
    // Do some I/O.
    if (FD_ISSET(m_socket.handle(), &in))
        syncInput();
    if (FD_ISSET(m_socket.handle(), &out))
        syncOutput();

    // Flush the queue.
    for (std::size_t pos; (pos = m_input.find("\r\n\r\n")) != std::string::npos; ) {
        auto message = m_input.substr(0, pos);

        m_input.erase(m_input.begin(), m_input.begin() + pos + 4);

        parse(message);
    }
}

void TransportClient::send(const nlohmann::json &json)
{
    assert(json.is_object());

    m_output += json.dump();
    m_output += "\r\n\r\n";
}

/*
 * TransportClientTls
 * ------------------------------------------------------------------
 */

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
                                       net::TcpSocket socket)
    : TransportClient(std::move(socket))
    , m_ssl(m_socket)
{
    m_ssl.setPrivateKey(pkey);
    m_ssl.setCertificate(cert);

    handshake();
}

unsigned TransportClientTls::recv(char *buffer, unsigned length)
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

unsigned TransportClientTls::send(const char *buffer, unsigned length)
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
        if (address == "*")
            m_socket.bind(net::ipv6::any(port));
        else
            m_socket.bind(net::ipv6::pton(address, port));

        // Disable or enable IPv4 when using IPv6.
        if (!(mode & v4))
            m_socket.set(net::option::Ipv6Only(true));
    } else {
        if (address == "*")
            m_socket.bind(net::ipv4::any(port));
        else
            m_socket.bind(net::ipv4::pton(address, port));
    }

    m_socket.listen();
}

/*
 * TransportServerTls
 * ------------------------------------------------------------------
 */

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
    return std::make_unique<TransportClientTls>(m_privatekey, m_cert, m_socket.accept());
}

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
