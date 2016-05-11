/*
 * cmd-server-cmode.cpp -- implementation of server-cmode transport command
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

#include "cmd-server-cmode.hpp"
#include "irccd.hpp"
#include "server.hpp"
#include "service-server.hpp"

namespace irccd {

namespace command {

ServerChannelMode::ServerChannelMode()
	: RemoteCommand("server-cmode", "Server")
{
}

std::string ServerChannelMode::help() const
{
	return "";
}

std::vector<RemoteCommand::Arg> ServerChannelMode::args() const
{
	return {
		{ "server", true },
		{ "channel", true },
		{ "mode", true }
	};
}

json::Value ServerChannelMode::exec(Irccd &irccd, const json::Value &request) const
{
	irccd.serverService().require(request.at("server").toString())->cmode(
		request.at("channel").toString(),
		request.at("mode").toString()
	);

	return RemoteCommand::exec(irccd, request);
}

} // !command

} // !irccd

