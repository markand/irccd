/*
 * cmd-server-notice.cpp -- implementation of server-notice transport command
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

#include "cmd-server-notice.hpp"
#include "irccd.hpp"
#include "server.hpp"
#include "service-server.hpp"

namespace irccd {

namespace command {

ServerNoticeCommand::ServerNoticeCommand()
    : Command("server-notice", "Server", "Send a private notice")
{
}

std::vector<Command::Arg> ServerNoticeCommand::args() const
{
    return {
        { "server",     true },
        { "target",     true },
        { "message",    true }
    };
}

std::vector<Command::Property> ServerNoticeCommand::properties() const
{
    return {
        { "server",     { nlohmann::json::value_t::string }},
        { "target",     { nlohmann::json::value_t::string }},
        { "message",    { nlohmann::json::value_t::string }}
    };
}

nlohmann::json ServerNoticeCommand::request(Irccdctl &, const CommandRequest &args) const
{
    return nlohmann::json::object({
        { "server",     args.arg(0) },
        { "target",     args.arg(1) },
        { "message",    args.arg(2) }
    });
}

nlohmann::json ServerNoticeCommand::exec(Irccd &irccd, const nlohmann::json &request) const
{
    Command::exec(irccd, request);

    irccd.servers().require(request["server"])->notice(request["target"], request["message"]);

    return nlohmann::json::object();
}

} // !command

} // !irccd
