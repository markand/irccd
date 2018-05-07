/*
 * server_me_command.cpp -- implementation of server-me transport command
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
#include <irccd/string_util.hpp>

#include <irccd/daemon/irccd.hpp>
#include <irccd/daemon/transport_client.hpp>

#include <irccd/daemon/service/server_service.hpp>

#include "server_me_command.hpp"

namespace irccd {

std::string server_me_command::get_name() const noexcept
{
    return "server-me";
}

void server_me_command::exec(irccd& irccd, transport_client& client, const document& args)
{
    const auto id = args.get<std::string>("server");
    const auto channel = args.get<std::string>("target");
    const auto message = args.optional<std::string>("message", "");

    if (!id || !string_util::is_identifier(*id))
        throw server_error(server_error::invalid_identifier);
    if (!channel || channel->empty())
        throw server_error(server_error::invalid_channel);
    if (!message)
        throw server_error(server_error::invalid_message);

    irccd.servers().require(*id)->me(*channel, *message);
    client.success("server-me");
}

} // !irccd
