/*
 * command-help.cpp -- implementation of irccdctl help
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

#include "command-help.h"

namespace irccd {

namespace command {

void Help::usage(Irccdctl &) const
{
	log::warning() << "usage: " << sys::programName() << " help topic\n\n";
	log::warning() << "Show command help\n\n";
	log::warning() << sys::programName() << " help server-message" << std::endl;
}

void Help::exec(Irccdctl &ctl, const std::vector<std::string> &args) const
{
	if (args.size() < 1)
		throw std::invalid_argument("help requires 1 argument");

	auto it = ctl.commands().find(args[0]);

	if (it == ctl.commands().end())
		throw std::invalid_argument(": there is no topic named '" + args[0] + "'");

	it->second->usage(ctl);
}

} // !command

} // !irccd
