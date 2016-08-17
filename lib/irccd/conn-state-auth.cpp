/*
 * conn-state-auth.cpp -- connection is authenticating
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

#include "conn-state-auth.hpp"
#include "conn-state-disconnected.hpp"
#include "conn-state-ready.hpp"

namespace irccd {

void Connection::AuthState::send(Connection &cnt) noexcept
{
    try {
        auto n = cnt.send(m_output.data(), m_output.size());

        if (n == 0) {
            m_output.clear();
            throw std::runtime_error("connection lost");
        }

        m_output.erase(0, n);

        if (m_output.empty())
            m_auth = Checking;
    } catch (const std::exception &ex) {
        cnt.m_state = std::make_unique<DisconnectedState>();
        cnt.onDisconnect(ex.what());
    }
}

void Connection::AuthState::check(Connection &cnt) noexcept
{
    cnt.syncInput();

    auto msg = util::nextNetwork(cnt.m_input);

    if (msg.empty())
        return;

    try {
        auto doc = nlohmann::json::parse(msg);

        if (!doc.is_object())
            throw std::invalid_argument("invalid argument");

        auto cmd = doc.find("response");

        if (cmd == doc.end() || !cmd->is_string() || *cmd != "auth")
            throw std::invalid_argument("authentication result expected");

        auto result = doc.find("result");

        if (result == doc.end() || !result->is_boolean())
            throw std::invalid_argument("bad protocol");

        if (!*result)
            throw std::runtime_error("authentication failed");

        cnt.m_state = std::make_unique<ReadyState>();
    } catch (const std::exception &ex) {
        cnt.m_state = std::make_unique<DisconnectedState>();
        cnt.onDisconnect(ex.what());
    }
}

Connection::Status Connection::AuthState::status() const noexcept
{
    return Authenticating;
}

void Connection::AuthState::prepare(Connection &cnt, fd_set &in, fd_set &out)
{
    switch (m_auth) {
    case Created:
        m_auth = Sending;
        m_output += nlohmann::json({
            { "command", "auth" },
            { "password", cnt.m_password }
        }).dump();
        m_output += "\r\n\r\n";

        // FALLTHROUGH
    case Sending:
        FD_SET(cnt.m_socket.handle(), &out);
        break;
    case Checking:
        FD_SET(cnt.m_socket.handle(), &in);
        break;
    default:
        break;
    }
}

void Connection::AuthState::sync(Connection &cnt, fd_set &in, fd_set &out)
{
    switch (m_auth) {
    case Sending:
        if (FD_ISSET(cnt.m_socket.handle(), &out))
            send(cnt);
        break;
    case Checking:
        if (FD_ISSET(cnt.m_socket.handle(), &in))
            check(cnt);
        break;
    default:
        break;
    }
}

} // !irccd
