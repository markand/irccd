/*
 * command-server-reconnect.cpp -- implementation of irccdctl server-reconnect
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

#include "cli-server-reconnect.hpp"

namespace irccd {

namespace cli {

#if 0

void ServerReconnect::usage(Irccdctl &) const
{
"usage: " << sys::programName() << " server-reconnect [server]\n\n";

}

#endif

ServerReconnectCli::ServerReconnectCli()
    : Cli("server-reconnect",
          "force reconnection of a server",
          "server-reconnect [server]",
          "Force reconnection of one or all servers.\n\n"
          "If server is not specified, all servers will try to reconnect.\n\n"
          "Example:\n"
          "\tirccdctl server-reconnect\n"
          "\tirccdctl server-reconnect wanadoo")
{
}

void ServerReconnectCli::exec(Irccdctl &irccdctl, const std::vector<std::string> &args)
{
    auto object = nlohmann::json::object({
        { "command", "server-reconnect" }
    });

    if (args.size() >= 1)
        object["server"] = args[0];

    check(request(irccdctl, object));
}

} // !command

} // !irccd
