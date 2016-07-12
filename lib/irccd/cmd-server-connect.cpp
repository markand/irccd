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

namespace {

std::string readInfoName(const json &object)
{
    std::string name = object["name"];

    if (!util::isIdentifierValid(name))
        throw PropertyError("name", "invalid identifier");

    return name;
}

std::string readInfoHost(const json &object)
{
    std::string host = object["host"];

    if (host.empty())
        throw PropertyError("host", "empty hostname");

    return host;
}

std::uint16_t readInfoPort(const json &object)
{
    json::const_iterator it = object.find("port");

    if (it == object.end())
        return 6667;

    if (!it->is_number())
        throw InvalidPropertyError("port", json::value_t::number_unsigned, it->type());
    if (!util::isBound(it->get<int>(), 0, UINT16_MAX))
        throw PropertyRangeError("port", 0, UINT16_MAX, it->get<int>());

    return static_cast<std::uint16_t>(it->get<int>());
}

ServerInfo readInfo(const json &object)
{
    ServerInfo info;

    info.host = readInfoHost(object);
    info.port = readInfoPort(object);

    json::const_iterator it;

    if ((it = object.find("ssl")) != object.end() && it->is_boolean() && *it)
        info.flags |= ServerInfo::Ssl;
    if ((it = object.find("sslVerify")) != object.end() && it->is_boolean() && *it)
        info.flags |= ServerInfo::SslVerify;

    return info;
}

void readIdentity(Server &server, const json &object)
{
    json::const_iterator it;

    if ((it = object.find("nickname")) != object.end() && it->is_string())
        server.setNickname(*it);
    if ((it = object.find("realname")) != object.end() && it->is_string())
        server.setRealname(*it);
    if ((it = object.find("username")) != object.end() && it->is_string())
        server.setUsername(*it);
    if ((it = object.find("ctcpVersion")) != object.end() && it->is_string())
        server.setCtcpVersion(*it);
}

ServerSettings readSettings(const json &object)
{
    ServerSettings settings;
    json::const_iterator it;

    if ((it = object.find("commandChar")) != object.end() && it->is_string())
        settings.command = *it;
    if ((it = object.find("reconnectTries")) != object.end() && it->is_number_integer())
        settings.reconnectTries = *it;
    if ((it = object.find("reconnectTimeout")) != object.end() && it->is_number_integer())
        settings.reconnectDelay = *it;

    return settings;
}

} // !namespace

ServerConnect::ServerConnect()
    : Command("server-connect", "Server")
{
}

std::string ServerConnect::help() const
{
    return "Connect to a server.";
}

std::vector<Command::Option> ServerConnect::options() const
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

std::vector<Command::Arg> ServerConnect::args() const
{
    return {
        { "id",     true    },
        { "host",   true    },
        { "port",   false   }
    };
}

std::vector<Command::Property> ServerConnect::properties() const
{
    return {
        { "name",   { json::value_t::string }},
        { "host",   { json::value_t::string }}
    };
}

json ServerConnect::exec(Irccd &irccd, const json &request) const
{
    auto server = std::make_shared<Server>(readInfoName(request), readInfo(request), readSettings(request));

    readIdentity(*server, request);

    if (irccd.serverService().has(server->name()))
        throw std::invalid_argument("server '{}' already exists"_format(server->name()));

    irccd.serverService().add(std::move(server));

    return Command::exec(irccd, request);
}

} // !command

} // !irccd
