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

nlohmann::json execSet(Irccd &irccd, const nlohmann::json &request, const std::string &var, const std::string &value)
{
    auto plugin = irccd.plugins().require(request["plugin"].get<std::string>());
    auto config = plugin->config();

    config[var] = value;
    plugin->setConfig(config);

    return nullptr;
}

nlohmann::json execGet(Irccd &irccd, const nlohmann::json &request, const nlohmann::json::const_iterator &var)
{
    auto config = irccd.plugins().require(request["plugin"].get<std::string>())->config();

    // 'vars' property.
    std::map<std::string, nlohmann::json> vars;

    if (var != request.end())
        vars.emplace(var->get<std::string>(), config[var->get<std::string>()]);
    else
        for (const auto &pair : config)
            vars.emplace(pair.first, pair.second);

    return nlohmann::json::object({{ "variables", nlohmann::json(vars) }});
}

} // !namespace

PluginConfig::PluginConfig()
    : Command("plugin-config", "Plugins", "Get or set a plugin config variable")
{
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
    return {{ "plugin", { nlohmann::json::value_t::string }}};
}

nlohmann::json PluginConfig::request(Irccdctl &, const CommandRequest &args) const
{
    auto object = nlohmann::json::object({
        { "plugin", args.arg(0) }
    });

    if (args.length() >= 2U) {
        object.push_back({"variable", args.arg(1)});

        if (args.length() == 3U)
            object.push_back({"value", args.arg(2)});
    }

    return object;
}

nlohmann::json PluginConfig::exec(Irccd &irccd, const nlohmann::json &request) const
{
    Command::exec(irccd, request);

    auto var = request.find("variable");

    if (var != request.end() && var->is_string())
        throw InvalidPropertyError("variable", nlohmann::json::value_t::string, var->type());

    auto value = request.find("value");

    if (value != request.end())
        return execSet(irccd, request, var->dump(), value->dump());

    return execGet(irccd, request, var);
}

void PluginConfig::result(Irccdctl &irccdctl, const nlohmann::json &response) const
{
    Command::result(irccdctl, response);

    auto it = response.find("variables");

    if (it == response.end() || !it->is_object())
        return;

    if (it->size() > 1U)
        for (auto v = it->begin(); v != it->end(); ++v)
            std::cout << std::setw(16) << std::left << v.key() << " : " << v->dump() << std::endl;
    else
        std::cout << it->begin()->dump() << std::endl;
}

} // !command

} // !irccd
