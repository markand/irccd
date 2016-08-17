/*
 * cmd-server-cmode.cpp -- implementation of server-cmode transport command
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

#include "cmd-server-cmode.hpp"
#include "irccd.hpp"
#include "server.hpp"
#include "service-server.hpp"

namespace irccd {

namespace command {

ServerChannelMode::ServerChannelMode()
    : Command("server-cmode", "Server", "Change a channel mode")
{
}

std::vector<Command::Arg> ServerChannelMode::args() const
{
    return {
        { "server",     true },
        { "channel",    true },
        { "mode",       true }
    };
}

std::vector<Command::Property> ServerChannelMode::properties() const
{
    return {
        { "server",     { nlohmann::json::value_t::string }},
        { "channel",    { nlohmann::json::value_t::string }},
        { "mode",       { nlohmann::json::value_t::string }}
    };
}

nlohmann::json ServerChannelMode::exec(Irccd &irccd, const nlohmann::json &request) const
{
    Command::exec(irccd, request);

    irccd.servers().require(request["server"].get<std::string>())->cmode(
        request["channel"].get<std::string>(),
        request["mode"].get<std::string>()
    );

    return nlohmann::json::object();
}

} // !command

} // !irccd

