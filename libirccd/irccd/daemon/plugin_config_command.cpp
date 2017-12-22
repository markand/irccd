/*
 * plugin_config_command.cpp -- implementation of plugin-config transport command
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

#include "irccd.hpp"
#include "plugin_config_command.hpp"
#include "plugin_service.hpp"
#include "transport_client.hpp"

namespace irccd {

namespace {

void exec_set(transport_client& client, plugin& plugin, const nlohmann::json& args)
{
    assert(args.count("value") > 0);

    auto var = args.find("variable");
    auto value = args.find("value");

    if (var == args.end() || !var->is_string())
        throw irccd_error(irccd_error::error::incomplete_message);
    if (value == args.end() || !value->is_string())
        throw irccd_error(irccd_error::error::incomplete_message);

    auto config = plugin.config();

    config[*var] = *value;
    plugin.set_config(config);
    client.success("plugin-config");
}

void exec_get(transport_client& client, plugin& plugin, const nlohmann::json& args)
{
    auto variables = nlohmann::json::object();
    auto var = args.find("variable");

    if (var != args.end() && var->is_string())
        variables[var->get<std::string>()] = plugin.config()[*var];
    else
        for (const auto& pair : plugin.config())
            variables[pair.first] = pair.second;

    /*
     * Don't put all variables into the response, put them into a sub property
     * 'variables' instead.
     *
     * It's easier for the client to iterate over all.
     */
    client.send({
        { "command",    "plugin-config" },
        { "variables",  variables       }
    });
}

} // !namespace

std::string plugin_config_command::get_name() const noexcept
{
    return "plugin-config";
}

void plugin_config_command::exec(irccd& irccd, transport_client& client, const nlohmann::json& args)
{
    auto plugin = irccd.plugins().require(json_util::require_identifier(args, "plugin"));

    if (args.count("value") > 0)
        exec_set(client, *plugin, args);
    else
        exec_get(client, *plugin, args);
}

} // !irccd
