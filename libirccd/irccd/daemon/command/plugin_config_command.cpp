/*
 * plugin_config_command.cpp -- implementation of plugin-config transport command
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

#include <irccd/json_util.hpp>
#include <irccd/string_util.hpp>

#include <irccd/daemon/irccd.hpp>
#include <irccd/daemon/transport_client.hpp>

#include <irccd/daemon/service/plugin_service.hpp>

#include "plugin_config_command.hpp"

namespace irccd {

namespace {

void exec_set(transport_client& client, plugin& plugin, const nlohmann::json& args)
{
    assert(args.count("value") > 0);

    const auto var = args.find("variable");
    const auto value = args.find("value");

    if (var == args.end() || !var->is_string())
        throw irccd_error(irccd_error::error::incomplete_message);
    if (value == args.end() || !value->is_string())
        throw irccd_error(irccd_error::error::incomplete_message);

    auto config = plugin.get_options();

    config[*var] = *value;
    plugin.set_options(config);
    client.success("plugin-config");
}

void exec_get(transport_client& client, plugin& plugin, const nlohmann::json& args)
{
    auto variables = nlohmann::json::object();
    auto var = args.find("variable");

    if (var != args.end() && var->is_string())
        variables[var->get<std::string>()] = plugin.get_options()[*var];
    else
        for (const auto& pair : plugin.get_options())
            variables[pair.first] = pair.second;

    /*
     * Don't put all variables into the response, put them into a sub property
     * 'variables' instead.
     *
     * It's easier for the client to iterate over all.
     */
    client.write({
        { "command",    "plugin-config" },
        { "variables",  variables       }
    });
}

} // !namespace

std::string plugin_config_command::get_name() const noexcept
{
    return "plugin-config";
}

void plugin_config_command::exec(irccd& irccd, transport_client& client, const document& args)
{
    const auto id = args.get<std::string>("plugin");

    if (!id || !string_util::is_identifier(*id))
        throw plugin_error(plugin_error::invalid_identifier);

    const auto plugin = irccd.plugins().require(*id);

    if (args.count("value") > 0)
        exec_set(client, *plugin, args);
    else
        exec_get(client, *plugin, args);
}

} // !irccd
