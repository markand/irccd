/*
 * cmd-server-part.cpp -- implementation of server-part transport command
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

#include "cmd-server-part.hpp"
#include "irccd.hpp"
#include "server.hpp"
#include "service-server.hpp"

namespace irccd {

namespace command {

ServerPart::ServerPart()
    : Command("server-part", "Server", "Leave a channel")
{
}

std::vector<Command::Arg> ServerPart::args() const
{
    return {
        { "server",     true    },
        { "channel",    true    },
        { "reason",     false    }
    };
}

std::vector<Command::Property> ServerPart::properties() const
{
    return {
        { "server",     { nlohmann::json::value_t::string }},
        { "channel",    { nlohmann::json::value_t::string }}
    };
}

nlohmann::json ServerPart::request(Irccdctl &, const CommandRequest &args) const
{
    auto req = nlohmann::json::object({
        { "server",     args.arg(0) },
        { "channel",    args.arg(1) }
    });

    if (args.length() == 3)
        req.push_back({"reason", args.arg(2)});

    return req;
}

nlohmann::json ServerPart::exec(Irccd &irccd, const nlohmann::json &request) const
{
    Command::exec(irccd, request);

    irccd.serverService().require(request["server"])->part(
        request["channel"],
        request.count("reason") > 0 ? request["reason"] : ""
    );

    return nlohmann::json::object();
}

} // !command

} // !irccd
