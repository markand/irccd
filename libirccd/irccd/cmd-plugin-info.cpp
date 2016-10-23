/*
 * cmd-plugin-info.cpp -- implementation of plugin-info command
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

#include <iostream>

#include "cmd-plugin-info.hpp"
#include "irccd.hpp"
#include "plugin.hpp"
#include "service-plugin.hpp"
#include "transport.hpp"
#include "util.hpp"

namespace irccd {

namespace command {

PluginInfoCommand::PluginInfoCommand()
    : Command("plugin-info")
{
}

void PluginInfoCommand::exec(Irccd &irccd, TransportClient &client, const nlohmann::json &args)
{
    auto plugin = irccd.plugins().require(util::json::requireIdentifier(args, "plugin"));

    client.success("plugin-info", {
        { "author",     plugin->author()    },
        { "license",    plugin->license()   },
        { "summary",    plugin->summary()   },
        { "version",    plugin->version()   }
    });
}

} // !command

} // !irccd

