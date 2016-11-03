/*
 * cli-plugin-unload.cpp -- implementation of irccdctl plugin-unload
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

#include "cli-plugin-unload.hpp"

namespace irccd {

namespace cli {

PluginUnloadCli::PluginUnloadCli()
    : Cli("plugin-unload",
          "unload a plugin",
          "plugin-unload plugin",
          "Unload a loaded plugin from the irccd instance.\n\n"
          "Example:\n"
          "tirccdctl plugin-unload logger")
{
}

void PluginUnloadCli::exec(Irccdctl &irccdctl, const std::vector<std::string> &args)
{
    if (args.size() < 1)
        throw std::invalid_argument("plugin-unload requires 1 argument");

    check(request(irccdctl, {{ "plugin", args[0] }}));
}

} // !cli

} // !irccd
