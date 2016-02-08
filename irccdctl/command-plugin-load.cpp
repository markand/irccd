/*
 * command-plugin-load.cpp -- implementation of irccdctl plugin-load
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

#include "command-plugin-load.h"

namespace irccd {

namespace command {

void PluginLoad::usage(Irccdctl &) const
{
	log::warning() << "usage: " << sys::programName() << " plugin-load name\n\n";
	log::warning() << "Load a plugin into the irccd instance.\n\n";
	log::warning() << "Example:\n";
	log::warning() << "\t " << sys::programName() << " plugin-load logger" << std::endl;
}

void PluginLoad::exec(Irccdctl &ctl, const std::vector<std::string> &args) const
{
	if (args.size() < 1)
		throw std::invalid_argument("plugin-load requires 1 argument");

	ctl.connection().send(json::object({
		{ "command",	"plugin-load"	},
		{ "plugin",	args[0]		}
	}).toJson(0));

	ctl.connection().verify("plugin-load");
}

} // !command

} // !irccd
