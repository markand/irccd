/*
 * server_connect_command.cpp -- implementation of server-connect transport command
 *
 * Copyright (c) 2013-2018 David Demelier <markand@malikania.fr>
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

#include <irccd/daemon/irccd.hpp>
#include <irccd/daemon/server_util.hpp>
#include <irccd/daemon/transport_client.hpp>

#include <irccd/daemon/service/server_service.hpp>

#include "server_connect_command.hpp"

namespace irccd {

std::string server_connect_command::get_name() const noexcept
{
    return "server-connect";
}

void server_connect_command::exec(irccd& irccd, transport_client& client, const document& args)
{
    auto server = server_util::from_json(irccd.get_service(), args);

    if (irccd.servers().has(server->get_id()))
        throw server_error(server_error::already_exists);

    irccd.servers().add(std::move(server));
    client.success("server-connect");
}

} // !irccd
