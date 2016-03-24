/*
 * server-me.cpp -- implementation of server-me transport command
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

#include "server-me.h"

namespace irccd {

namespace command {

ServerMe::ServerMe()
	: RemoteCommand("server-me", "Server")
{
}

std::string ServerMe::help() const
{
	return "";
}

RemoteCommandArgs ServerMe::args() const
{
	return RemoteCommandArgs{
		{ "server", true },
		{ "target", true },
		{ "message", true }
	};
}

json::Value ServerMe::request(Irccdctl &, const RemoteCommandRequest &args) const
{
	return json::object({
		{ "server", args.arg(0) },
		{ "target", args.arg(1) },
		{ "message", args.arg(2) }
	});
}

json::Value ServerMe::exec(Irccd &irccd, const json::Value &request) const
{
	irccd.requireServer(request.at("server").toString())->me(
		request.at("target").toString(),
		request.at("message").toString()
	);

	return nullptr;
}

} // !command

} // !irccd
