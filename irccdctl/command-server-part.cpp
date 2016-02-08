/*
 * command-server-part.cpp -- implementation of irccdctl server-part
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

#include "command-server-part.h"

namespace irccd {

namespace command {

void ServerPart::usage(Irccdctl &) const
{
	log::warning() << "usage: " << sys::programName() << " server-part server channel [reason]\n\n";
	log::warning() << "Leave the specified channel, the reason is optional.\n\n";
	log::warning() << "Not all IRC servers support giving a reason to leave a channel, do not specify it if this is a concern.\n\n";
	log::warning() << "Example:\n";
	log::warning() << "\t" << sys::programName() << " server-part freenode #staff\n";
	log::warning() << "\t" << sys::programName() << " server-part freenode #botwar \"too noisy\"" << std::endl;
}

void ServerPart::exec(Irccdctl &ctl, const std::vector<std::string> &args) const
{
	if (args.size() < 2)
		throw std::invalid_argument("server-part requires at least 2 arguments");

	json::Value object = json::object({
		{ "command",	"server-part"	},
		{ "server",	args[0]		},
		{ "channel",	args[1]		}
	});

	if (args.size() >= 3)
		object.insert("reason", args[2]);

	ctl.connection().send(object.toJson(0));
	ctl.connection().verify("server-part");
}

} // !command

} // !irccd
