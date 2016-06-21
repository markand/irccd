/*
 * cmd-server-reconnect.cpp -- implementation of server-reconnect transport command
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

#include "cmd-server-reconnect.hpp"
#include "irccd.hpp"
#include "server.hpp"
#include "service-server.hpp"

namespace irccd {

namespace command {

ServerReconnect::ServerReconnect()
    : Command("server-reconnect", "Server")
{
}

std::string ServerReconnect::help() const
{
    return "";
}

std::vector<Command::Arg> ServerReconnect::args() const
{
    return {{ "server", false }};
}

json::Value ServerReconnect::request(Irccdctl &, const CommandRequest &args) const
{
    return args.length() == 0 ? nullptr : json::object({ { "server", args.arg(0) } });
}

json::Value ServerReconnect::exec(Irccd &irccd, const json::Value &request) const
{
    auto server = request.find("server");

    if (server != request.end() && server->isString())
        irccd.serverService().require(server->toString())->reconnect();
    else
        for (auto &server : irccd.serverService().servers())
            server->reconnect();

    return nullptr;
}

} // !command

} // !irccd
