/*
 * cli-server-list.cpp -- implementation of irccdctl server-list
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

#include "cli-server-list.hpp"

namespace irccd {

namespace cli {

ServerListCli::ServerListCli()
    : Cli("server-list",
          "get list of servers",
          "server-list\n\n",
          "Get the list of all connected servers.\n\n"
          "Example:\n"
          "\tirccdctl server-list")
{
}

void ServerListCli::exec(Irccdctl &irccdctl, const std::vector<std::string> &)
{
    auto response = request(irccdctl);

    check(response);

    for (const auto &n : response["list"])
        if (n.is_string())
            std::cout << n.get<std::string>() << std::endl;
}

} // !cli

} // !irccd
