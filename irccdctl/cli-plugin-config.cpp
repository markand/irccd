/*
 * cli-plugin-config.cpp -- implementation of irccdctl plugin-info
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

#include <util.hpp>

#include "cli-plugin-config.hpp"

namespace irccd {

namespace cli {

void PluginConfigCli::set(Irccdctl &irccdctl, const std::vector<std::string> &args)
{
    check(request(irccdctl, nlohmann::json::object({
        { "plugin", args[0] },
        { "variable", args[1] },
        { "value", args[2] }
    })));
}

void PluginConfigCli::get(Irccdctl &irccdctl, const std::vector<std::string> &args)
{
    auto result = request(irccdctl, nlohmann::json::object({
        { "plugin", args[0] },
        { "variable", args[1] }
    }));

    check(result);

    if (result["variables"].is_object())
        std::cout << util::json::pretty(result["variables"][args[1]]) << std::endl;
}

void PluginConfigCli::getall(Irccdctl &irccdctl, const std::vector<std::string> &args)
{
    auto result = request(irccdctl, nlohmann::json::object({{ "plugin", args[0] }}));

    check(result);

    auto variables = result["variables"];

    for (auto v = variables.begin(); v != variables.end(); ++v)
        std::cout << std::setw(16) << std::left << v.key() << " : " << util::json::pretty(v.value()) << std::endl;
}

PluginConfigCli::PluginConfigCli()
    : Cli("plugin-config",
          "configure a plugin",
          "plugin-config plugin [variable] [value]",
          "Get or set plugin configuration.\n\n"
          "Examples:\n"
          "\tirccdctl plugin-config ask")
{
}

void PluginConfigCli::exec(Irccdctl &irccdctl, const std::vector<std::string> &args)
{
    switch (args.size()) {
    case 3:
        set(irccdctl, args);
        break;
    case 2:
        get(irccdctl, args);
        break;
    case 1:
        getall(irccdctl, args);
        break;
    default:
        throw std::invalid_argument("plugin-config requires at least 1 argument");
    }
}

} // !cli

} // !irccd
