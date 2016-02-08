/*
 * command-server-reconnect.cpp -- implementation of irccdctl server-reconnect
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

#include "command-server-reconnect.h"

namespace irccd {

namespace command {

void ServerReconnect::usage(Irccdctl &) const
{
	log::warning() << "usage: " << sys::programName() << " server-reconnect [server]\n\n";
	log::warning() << "Force reconnection of one or all servers.\n\n";
	log::warning() << "If server is not specified, all servers will try to reconnect.\n\n";
	log::warning() << "Example:\n";
	log::warning() << "\t" << sys::programName() << " server-reconnect\n";
	log::warning() << "\t" << sys::programName() << " server-reconnect wanadoo" << std::endl;
}

void ServerReconnect::exec(Irccdctl &ctl, const std::vector<std::string> &args) const
{
	json::Value object = json::object({
		{ "command", "server-reconnect" }
	});

	if (args.size() >= 1)
		object.insert("server", args[0]);

	ctl.connection().send(object.toJson(0));
	ctl.connection().verify("server-reconnect");
}

} // !command

} // !irccd
