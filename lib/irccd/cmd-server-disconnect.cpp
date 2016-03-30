/*
 * command-server-disconnect.cpp -- implementation of server-disconnect transport command
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

#include "cmd-server-disconnect.h"
#include "irccd.h"

namespace irccd {

namespace command {

ServerDisconnect::ServerDisconnect()
	: RemoteCommand("server-disconnect", "Server")
{
}

std::string ServerDisconnect::help() const
{
	return "";
}

std::vector<RemoteCommand::Arg> ServerDisconnect::args() const
{
	return {{ "server", false }};
}

json::Value ServerDisconnect::exec(Irccd &irccd, const json::Value &request) const
{
	auto it = request.find("server");

	if (it == request.end())
		irccd.clearServers();
	else
		irccd.removeServer(it->toString());

	return RemoteCommand::exec(irccd, request);
}

} // !command

} // !irccd