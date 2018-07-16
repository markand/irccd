/*
 * rule_add_cli.cpp -- implementation of irccdctl rule-add
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

#include <irccd/options.hpp>
#include <irccd/string_util.hpp>

#include "rule_add_cli.hpp"

using irccd::string_util::to_uint;

namespace irccd {

namespace ctl {

std::string rule_add_cli::name() const
{
    return "rule-add";
}

void rule_add_cli::exec(ctl::controller& ctl, const std::vector<std::string>& args)
{
    static const option::options options{
        { "-c",             true },
        { "--add-channel",  true },
        { "-e",             true },
        { "--add-event",    true },
        { "-i",             true },
        { "--index",        true },
        { "-p",             true },
        { "--add-plugin",   true },
        { "-s",             true },
        { "--add-server",   true }
    };

    auto copy = args;
    auto result = option::read(copy, options);

    if (copy.size() < 1)
        throw std::invalid_argument("rule-add requires at least 1 argument");

    auto json = nlohmann::json::object({
        { "command",    "rule-add"              },
        { "channels",   nlohmann::json::array() },
        { "events",     nlohmann::json::array() },
        { "plugins",    nlohmann::json::array() },
        { "servers",    nlohmann::json::array() }
    });

    // All sets.
    for (const auto& pair : result) {
        if (pair.first == "-c" || pair.first == "--add-channel")
            json["channels"].push_back(pair.second);
        if (pair.first == "-e" || pair.first == "--add-event")
            json["events"].push_back(pair.second);
        if (pair.first == "-p" || pair.first == "--add-plugin")
            json["plugins"].push_back(pair.second);
        if (pair.first == "-s" || pair.first == "--add-server")
            json["servers"].push_back(pair.second);
    }

    // Index.
    std::optional<unsigned> index;

    if (result.count("-i") > 0 && !(index = to_uint(result.find("-i")->second)))
        throw std::invalid_argument("invalid index argument");
    if (result.count("--index") > 0 && !(index = to_uint(result.find("--index")->second)))
        throw std::invalid_argument("invalid index argument");

    if (index)
        json["index"] = *index;

    // And action.
    if (copy[0] != "accept" && copy[0] != "drop")
        throw std::runtime_error(string_util::sprintf("invalid action '%s'", copy[0]));

    json["action"] = copy[0];

    request(ctl, json);
}

} // !ctl

} // !irccd
