/*
 * cli-server-info.cpp -- implementation of irccdctl server-info
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

#include "cli-server-info.hpp"
#include "util.hpp"

namespace irccd {

namespace cli {

ServerInfoCli::ServerInfoCli()
    : Cli("server-info",
          "get server information",
          "server-info server",
          "Get information about a server.\n\n"
          "Example:\n"
          "\tirccdctl server-info freenode")
{
}

void ServerInfoCli::exec(Irccdctl &irccdctl, const std::vector<std::string> &args)
{
    if (args.size() < 1)
        throw std::invalid_argument("server-info requires 1 argument");

    auto result = request(irccdctl, {
        { "command",    "server-info"   },
        { "server",     args[0]         }
    });

    check(result);

    std::cout << std::boolalpha;
    std::cout << "Name           : " << util::json::pretty(result["name"]) << std::endl;
    std::cout << "Host           : " << util::json::pretty(result["host"]) << std::endl;
    std::cout << "Port           : " << util::json::pretty(result["port"]) << std::endl;
    std::cout << "Ipv6           : " << util::json::pretty(result["ipv6"]) << std::endl;
    std::cout << "SSL            : " << util::json::pretty(result["ssl"]) << std::endl;
    std::cout << "SSL verified   : " << util::json::pretty(result["sslVerify"]) << std::endl;
    std::cout << "Channels       : ";

    for (const auto &v : result["channels"])
        if (v.is_string())
            std::cout << v.get<std::string>() << " ";

    std::cout << std::endl;

    std::cout << "Nickname       : " << util::json::pretty(result["nickname"]) << std::endl;
    std::cout << "User name      : " << util::json::pretty(result["username"]) << std::endl;
    std::cout << "Real name      : " << util::json::pretty(result["realname"]) << std::endl;
}

} // !cli

} // !irccd
