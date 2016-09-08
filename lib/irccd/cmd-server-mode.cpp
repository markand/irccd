/*
 * cmd-server-mode.cpp -- implementation of server-mode transport command
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

#include "cmd-server-mode.hpp"
#include "irccd.hpp"
#include "server.hpp"
#include "service-server.hpp"

namespace irccd {

namespace command {

ServerModeCommand::ServerModeCommand()
    : Command("server-mode", "Server", "Change your mode")
{
}

std::vector<Command::Arg> ServerModeCommand::args() const
{
    return {
        { "server",     true },
        { "mode",       true }
    };
}

std::vector<Command::Property> ServerModeCommand::properties() const
{
    return {
        { "server",     { nlohmann::json::value_t::string }},
        { "mode",       { nlohmann::json::value_t::string }}
    };
}

nlohmann::json ServerModeCommand::request(Irccdctl &, const CommandRequest &args) const
{
    return nlohmann::json::object({
        { "server",     args.arg(0) },
        { "mode",       args.arg(1) }
    });
}

nlohmann::json ServerModeCommand::exec(Irccd &irccd, const nlohmann::json &request) const
{
    Command::exec(irccd, request);

    irccd.servers().require(request["server"])->mode(request["mode"]);

    return nlohmann::json::object();
}

} // !command

} // !irccd
