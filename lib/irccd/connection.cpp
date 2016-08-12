/*
 * connection.cpp -- value wrapper for connecting to irccd
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

#include <stdexcept>

#include "connection.hpp"
#include "conn-state-connecting.hpp"
#include "conn-state-checking.hpp"
#include "conn-state-disconnected.hpp"
#include "util.hpp"

namespace irccd {

/*
 * Connection.
 * ------------------------------------------------------------------
 */

void Connection::syncInput()
{
    try {
        std::string buffer;

        buffer.resize(512);
        buffer.resize(recv(&buffer[0], buffer.size()));

        if (buffer.empty())
            throw std::runtime_error("connection lost");

        m_input += std::move(buffer);
    } catch (const std::exception &ex) {
        m_stateNext = std::make_unique<DisconnectedState>();
        onDisconnect(ex.what());
    }
}

void Connection::syncOutput()
{
    try {
        auto ns = send(m_output.data(), m_output.length());

        if (ns > 0)
            m_output.erase(0, ns);
    } catch (const std::exception &ex) {
        m_stateNext = std::make_unique<DisconnectedState>();
        onDisconnect(ex.what());
    }
}

unsigned Connection::recv(char *buffer, unsigned length)
{
    return m_socket.recv(buffer, length);
}

unsigned Connection::send(const char *buffer, unsigned length)
{
    return m_socket.send(buffer, length);
}

Connection::Connection()
    : m_state(std::make_unique<DisconnectedState>())
{
}

Connection::~Connection() = default;

Connection::Status Connection::status() const noexcept
{
    return m_state->status();
}

void Connection::connect(const net::Address &address)
{
    assert(status() == Disconnected);

    try {
        m_socket = net::TcpSocket(address.domain(), 0);
        m_socket.set(net::option::SockBlockMode(false));
        m_socket.connect(address);
        m_state = std::make_unique<CheckingState>();
    } catch (const net::WouldBlockError &) {
        m_state = std::make_unique<ConnectingState>();
    } catch (const std::exception &ex) {
        m_state = std::make_unique<DisconnectedState>();
        onDisconnect(ex.what());
    }
}

void Connection::prepare(fd_set &in, fd_set &out, net::Handle &max)
{
    try {
        m_state->prepare(*this, in, out);

        if (m_socket.handle() > max)
            max = m_socket.handle();
    } catch (const std::exception &ex) {
        m_state = std::make_unique<DisconnectedState>();
        onDisconnect(ex.what());
    }
}

void Connection::sync(fd_set &in, fd_set &out)
{
    try {
        m_state->sync(*this, in, out);

        if (m_stateNext) {
            m_state = std::move(m_stateNext);
            m_stateNext = nullptr;
        }
    } catch (const std::exception &ex) {
        m_state = std::make_unique<DisconnectedState>();
        onDisconnect(ex.what());
    }
}

/*
 * TlsConnection.
 * ------------------------------------------------------------------
 */

void TlsConnection::handshake()
{
    try {
        m_ssl->handshake();
        m_handshake = HandshakeReady;
    } catch (const net::WantReadError &) {
        m_handshake = HandshakeRead;
    } catch (const net::WantWriteError &) {
        m_handshake = HandshakeWrite;
    } catch (const std::exception &ex) {
        m_state = std::make_unique<DisconnectedState>();
        onDisconnect(ex.what());
    }
}

unsigned TlsConnection::recv(char *buffer, unsigned length)
{
    unsigned nread = 0;

    try {
        nread = m_ssl->recv(buffer, length);
    } catch (const net::WantReadError &) {
        m_handshake = HandshakeRead;
    } catch (const net::WantWriteError &) {
        m_handshake = HandshakeWrite;
    }

    return nread;
}

unsigned TlsConnection::send(const char *buffer, unsigned length)
{
    unsigned nsent = 0;

    try {
        nsent = m_ssl->send(buffer, length);
    } catch (const net::WantReadError &) {
        m_handshake = HandshakeRead;
    } catch (const net::WantWriteError &) {
        m_handshake = HandshakeWrite;
    }

    return nsent;
}

void TlsConnection::connect(const net::Address &address)
{
    Connection::connect(address);

    m_ssl = std::make_unique<net::TlsSocket>(m_socket, net::TlsSocket::Client);
}

void TlsConnection::prepare(fd_set &in, fd_set &out, net::Handle &max)
{
    if (m_state->status() == Connecting)
        Connection::prepare(in, out, max);
    else {
        if (m_socket.handle() > max)
            max = m_socket.handle();

        /*
         * Attempt an immediate handshake immediately if connection succeeded
         * in last iteration.
         */
        if (m_handshake == HandshakeUndone)
            handshake();

        switch (m_handshake) {
        case HandshakeRead:
            FD_SET(m_socket.handle(), &in);
            break;
        case HandshakeWrite:
            FD_SET(m_socket.handle(), &out);
            break;
        default:
            Connection::prepare(in, out, max);
        }
    }
}

void TlsConnection::sync(fd_set &in, fd_set &out)
{
    if (m_state->status() == Connecting)
        Connection::sync(in, out);
    else if (m_handshake != HandshakeReady)
        handshake();
    else
        Connection::sync(in, out);
}

} // !irccd
