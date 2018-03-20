/*
 * server_invite_command.cpp -- implementation of server-invite transport command
 *
 * Copyright (c) 2013-2018 David Demelier <markand@malikania.fr>
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

#include <irccd/daemon/irccd.hpp>
#include <irccd/daemon/server_util.hpp>
#include <irccd/daemon/transport_client.hpp>

#include <irccd/daemon/service/server_service.hpp>

#include "server_invite_command.hpp"

namespace irccd {

std::string server_invite_command::get_name() const noexcept
{
    return "server-invite";
}

void server_invite_command::exec(irccd& irccd, transport_client& client, const nlohmann::json& args)
{
    const auto id = server_util::get_identifier(args);
    const auto server = irccd.servers().require(id);
    const auto target = json_util::get_string(args, "/target"_json_pointer);
    const auto channel = json_util::get_string(args, "/channel"_json_pointer);

    if (!target || target->empty())
        throw server_error(server->name(), server_error::invalid_nickname);
    if (!channel || channel->empty())
        throw server_error(server->name(), server_error::invalid_channel);

    server->invite(*target, *channel);
    client.success("server-invite");
}

} // !irccd
