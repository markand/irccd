/*
 * cmd-server-nick.cpp -- implementation of server-nick transport command
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

#include "cmd-server-nick.hpp"
#include "irccd.hpp"
#include "server.hpp"
#include "service-server.hpp"

namespace irccd {

namespace command {

ServerNick::ServerNick()
    : Command("server-nick", "Server", "Change your nickname")
{
}

std::vector<Command::Arg> ServerNick::args() const
{
    return {
        { "server",     true },
        { "nickname",   true }
    };
}

std::vector<Command::Property> ServerNick::properties() const
{
    return {
        { "server",     { nlohmann::json::value_t::string }},
        { "nickname",   { nlohmann::json::value_t::string }}
    };
}

nlohmann::json ServerNick::request(Irccdctl &, const CommandRequest &args) const
{
    return nlohmann::json::object({
        { "server",     args.arg(0) },
        { "nickname",   args.arg(1) }
    });
}

nlohmann::json ServerNick::exec(Irccd &irccd, const nlohmann::json &object) const
{
    Command::exec(irccd, object);

    irccd.servers().require(object["server"])->setNickname(object["nickname"]);

    return nlohmann::json::object();
}

} // !command

} // !irccd
