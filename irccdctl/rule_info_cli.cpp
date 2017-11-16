/*
 * rule_info_cli.cpp -- implementation of irccdctl rule-info
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

#include "rule_info_cli.hpp"

namespace irccd {

namespace ctl {

void rule_info_cli::print(const nlohmann::json& json, int index)
{
    assert(json.is_object());

    auto unjoin = [] (auto array) {
        std::ostringstream oss;

        for (auto it = array.begin(); it != array.end(); ++it) {
            if (!it->is_string())
                continue;

            oss << it->template get<std::string>() << " ";
        }

        return oss.str();
    };
    auto unstr = [] (auto action) {
        if (action.is_string() && action == "accept")
            return "accept";
        else
            return "drop";
    };

    std::cout << "rule:        " << index << std::endl;
    std::cout << "servers:     " << unjoin(json["servers"]) << std::endl;
    std::cout << "channels:    " << unjoin(json["channels"]) << std::endl;
    std::cout << "plugins:     " << unjoin(json["plugins"]) << std::endl;
    std::cout << "events:      " << unjoin(json["events"]) << std::endl;
    std::cout << "action:      " << unstr(json["action"]) << std::endl;
    std::cout << std::endl;
}

std::string rule_info_cli::name() const
{
    return "rule-info";
}

void rule_info_cli::exec(ctl::controller& ctl, const std::vector<std::string>& args)
{
    if (args.size() < 1)
        throw std::invalid_argument("rule-info requires 1 argument");

    int index = 0;

    try {
        index = std::stoi(args[0]);
    } catch (...) {
        throw std::invalid_argument("invalid number '" + args[0] + "'");
    }

    auto json = nlohmann::json::object({
        { "command",    "rule-info" },
        { "index",      index       }
    });

    request(ctl, std::move(json), [] (auto result) {
        print(result, 0);
    });
}

} // !ctl

} // !irccd
