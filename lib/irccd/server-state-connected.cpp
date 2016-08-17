/*
 * server-state-connected.cpp -- connected state
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

#include <format.h>

#include "logger.hpp"
#include "server-state-connected.hpp"
#include "server-state-disconnected.hpp"
#include "server-private.hpp"

using namespace fmt::literals;

namespace irccd {

void Server::ConnectedState::prepare(Server &server, fd_set &setinput, fd_set &setoutput, net::Handle &maxfd)
{
    if (!irc_is_connected(*server.m_session)) {
        log::warning() << "server " << server.m_name << ": disconnected" << std::endl;

        if (server.m_reconnectDelay > 0)
            log::warning("server {}: retrying in {} seconds"_format(server.m_name, server.m_reconnectDelay));

        server.next(std::make_unique<DisconnectedState>());
    } else if (server.m_timer.elapsed() >= server.m_pingTimeout * 1000) {
        log::warning() << "server " << server.m_name << ": ping timeout after "
                   << (server.m_timer.elapsed() / 1000) << " seconds" << std::endl;
        server.next(std::make_unique<DisconnectedState>());
    } else
        irc_add_select_descriptors(*server.m_session, &setinput, &setoutput, reinterpret_cast<int *>(&maxfd));
}

std::string Server::ConnectedState::ident() const
{
    return "Connected";
}

} // !irccd
