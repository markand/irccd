/*
 * cmd-plugin-reload.cpp -- implementation of plugin-reload transport command
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

#include "cmd-plugin-reload.hpp"
#include "irccd.hpp"
#include "plugin.hpp"
#include "service-plugin.hpp"
#include "transport.hpp"
#include "util.hpp"

namespace irccd {

namespace command {

PluginReloadCommand::PluginReloadCommand()
    : Command("plugin-reload")
{
}

void PluginReloadCommand::exec(Irccd &irccd, TransportClient &client, const nlohmann::json &args)
{
    irccd.plugins().require(util::json::requireIdentifier(args, "plugin"))->onReload(irccd);
    client.success("plugin-reload");
}

} // !command

} // !irccd
