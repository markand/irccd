/*
 * cmd-plugin-unload.cpp -- implementation of plugin-unload transport command
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

#include "cmd-plugin-unload.hpp"
#include "irccd.hpp"
#include "service-plugin.hpp"
#include "util.hpp"

namespace irccd {

namespace command {

PluginUnloadCommand::PluginUnloadCommand()
    : Command("plugin-unload", "Plugins", "Unload a plugin")
{
}

std::vector<Command::Arg> PluginUnloadCommand::args() const
{
    return {{ "plugin", true }};
}

std::vector<Command::Property> PluginUnloadCommand::properties() const
{
    return {{ "plugin", { nlohmann::json::value_t::string }}};
}

nlohmann::json PluginUnloadCommand::request(Irccdctl &, const CommandRequest &args) const
{
    return nlohmann::json::object({{ "plugin", args.arg(0) }});
}

nlohmann::json PluginUnloadCommand::exec(Irccd &irccd, const nlohmann::json &request) const
{
    Command::exec(irccd, request);

    irccd.plugins().unload(request["plugin"].get<std::string>());

    return nlohmann::json::object();
}

} // !command

} // !irccd
