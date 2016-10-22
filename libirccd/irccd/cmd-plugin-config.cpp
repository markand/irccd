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

#include "irccd.hpp"
#include "cmd-plugin-config.hpp"
#include "service-plugin.hpp"
#include "transport.hpp"
#include "util.hpp"

namespace irccd {

namespace command {

namespace {

void execSet(Irccd &, TransportClient &client, Plugin &plugin, const nlohmann::json &args)
{
    assert(args.count("value") > 0);

    auto var = args.find("variable");
    auto value = args.find("value");

    if (var == args.end() || !var->is_string())
        client.error("plugin-config", "missing 'variable' property (string expected)");
    else if (!value->is_string())
        client.error("plugin-config", "invalid 'value' property (string expected)");
    else {
        auto config = plugin.config();

        config[*var] = *value;
        plugin.setConfig(config);
        client.success("plugin-config");
    }
}

void execGet(Irccd &, TransportClient &client, Plugin &plugin, const nlohmann::json &args)
{
    auto variables = nlohmann::json::object();
    auto var = args.find("variable");

    if (var != args.end() && var->is_string())
        variables[var->get<std::string>()] = plugin.config()[*var];
    else
        for (const auto &pair : plugin.config())
            variables[pair.first] = pair.second;

    /*
     * Don't put all variables into the response, put them into a sub property
     * 'variables' instead.
     *
     * It's easier for the client to iterate over all.
     */
    client.success("plugin-config", {
        { "variables", variables }
    });
}

} // !namespace

PluginConfigCommand::PluginConfigCommand()
    : Command("plugin-config")
{
}

void PluginConfigCommand::exec(Irccd &irccd, TransportClient &client, const nlohmann::json &args)
{
    auto plugin = irccd.plugins().require(util::json::requireIdentifier(args, "plugin"));

    if (args.count("value") > 0)
        execSet(irccd, client, *plugin, args);
    else
        execGet(irccd, client, *plugin, args);
}

} // !command

} // !irccd
