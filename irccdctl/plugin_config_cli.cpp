/*
 * plugin_config_cli.cpp -- implementation of irccdctl plugin-config
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

#include "plugin_config_cli.hpp"

namespace irccd {

namespace ctl {

void plugin_config_cli::set(ctl::controller& ctl, const std::vector<std::string>&args)
{
    request(ctl, {
        { "plugin", args[0] },
        { "variable", args[1] },
        { "value", args[2] }
    });
}

void plugin_config_cli::get(ctl::controller& ctl, const std::vector<std::string>& args)
{
    auto json = nlohmann::json::object({
        { "plugin", args[0] },
        { "variable", args[1] }
    });

    request(ctl, std::move(json), [args] (auto result) {
        if (result["variables"].is_object())
            std::cout << json_util::pretty(result["variables"][args[1]]) << std::endl;
    });
}

void plugin_config_cli::getall(ctl::controller& ctl, const std::vector<std::string> &args)
{
    request(ctl, {{ "plugin", args[0] }}, [] (auto result) {
        auto variables = result["variables"];

        for (auto v = variables.begin(); v != variables.end(); ++v)
            std::cout << std::setw(16) << std::left << v.key() << " : " << json_util::pretty(v.value()) << std::endl;
    });
}

std::string plugin_config_cli::name() const
{
    return "plugin-config";
}

void plugin_config_cli::exec(ctl::controller& ctl, const std::vector<std::string> &args)
{
    switch (args.size()) {
    case 3:
        set(ctl, args);
        break;
    case 2:
        get(ctl, args);
        break;
    case 1:
        getall(ctl, args);
        break;
    default:
        throw std::invalid_argument("plugin-config requires at least 1 argument");
    }
}

} // !ctl

} // !irccd
