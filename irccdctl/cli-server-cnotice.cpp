/*
 * cli-server-cnotice.cpp -- implementation of irccdctl server-cnotice
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

#include "cli-server-cnotice.hpp"

namespace irccd {

namespace cli {

ServerChannelNoticeCli::ServerChannelNoticeCli()
    : Cli("server-cnotice",
          "send a channel notice",
          "server-cnotice server channel message",
          "Send a message notice on a channel.\n\n"
          "Example:\n"
          "\tirccdctl server-cnotice freenode #staff \"Don't flood!\"")
{
}

void ServerChannelNoticeCli::exec(Irccdctl &irccdctl, const std::vector<std::string> &args)
{
    if (args.size() < 3)
        throw std::invalid_argument("server-cnotice requires 3 arguments");

    check(request(irccdctl, {
        { "command",    "server-cnotice"    },
        { "server",     args[0]             },
        { "channel",    args[1]             },
        { "message",    args[2]             }
    }));
}

} // !cli

} // !irccd
