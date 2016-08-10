/*
 * conn-state-connecting.cpp -- connection is in progress
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

#include "conn-state-checking.hpp"
#include "conn-state-connecting.hpp"
#include "conn-state-disconnected.hpp"

namespace irccd {

Connection::Status Connection::ConnectingState::status() const noexcept
{
    return Connecting;
}

void Connection::ConnectingState::prepare(Connection &cnx, fd_set &, fd_set &out)
{
    FD_SET(cnx.m_socket.handle(), &out);
}

void Connection::ConnectingState::sync(Connection &cnx, fd_set &, fd_set &out)
{
    if (!FD_ISSET(cnx.m_socket.handle(), &out))
        return;

    try {
        auto errc = cnx.m_socket.get<int>(SOL_SOCKET, SO_ERROR);

        if (errc != 0) {
            cnx.m_stateNext = std::make_unique<DisconnectedState>();
            cnx.onDisconnect(net::error(errc));
        } else
            cnx.m_stateNext = std::make_unique<CheckingState>();
    } catch (const std::exception &ex) {
        cnx.m_stateNext = std::make_unique<DisconnectedState>();
        cnx.onDisconnect(ex.what());
    }
}

} // !irccd
