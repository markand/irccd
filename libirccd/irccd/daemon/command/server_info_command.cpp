/*
 * server_info_command.cpp -- implementation of server-info transport command
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
#include <irccd/daemon/server_util.hpp>
#include <irccd/daemon/transport_client.hpp>

#include <irccd/daemon/service/server_service.hpp>

#include "server_info_command.hpp"

namespace irccd {

std::string server_info_command::get_name() const noexcept
{
    return "server-info";
}

void server_info_command::exec(irccd& irccd, transport_client& client, const nlohmann::json& args)
{
    const auto id = json_util::parser(args).get<std::string>("server");

    if (!id || !string_util::is_identifier(*id))
        throw server_error(server_error::invalid_identifier);

    const auto server = irccd.servers().require(*id);

    // Construct the JSON response.
    auto response = nlohmann::json::object();

    // General stuff.
    response.push_back({"command", "server-info"});
    response.push_back({"name", server->get_name()});
    response.push_back({"host", server->get_host()});
    response.push_back({"port", server->get_port()});
    response.push_back({"nickname", server->get_nickname()});
    response.push_back({"username", server->get_username()});
    response.push_back({"realname", server->get_realname()});
    response.push_back({"channels", server->get_channels()});

    // Optional stuff.
    if (server->get_flags() & server::ipv6)
        response.push_back({"ipv6", true});
    if (server->get_flags() & server::ssl)
        response.push_back({"ssl", true});
    if (server->get_flags() & server::ssl_verify)
        response.push_back({"sslVerify", true});

    client.send(response);
}

} // !irccd
