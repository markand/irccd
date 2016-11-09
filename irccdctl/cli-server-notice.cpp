/*
 * cli-server-notice.cpp -- implementation of irccdctl server-notice
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

#include "cli-server-notice.hpp"

namespace irccd {

namespace cli {

ServerNoticeCli::ServerNoticeCli()
    : Cli("server-notice",
          "send a private notice",
          "server-notice server target message",
          "Send a private notice to the specified target.\n\n"
          "Example:\n"
          "\tirccdctl server-notice freenode jean \"I know you are here.\"")
{
}

void ServerNoticeCli::exec(Irccdctl &irccdctl, const std::vector<std::string> &args)
{
    if (args.size() < 3)
        throw std::invalid_argument("server-notice requires 3 arguments");

    check(request(irccdctl, {
        { "server",     args[0] },
        { "target",     args[1] },
        { "message",    args[2] }
    }));
}

} // !cli

} // !irccd
