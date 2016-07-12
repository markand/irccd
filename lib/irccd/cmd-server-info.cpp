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

namespace irccd {

namespace command {

ServerInfo::ServerInfo()
    : Command("server-info", "Server")
{
}

std::string ServerInfo::help() const
{
    return "";
}

std::vector<Command::Arg> ServerInfo::args() const
{
    return {{ "server", true }};
}

std::vector<Command::Property> ServerInfo::properties() const
{
    return {{ "server", { nlohmann::json::value_t::string }}};
}

nlohmann::json ServerInfo::request(Irccdctl &, const CommandRequest &args) const
{
    return nlohmann::json::object({
        { "server",     args.args()[0] },
        { "target",     args.args()[1] },
        { "channel",    args.args()[2] }
    });
}

nlohmann::json ServerInfo::exec(Irccd &irccd, const nlohmann::json &request) const
{
    auto response = Command::exec(irccd, request);
    auto server = irccd.serverService().require(request["server"]);

    // General stuff.
    response.push_back({"name", server->name()});
    response.push_back({"host", server->info().host});
    response.push_back({"port", server->info().port});
    response.push_back({"nickname", server->nickname()});
    response.push_back({"username", server->username()});
    response.push_back({"realname", server->realname()});

    // Optional stuff.
    if (server->info().flags & irccd::ServerInfo::Ipv6)
        response.push_back({"ipv6", true});
    if (server->info().flags & irccd::ServerInfo::Ssl)
        response.push_back({"ssl", true});
    if (server->info().flags & irccd::ServerInfo::SslVerify)
        response.push_back({"sslVerify", true});

    // Channel list.
    auto channels = nlohmann::json::array();

    for (const auto &c : server->settings().channels)
        channels.push_back(c.name);

    response.push_back({"channels", std::move(channels)});

    return response;
}

void ServerInfo::result(Irccdctl &irccdctl, const nlohmann::json &response) const
{
    Command::result(irccdctl, response);

    auto get = [&] (auto key) -> std::string {
        auto v = response.find(key);

        if (v == response.end() || !v->is_primitive())
            return "";

        return v->dump();
    };

    // Server information.
    std::cout << std::boolalpha;
    std::cout << "Name           : " << get("name") << std::endl;
    std::cout << "Host           : " << get("host") << std::endl;
    std::cout << "Port           : " << get("port") << std::endl;
    std::cout << "Ipv6           : " << get("ipv6") << std::endl;
    std::cout << "SSL            : " << get("ssl") << std::endl;
    std::cout << "SSL verified   : " << get("sslVerify") << std::endl;

    // Channels.
    std::cout << "Channels       : ";

    if (response.count("channels") != 0)
        for (const auto &v : response["channels"])
            std::cout << v.dump() << " ";

    std::cout << std::endl;

    // Identity.
    std::cout << "Nickname       : " << get("nickname") << std::endl;
    std::cout << "User name      : " << get("username") << std::endl;
    std::cout << "Real name      : " << get("realname") << std::endl;
}

} // !command

} // !irccd
