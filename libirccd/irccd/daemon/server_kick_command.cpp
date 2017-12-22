/*
 * server_kick_command.cpp -- implementation of server-kick transport command
 *
 * Copyright (c) 2013-2017 David Demelier <markand@malikania.fr>
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

#include <irccd/json_util.hpp>

#include "irccd.hpp"
#include "server_kick_command.hpp"
#include "server_service.hpp"
#include "transport_client.hpp"

namespace irccd {

std::string server_kick_command::get_name() const noexcept
{
    return "server-kick";
}

void server_kick_command::exec(irccd& irccd, transport_client& client, const nlohmann::json& args)
{
    auto server = irccd.servers().require(args);
    auto target = json_util::get_string(args, "target");
    auto channel = json_util::get_string(args, "channel");
    auto reason = json_util::get_string(args, "reason");

    if (target.empty())
        throw server_error(server_error::invalid_nickname, server->name());
    if (channel.empty())
        throw server_error(server_error::invalid_channel, server->name());

    server->kick(target, channel, reason);
    client.success("server-kick");
}

} // !irccd
