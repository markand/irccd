/*
 * cmd-server-disconnect.cpp -- implementation of server-disconnect transport command
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

#include "cmd-server-disconnect.hpp"
#include "irccd.hpp"
#include "server.hpp"
#include "service-server.hpp"

namespace irccd {

namespace command {

ServerDisconnect::ServerDisconnect()
    : Command("server-disconnect", "Server", "Disconnect one or more servers")
{
}

std::vector<Command::Arg> ServerDisconnect::args() const
{
    return {{ "server", false }};
}

nlohmann::json ServerDisconnect::exec(Irccd &irccd, const nlohmann::json &request) const
{
    auto it = request.find("server");

    if (it == request.end())
        irccd.serverService().clear();
    else
        irccd.serverService().remove(*it);

    return Command::exec(irccd, request);
}

} // !command

} // !irccd
