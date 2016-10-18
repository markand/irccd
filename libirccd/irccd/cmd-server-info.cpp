/*
 * cmd-server-info.cpp -- implementation of server-info transport command
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

#include <iostream>

#include "cmd-server-info.hpp"
#include "irccd.hpp"
#include "server.hpp"
#include "service-server.hpp"
#include "transport.hpp"
#include "util.hpp"

namespace irccd {

namespace command {

ServerInfoCommand::ServerInfoCommand()
    : Command("server-info")
{
}

void ServerInfoCommand::exec(Irccd &irccd, TransportClient &client, const nlohmann::json &args)
{
    auto response = nlohmann::json::object();
    auto server = irccd.servers().require(util::json::requireIdentifier(args, "server"));

    // General stuff.
    response.push_back({"name", server->name()});
    response.push_back({"host", server->host()});
    response.push_back({"port", server->port()});
    response.push_back({"nickname", server->nickname()});
    response.push_back({"username", server->username()});
    response.push_back({"realname", server->realname()});
    response.push_back({"channels", server->channels()});

    // Optional stuff.
    if (server->flags() & Server::Ipv6)
        response.push_back({"ipv6", true});
    if (server->flags() & Server::Ssl)
        response.push_back({"ssl", true});
    if (server->flags() & Server::SslVerify)
        response.push_back({"sslVerify", true});

    client.success("server-info", response);
}

} // !command

} // !irccd
