/*
 * cli-server-kick.cpp -- implementation of irccdctl server-kick
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

#include "cli-server-kick.hpp"

namespace irccd {

namespace cli {

ServerKickCli::ServerKickCli()
    : Cli("server-kick",
          "kick someone from a channel",
          "server-kick server target channel [reason]",
          "Kick the specified target from the channel, the reason is optional.\n\n"
          "Example:\n"
          "\tirccdctl server-kick freenode jean #staff \"Stop flooding\"")
{
}

void ServerKickCli::exec(Irccdctl &irccdctl, const std::vector<std::string> &args)
{
    if (args.size() < 3)
        throw std::invalid_argument("server-kick requires at least 3 arguments ");

    auto object = nlohmann::json::object({
        { "server",     args[0] },
        { "target",     args[1] },
        { "channel",    args[2] }
    });

    if (args.size() == 4)
        object["reason"] = args[3];

    check(request(irccdctl, object));
}

} // !cli

} // !irccd
