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
#include "service-server.hpp"

namespace irccd {

namespace command {

ServerTopic::ServerTopic()
	: RemoteCommand("server-topic", "Server")
{
}

std::string ServerTopic::help() const
{
	return "";
}

std::vector<RemoteCommand::Arg> ServerTopic::args() const
{
	return {
		{ "server", true },
		{ "channel", true },
		{ "topic", true }
	};
}

json::Value ServerTopic::request(Irccdctl &, const RemoteCommandRequest &args) const
{
	return json::object({
		{ "server", args.arg(0) },
		{ "channel", args.arg(1) },
		{ "topic", args.arg(2) }
	});
}

json::Value ServerTopic::exec(Irccd &irccd, const json::Value &request) const
{
	irccd.serverService().requireServer(request.at("server").toString())->topic(
		request.at("channel").toString(),
		request.at("topic").toString()
	);

	return nullptr;
}

} // !command

} // !irccd
