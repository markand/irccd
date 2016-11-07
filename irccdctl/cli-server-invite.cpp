/*
 * command-server-invite.cpp -- implementation of irccdctl server-invite
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

#include "cli-server-invite.hpp"

namespace irccd {

namespace cli {

ServerInviteCli::ServerInviteCli()
    : Cli("server-invite",
          "invite someone",
          "server-invite server nickname channel",
          "Invite the specified target on the channel.\n\n"
          "Example:\n"
          "\tirccdctl server-invite freenode xorg62 #staff")
{
}

void ServerInviteCli::exec(Irccdctl &irccdctl, const std::vector<std::string> &args)
{
    if (args.size() < 3)
        throw std::invalid_argument("server-invite requires 3 arguments");

    check(request(irccdctl, {
        { "command",    "server-invite" },
        { "server",     args[0]         },
        { "target",     args[1]         },
        { "channel",    args[2]         }
    }));
}

} // !cli

} // !irccd
