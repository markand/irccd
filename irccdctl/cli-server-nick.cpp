/*
 * cli-server-nick.cpp -- implementation of irccdctl server-nick
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

#include "cli-server-nick.hpp"

namespace irccd {

namespace cli {

ServerNickCli::ServerNickCli()
    : Cli("server-nick",
          "change your nickname",
          "server-nick server nickname",
          "Change irccd's nickname.\n\n"
          "Example:\n"
          "\tirccdctl server-nick freenode david")
{
}

void ServerNickCli::exec(Irccdctl &irccdctl, const std::vector<std::string> &args)
{
    if (args.size() < 2)
        throw std::invalid_argument("server-nick requires 2 arguments");

    check(request(irccdctl, {
        { "server",     args[0] },
        { "nickname",   args[1] }
    }));
}

} // !cli

} // !irccd
