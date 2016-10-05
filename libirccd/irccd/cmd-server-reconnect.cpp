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

ServerReconnectCommand::ServerReconnectCommand()
    : Command("server-reconnect", "Server", "Force reconnection of one or more servers")
{
}

std::vector<Command::Arg> ServerReconnectCommand::args() const
{
    return {{ "server", false }};
}

nlohmann::json ServerReconnectCommand::request(Irccdctl &, const CommandRequest &args) const
{
    return args.length() == 0 ? nlohmann::json::object() : nlohmann::json::object({ { "server", args.arg(0) } });
}

nlohmann::json ServerReconnectCommand::exec(Irccd &irccd, const nlohmann::json &request) const
{
    auto server = request.find("server");

    if (server != request.end() && server->is_string())
        irccd.servers().require(*server)->reconnect();
    else
        for (auto &server : irccd.servers().servers())
            server->reconnect();

    return nullptr;
}

} // !command

} // !irccd
