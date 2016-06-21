/*
 * cmd-plugin-config.cpp -- implementation of plugin-config command
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

#include <iomanip>
#include <iostream>

#include "irccd.hpp"
#include "cmd-plugin-config.hpp"
#include "service-plugin.hpp"

namespace irccd {

namespace command {

namespace {

json::Value execSet(Irccd &irccd, const json::Value &request)
{
    auto plugin = irccd.pluginService().require(request.at("plugin").toString());
    auto config = plugin->config();

    config[request.at("variable").toString()] = request.at("value").toString(true);
    plugin->setConfig(config);

    return nullptr;
}

json::Value execGet(Irccd &irccd, const json::Value &request)
{
    auto config = irccd.pluginService().require(request.at("plugin").toString())->config();
    auto var = request.find("variable");

    // 'vars' property.
    std::map<std::string, json::Value> vars;

    if (var != request.end())
        vars.emplace(var->toString(), config[var->toString()]);
    else
        for (const auto &pair : config)
            vars.emplace(pair.first, pair.second);

    return json::object({{ "vars", json::Value(vars) }});
}

} // !namespace

PluginConfig::PluginConfig()
    : Command("plugin-config", "Plugins")
{
}

std::string PluginConfig::help() const
{
    return "Get or set a plugin configuration option.";
}

std::vector<Command::Arg> PluginConfig::args() const
{
    return {
        { "plugin",     true    },
        { "variable",   false   },
        { "value",      false   }
    };
}

std::vector<Command::Property> PluginConfig::properties() const
{
    return {{ "plugin", { json::Type::String }}};
}

json::Value PluginConfig::request(Irccdctl &, const CommandRequest &args) const
{
    auto object = json::object({
        { "plugin", args.arg(0) }
    });

    if (args.length() >= 2U) {
        object.insert("variable", args.arg(1));

        if (args.length() == 3U)
            object.insert("value", args.arg(2));
    }

    return object;
}

json::Value PluginConfig::exec(Irccd &irccd, const json::Value &request) const
{
    Command::exec(irccd, request);

    return request.contains("value") ? execSet(irccd, request) : execGet(irccd, request);
}

void PluginConfig::result(Irccdctl &irccdctl, const json::Value &response) const
{
    Command::result(irccdctl, response);

    auto it = response.find("vars");

    if (it == response.end() || !it->isObject())
        return;

    if (it->size() > 1U)
        for (auto v = it->begin(); v != it->end(); ++v)
            std::cout << std::setw(16) << std::left << v.key() << " : " << v->toString(true) << std::endl;
    else
        std::cout << it->begin()->toString(true) << std::endl;
}

} // !command

} // !irccd
