/*
 * command-plugin-reload.cpp -- implementation of irccdctl plugin-reload
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

#include "cli-plugin-reload.hpp"

namespace irccd {

namespace cli {

PluginReloadCli::PluginReloadCli()
    : Cli("plugin-reload",
          "reload a plugin",
          "plugin-reload plugin",
          "Call the onReload event on the specified plugin.")
{
}

void PluginReloadCli::exec(Irccdctl &irccdctl, const std::vector<std::string> &args)
{
    if (args.size() < 1)
        throw std::invalid_argument("plugin-reload requires 1 argument");

    check(request(irccdctl, {{ "plugin", args[0] }}));
}

} // !command

} // !irccd
