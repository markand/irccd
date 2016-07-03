/*
 * cmd-server-join.cpp -- implementation of server-join transport command
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

#include "cmd-server-join.hpp"
#include "irccd.hpp"
#include "server.hpp"
#include "service-server.hpp"

namespace irccd {

namespace command {

ServerJoin::ServerJoin()
    : Command("server-join", "Server")
{
}

std::string ServerJoin::help() const
{
    return "";
}

std::vector<Command::Arg> ServerJoin::args() const
{
    return {
        { "server",     true    },
        { "channel",    true    },
        { "password",   false   }
    };
}

std::vector<Command::Property> ServerJoin::properties() const
{
    return {
        { "server",     { nlohmann::json::value_t::string }},
        { "channel",    { nlohmann::json::value_t::string }},
        { "password",   { nlohmann::json::value_t::string }}
    };
}

nlohmann::json ServerJoin::request(Irccdctl &, const CommandRequest &args) const
{
    auto req = nlohmann::json::object({
        { "server",     args.args()[0] },
        { "channel",    args.args()[1] }
    });

    if (args.length() == 3)
        req.push_back(nlohmann::json::object({"password", args.args()[2]}));

    return req;
}

nlohmann::json ServerJoin::exec(Irccd &irccd, const nlohmann::json &request) const
{
    Command::exec(irccd, request);

    auto password = request["password"];

    irccd.serverService().require(
        request.at("server").get<std::string>())->join(
        request.at("channel").get<std::string>(),
        password.is_string() ? password.get<std::string>() : ""
    );

    return nlohmann::json::object();
}

} // !command

} // !irccd
