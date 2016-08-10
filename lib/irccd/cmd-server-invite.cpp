/*
 * cmd-server-invite.cpp -- implementation of server-invite transport command
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

#include "cmd-server-invite.hpp"
#include "irccd.hpp"
#include "server.hpp"
#include "service-server.hpp"

namespace irccd {

namespace command {

ServerInvite::ServerInvite()
    : Command("server-invite", "Server", "Invite someone into a channel")
{
}

std::vector<Command::Arg> ServerInvite::args() const
{
    return {
        { "server",     true },
        { "nickname",   true },
        { "channel",    true }
    };
}

std::vector<Command::Property> ServerInvite::properties() const
{
    return {
        { "server",     { nlohmann::json::value_t::string }},
        { "target",     { nlohmann::json::value_t::string }},
        { "channel",    { nlohmann::json::value_t::string }}
    };
}

nlohmann::json ServerInvite::request(Irccdctl &, const CommandRequest &args) const
{
    return nlohmann::json::object({
        { "server",     args.args()[0] },
        { "target",     args.args()[1] },
        { "channel",    args.args()[2] }
    });
}

nlohmann::json ServerInvite::exec(Irccd &irccd, const nlohmann::json &request) const
{
    Command::exec(irccd, request);

    irccd.serverService().require(request["server"])->invite(request["target"], request["channel"]);

    return nlohmann::json::object();
}

} // !command

} // !irccd

