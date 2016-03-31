/*
 * cmd-help.cpp -- implementation of irccdctl help
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

#include <irccd/irccdctl.h>
#include <irccd/logger.h>

#include "cmd-help.h"

namespace irccd {

namespace command {

Help::Help()
	: RemoteCommand("help", "General")
{
}

std::vector<RemoteCommand::Arg> Help::args() const
{
	return {{ "command", true }};
}

std::string Help::help() const
{
	return "Get help about a command.";
}

json::Value Help::request(Irccdctl &irccdctl, const RemoteCommandRequest &args) const
{
	auto it = irccdctl.commands().find(args.arg(0U));

	if (it == irccdctl.commands().end())
		log::warning() << "there is no command named: " << args.arg(0U) << std::endl;
	else
		log::warning() << it->second->usage() << std::flush;

	return nullptr;
}

} // !command

} // !irccd
