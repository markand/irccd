/*
 * server_reconnect_cli.cpp -- implementation of irccdctl server-reconnect
 *
 * Copyright (c) 2013-2017 David Demelier <markand@malikania.fr>
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

#include "server_reconnect_cli.hpp"

namespace irccd {

namespace ctl {

std::string server_reconnect_cli::name() const
{
    return "server-reconnect";
}

void server_reconnect_cli::exec(ctl::controller& ctl, const std::vector<std::string>& args)
{
    auto object = nlohmann::json::object({
        { "command", "server-reconnect" }
    });

    if (args.size() >= 1)
        object["server"] = args[0];

    request(ctl, object);
}

} // !ctl

} // !irccd
