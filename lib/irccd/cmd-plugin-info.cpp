/*
 * cmd-plugin-info.cpp -- implementation of plugin-info command
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

#include "cmd-plugin-info.hpp"
#include "irccd.hpp"
#include "plugin.hpp"
#include "service-plugin.hpp"
#include "util.hpp"

namespace irccd {

namespace command {

PluginInfo::PluginInfo()
    : Command("plugin-info", "Plugins", "Get plugin information")
{
}

std::vector<Command::Arg> PluginInfo::args() const
{
    return {{ "plugin", true }};
}

std::vector<Command::Property> PluginInfo::properties() const
{
    return {{ "plugin", { nlohmann::json::value_t::string }}};
}

nlohmann::json PluginInfo::request(Irccdctl &, const CommandRequest &args) const
{
    return nlohmann::json::object({{ "plugin", args.arg(0) }});
}

nlohmann::json PluginInfo::exec(Irccd &irccd, const nlohmann::json &request) const
{
    Command::exec(irccd, request);

    auto plugin = irccd.plugins().require(request.at("plugin").get<std::string>());

    return nlohmann::json::object({
        { "author",     plugin->author()    },
        { "license",    plugin->license()   },
        { "summary",    plugin->summary()   },
        { "version",    plugin->version()   }
    });
}

void PluginInfo::result(Irccdctl &irccdctl, const nlohmann::json &result) const
{
    Command::result(irccdctl, result);

    auto it = result.find("status");

    if (!it->is_boolean() || !*it)
        return;

    auto get = [&] (auto key) -> std::string {
        auto v = result.find(key);

        if (v == result.end() || !v->is_primitive())
            return "";

        return v->dump();
    };

    std::cout << std::boolalpha;
    std::cout << "Author         : " << get("author") << std::endl;
    std::cout << "License        : " << get("license") << std::endl;
    std::cout << "Summary        : " << get("summary") << std::endl;
    std::cout << "Version        : " << get("version") << std::endl;
}

} // !command

} // !irccd

