/*
 * cmd-server-list.cpp -- implementation of server-list transport command
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

#include "cmd-server-list.hpp"
#include "irccd.hpp"
#include "server.hpp"
#include "service-server.hpp"

namespace irccd {

namespace command {

ServerList::ServerList()
    : Command("server-list", "Server")
{
}

std::string ServerList::help() const
{
    return "";
}

json::Value ServerList::exec(Irccd &irccd, const json::Value &) const
{
    auto json = json::object({});
    auto list = json::array({});

    for (const auto &server : irccd.serverService().servers())
        list.append(server->name());

    json.insert("list", std::move(list));

    return json;
}

void ServerList::result(Irccdctl &, const json::Value &response) const
{
    for (const auto &n : response.valueOr("list", json::Type::Array, json::array({})))
        std::cout << n.toString() << std::endl;
}

} // !command

} // !irccd
