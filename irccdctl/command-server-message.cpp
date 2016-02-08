/*
 * command-server-message.cpp -- implementation of irccdctl server-message
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

#include "command-server-message.h"

namespace irccd {

namespace command {

void ServerMessage::usage(Irccdctl &) const
{
	log::warning() << "usage: " << sys::programName() << " server-message server target message\n\n";
	log::warning() << "Send a message to the specified target or channel.\n\n";
	log::warning() << "Example:\n";
	log::warning() << "\t" << sys::programName() << " server-message freenode #staff \"Hello from irccd\"" << std::endl;
}

void ServerMessage::exec(Irccdctl &ctl, const std::vector<std::string> &args) const
{
	if (args.size() < 3)
		throw std::invalid_argument("server-message requires 3 arguments");

	ctl.connection().send(json::object({
		{ "command",	"server-message"	},
		{ "server",	args[0]			},
		{ "target",	args[1]			},
		{ "message",	args[2]			}
	}).toJson(0));

	ctl.connection().verify("server-message");
}

} // !command

} // !irccd
