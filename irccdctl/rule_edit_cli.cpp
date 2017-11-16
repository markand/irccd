/*
 * rule_edit_cli.cpp -- implementation of irccdctl rule-edit
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

#include <irccd/options.hpp>
#include <irccd/string_util.hpp>

#include "rule_edit_cli.hpp"

namespace irccd {

namespace ctl {

std::string rule_edit_cli::name() const
{
    return "rule-edit";
}

void rule_edit_cli::exec(ctl::controller& ctl, const std::vector<std::string>& args)
{
    static const option::options options{
        { "-a",                 true },
        { "--action",           true },
        { "-c",                 true },
        { "--add-channel",      true },
        { "-C",                 true },
        { "--remove-channel",   true },
        { "-e",                 true },
        { "--add-event",        true },
        { "-E",                 true },
        { "--remove-event",     true },
        { "-p",                 true },
        { "--add-plugin",       true },
        { "-P",                 true },
        { "--remove-plugin",    true },
        { "-s",                 true },
        { "--add-server",       true },
        { "-S",                 true },
        { "--remove-server",    true },
    };

    auto copy = args;
    auto result = option::read(copy, options);

    if (copy.size() < 1)
        throw std::invalid_argument("rule-edit requires at least 1 argument");

    auto json = nlohmann::json::object({
        { "command",    "rule-edit"             },
        { "channels",   nlohmann::json::array() },
        { "events",     nlohmann::json::array() },
        { "plugins",    nlohmann::json::array() },
        { "servers",    nlohmann::json::array() }
    });

    for (const auto& pair : result) {
        // Action.
        if (pair.first == "-a" || pair.first == "--action")
            json["action"] = pair.second;

        // Additions.
        if (pair.first == "-c" || pair.first == "--add-channel")
            json["add-channels"].push_back(pair.second);
        if (pair.first == "-e" || pair.first == "--add-event")
            json["add-events"].push_back(pair.second);
        if (pair.first == "-p" || pair.first == "--add-plugin")
            json["add-plugins"].push_back(pair.second);
        if (pair.first == "-s" || pair.first == "--add-server")
            json["add-servers"].push_back(pair.second);

        // Removals.
        if (pair.first == "-C" || pair.first == "--remove-channel")
            json["remove-channels"].push_back(pair.second);
        if (pair.first == "-E" || pair.first == "--remove-event")
            json["remove-events"].push_back(pair.second);
        if (pair.first == "-P" || pair.first == "--remove-plugin")
            json["remove-plugins"].push_back(pair.second);
        if (pair.first == "-S" || pair.first == "--remove-server")
            json["remove-servers"].push_back(pair.second);
    }

    // Index.
    json["index"] = string_util::to_number<unsigned>(copy[0]);

    request(ctl, json);
}

} // !ctl

} // !irccd
