/*
 * cmd-server-connect.cpp -- implementation of server-connect transport command
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

#include <limits>

#include <format.h>

#include "cmd-server-connect.hpp"
#include "irccd.hpp"
#include "server.hpp"
#include "service-server.hpp"
#include "util.hpp"

using namespace fmt::literals;

using json = nlohmann::json;

namespace irccd {

namespace command {

ServerConnectCommand::ServerConnectCommand()
    : Command("server-connect", "Server", "Connect to a server")
{
}

std::vector<Command::Option> ServerConnectCommand::options() const
{
    return {
        { "command",    "c", "command",     "char",     "command character to use"  },
        { "nickname",   "n", "nickname",    "nickname", "nickname to use"           },
        { "realname",   "r", "realname",    "realname", "realname to use"           },
        { "sslverify",  "S", "ssl-verify",  "",         "verify SSL"                },
        { "ssl",        "s", "ssl",         "",         "connect with SSL"          },
        { "username",   "u", "username",    "",         "username to use"           }
    };
}

std::vector<Command::Arg> ServerConnectCommand::args() const
{
    return {
        { "id",     true    },
        { "host",   true    },
        { "port",   false   }
    };
}

std::vector<Command::Property> ServerConnectCommand::properties() const
{
    return {
        { "name",   { json::value_t::string }},
        { "host",   { json::value_t::string }}
    };
}

json ServerConnectCommand::exec(Irccd &irccd, const json &request) const
{
    auto server = Server::fromJson(request);

    if (irccd.servers().has(server->name()))
        throw std::invalid_argument("server '{}' already exists"_format(server->name()));

    irccd.servers().add(std::move(server));

    return Command::exec(irccd, request);
}

} // !command

} // !irccd
