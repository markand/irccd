/*
 * server-part.cpp -- implementation of server-part transport command
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

#include <irccd/irccd.h>

#include "server-part.h"

namespace irccd {

namespace command {

ServerPart::ServerPart()
	: RemoteCommand("server-part", "Server")
{
}

std::string ServerPart::help() const
{
	return "";
}

RemoteCommandArgs ServerPart::args() const
{
	return RemoteCommandArgs{
		{ "server", true },
		{ "channel", true },
		{ "reason", false }
	};
}

json::Value ServerPart::request(Irccdctl &, const RemoteCommandRequest &args) const
{
	auto req = json::object({
		{ "server", args.arg(0) },
		{ "channel", args.arg(1) }
	});

	if (args.length() == 3)
		req.insert("reason", args.arg(2));

	return req;
}

json::Value ServerPart::exec(Irccd &irccd, const json::Value &request) const
{
	irccd.requireServer(request.at("server").toString())->part(
		request.at("channel").toString(),
		request.valueOr("reason", "").toString()
	);

	return nullptr;
}

} // !command

} // !irccd
