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

void Connection::syncInput()
{
    try {
        std::string buffer;

        buffer.resize(512);
        buffer.resize(m_socket.recv(&buffer[0], buffer.size()));

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
        auto ns = m_socket.send(m_output.data(), m_output.length());

        if (ns > 0)
            m_output.erase(0, ns);
    } catch (const std::exception &ex) {
        m_stateNext = std::make_unique<DisconnectedState>();
        onDisconnect(ex.what());
    }
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

} // !irccd
