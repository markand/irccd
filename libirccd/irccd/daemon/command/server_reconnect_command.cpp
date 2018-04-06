/*
 * server_reconnect_command.cpp -- implementation of server-reconnect transport command
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

#include "server_reconnect_command.hpp"

namespace irccd {

std::string server_reconnect_command::get_name() const noexcept
{
    return "server-reconnect";
}

void server_reconnect_command::exec(irccd& irccd, transport_client& client, const nlohmann::json& args)
{
    const auto it = args.find("server");

    if (it == args.end()) {
        for (const auto& server : irccd.servers().servers())
            server->reconnect();
    } else {
        if (!it->is_string() || !string_util::is_identifier(it->get<std::string>()))
            throw server_error(server_error::invalid_identifier);

        irccd.servers().require(it->get<std::string>())->reconnect();
    }

    client.success("server-reconnect");
}

} // !irccd