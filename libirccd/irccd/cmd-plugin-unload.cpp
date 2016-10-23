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
#include "transport.hpp"
#include "util.hpp"

namespace irccd {

namespace command {

PluginUnloadCommand::PluginUnloadCommand()
    : Command("plugin-unload")
{
}

void PluginUnloadCommand::exec(Irccd &irccd, TransportClient &client, const nlohmann::json &args)
{
    irccd.plugins().unload(util::json::requireIdentifier(args, "plugin"));
    client.success("plugin-unload");
}

} // !command

} // !irccd
