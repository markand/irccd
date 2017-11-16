/*
 * server_info_cli.cpp -- implementation of irccdctl server-info
 *
 * Copyright (c) 2013-2017 David Demelier <markand@malikania.fr>
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

#include "server_info_cli.hpp"

namespace irccd {

namespace ctl {

std::string server_info_cli::name() const
{
    return "server-info";
}

void server_info_cli::exec(ctl::controller& ctl, const std::vector<std::string>& args)
{
    if (args.size() < 1)
        throw std::invalid_argument("server-info requires 1 argument");

    auto json = nlohmann::json::object({
        { "command",    "server-info"   },
        { "server",     args[0]         }
    });

    request(ctl, std::move(json), [] (auto result) {
        std::cout << std::boolalpha;
        std::cout << "Name           : " << json_util::pretty(result["name"]) << std::endl;
        std::cout << "Host           : " << json_util::pretty(result["host"]) << std::endl;
        std::cout << "Port           : " << json_util::pretty(result["port"]) << std::endl;
        std::cout << "Ipv6           : " << json_util::pretty(result["ipv6"]) << std::endl;
        std::cout << "SSL            : " << json_util::pretty(result["ssl"]) << std::endl;
        std::cout << "SSL verified   : " << json_util::pretty(result["sslVerify"]) << std::endl;
        std::cout << "Channels       : ";

        for (const auto& v : result["channels"])
            if (v.is_string())
                std::cout << v.template get<std::string>() << " ";

        std::cout << std::endl;

        std::cout << "Nickname       : " << json_util::pretty(result["nickname"]) << std::endl;
        std::cout << "User name      : " << json_util::pretty(result["username"]) << std::endl;
        std::cout << "Real name      : " << json_util::pretty(result["realname"]) << std::endl;
    });
}

} // !ctl

} // !irccd
