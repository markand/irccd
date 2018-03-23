/*
 * server_mode_command.cpp -- implementation of server-mode transport command
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

#include "server_mode_command.hpp"

namespace irccd {

std::string server_mode_command::get_name() const noexcept
{
    return "server-mode";
}

void server_mode_command::exec(irccd& irccd, transport_client& client, const nlohmann::json& args)
{
    const auto id = json_util::get_string(args, "server");
    const auto channel = json_util::get_string(args, "channel");
    const auto mode = json_util::get_string(args, "mode");
    const auto limit = json_util::optional_string(args, "limit", "");
    const auto user = json_util::optional_string(args, "user", "");
    const auto mask = json_util::optional_string(args, "mask", "");

    if (!id || !string_util::is_identifier(*id))
        throw server_error(server_error::invalid_identifier);
    if (!channel || channel->empty())
        throw server_error(server_error::invalid_channel);
    if (!mode || mode->empty())
        throw server_error(server_error::invalid_mode);
    if (!limit || !user || !mask)
        throw server_error(server_error::invalid_mode);

    irccd.servers().require(*id)->mode(*channel, *mode, *limit, *user, *mask);
    client.success("server-mode");
}

} // !irccd
