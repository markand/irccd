/*
 * cmd-server-topic.cpp -- implementation of server-topic transport command
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

#include "cmd-server-topic.hpp"
#include "irccd.hpp"
#include "server.hpp"
#include "service-server.hpp"

namespace irccd {

namespace command {

ServerTopicCommand::ServerTopicCommand()
    : Command("server-topic", "Server", "Change a channel topic")
{
}

std::vector<Command::Arg> ServerTopicCommand::args() const
{
    return {
        { "server",     true },
        { "channel",    true },
        { "topic",      true }
    };
}

std::vector<Command::Property> ServerTopicCommand::properties() const
{
    return {
        { "server",     { nlohmann::json::value_t::string }},
        { "channel",    { nlohmann::json::value_t::string }},
        { "topic",      { nlohmann::json::value_t::string }}
    };
}

nlohmann::json ServerTopicCommand::request(Irccdctl &, const CommandRequest &args) const
{
    return nlohmann::json::object({
        { "server",     args.arg(0) },
        { "channel",    args.arg(1) },
        { "topic",      args.arg(2) }
    });
}

nlohmann::json ServerTopicCommand::exec(Irccd &irccd, const nlohmann::json &request) const
{
    Command::exec(irccd, request);

    irccd.servers().require(request["server"])->topic(request["channel"], request["topic"]);

    return nlohmann::json::object();
}

} // !command

} // !irccd
