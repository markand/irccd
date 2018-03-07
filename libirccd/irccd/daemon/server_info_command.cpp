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

#include "irccd.hpp"
#include "server_info_command.hpp"
#include "transport_client.hpp"

#include <irccd/daemon/service/server_service.hpp>

namespace irccd {

std::string server_info_command::get_name() const noexcept
{
    return "server-info";
}

void server_info_command::exec(irccd& irccd, transport_client& client, const nlohmann::json& args)
{
    auto response = nlohmann::json::object();
    auto server = irccd.servers().require(args);

    // General stuff.
    response.push_back({"command", "server-info"});
    response.push_back({"name", server->name()});
    response.push_back({"host", server->host()});
    response.push_back({"port", server->port()});
    response.push_back({"nickname", server->nickname()});
    response.push_back({"username", server->username()});
    response.push_back({"realname", server->realname()});
    response.push_back({"channels", server->channels()});

    // Optional stuff.
    if (server->flags() & server::ipv6)
        response.push_back({"ipv6", true});
    if (server->flags() & server::ssl)
        response.push_back({"ssl", true});
    if (server->flags() & server::ssl_verify)
        response.push_back({"sslVerify", true});

    client.send(response);
}

} // !irccd
