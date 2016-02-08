/*
 * command-server-disconnect.cpp -- implementation of irccdctl server-disconnect
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

#include "command-server-disconnect.h"

namespace irccd {

namespace command {

void ServerDisconnect::usage(Irccdctl &) const
{
	log::warning() << "usage: " << sys::programName() << " server-disconnect [server]\n\n";
	log::warning() << "Disconnect from a server.\n\n";
	log::warning() << "If server is not specified, irccd disconnects all servers.\n\n";
	log::warning() << "Example:\n";
	log::warning() << "\t" << sys::programName() << " server-disconnect localhost" << std::endl;
}

void ServerDisconnect::exec(Irccdctl &ctl, const std::vector<std::string> &args) const
{
	json::Value object = json::object({
		{ "command", "server-disconnect" }
	});

	if (args.size() > 0)
		object.insert("server", args[0]);

	ctl.connection().send(object.toJson(0));
	ctl.connection().verify("server-disconnect");
}

} // !command

} // !irccd
