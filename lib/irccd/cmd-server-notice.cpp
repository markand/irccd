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

ServerNotice::ServerNotice()
    : Command("server-notice", "Server")
{
}

std::string ServerNotice::help() const
{
    return "";
}

std::vector<Command::Arg> ServerNotice::args() const
{
    return {
        { "server",     true },
        { "target",     true },
        { "message",    true }
    };
}

std::vector<Command::Property> ServerNotice::properties() const
{
    return {
        { "server",     { json::Type::String }},
        { "target",     { json::Type::String }},
        { "message",    { json::Type::String }}
    };
}

json::Value ServerNotice::request(Irccdctl &, const CommandRequest &args) const
{
    return json::object({
        { "server",     args.arg(0) },
        { "target",     args.arg(1) },
        { "message",    args.arg(2) }
    });
}

json::Value ServerNotice::exec(Irccd &irccd, const json::Value &request) const
{
    Command::exec(irccd, request);

    irccd.serverService().require(request.at("server").toString())->notice(
        request.at("target").toString(),
        request.at("message").toString()
    );

    return json::object();
}

} // !command

} // !irccd
