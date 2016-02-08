/*
 * command-server-kick.cpp -- implementation of irccdctl server-kick
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

#include "command-server-kick.h"

namespace irccd {

namespace command {

void ServerKick::usage(Irccdctl &) const
{
	log::warning() << "usage: " << sys::programName() << " server-kick server target channel [reason]\n\n";
	log::warning() << "Kick the specified target from the channel, the reason is optional.\n\n";
	log::warning() << "Example:\n";
	log::warning() << "\t" << sys::programName() << " server-kick freenode jean #staff \"Stop flooding\"" << std::endl;
}

void ServerKick::exec(Irccdctl &ctl, const std::vector<std::string> &args) const
{
	if (args.size() < 3)
		throw std::invalid_argument("server-kick requires at least 3 arguments ");

	json::Value object = json::object({
		{ "command",	"server-kick"	},
		{ "server",	args[0]		},
		{ "target",	args[1]		},
		{ "channel",	args[2]		}
	});

	if (args.size() == 4)
		object.insert("reason", args[3]);

	ctl.connection().send(object.toJson(0));
	ctl.connection().verify("server-kick");
}

} // !command

} // !irccd
