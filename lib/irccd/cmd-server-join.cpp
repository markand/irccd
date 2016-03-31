/*
 * cmd-server-join.cpp -- implementation of server-join transport command
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

#include "cmd-server-join.h"
#include "irccd.h"

namespace irccd {

namespace command {

ServerJoin::ServerJoin()
	: RemoteCommand("server-join", "Server")
{
}

std::string ServerJoin::help() const
{
	return "";
}

std::vector<RemoteCommand::Arg> ServerJoin::args() const
{
	return {
		{ "server", true },
		{ "channel", true },
		{ "password", false }
	};
}

json::Value ServerJoin::request(Irccdctl &, const RemoteCommandRequest &args) const
{
	auto req = json::object({
		{ "server", args.args()[0] },
		{ "channel", args.args()[1] }
	});

	if (args.length() == 3)
		req.insert("password", args.args()[2]);

	return req;
}

json::Value ServerJoin::exec(Irccd &irccd, const json::Value &request) const
{
	irccd.requireServer(
		request.at("server").toString())->join(
		request.at("channel").toString(),
		request.valueOr("password", json::Type::String, "").toString()
	);

	return RemoteCommand::exec(irccd, request);
}

} // !command

} // !irccd
