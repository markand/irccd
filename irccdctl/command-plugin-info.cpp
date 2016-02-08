/*
 * command-plugin-info.cpp -- implementation of irccdctl plugin-info
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

#include <iostream>

#include "command-plugin-info.h"

namespace irccd {

namespace command {

void PluginInfo::usage(Irccdctl &) const
{
	log::warning() << "usage: " << sys::programName() << " plugin-info name\n\n";
	log::warning() << "Get plugin information.\n\n";
	log::warning() << "Example:\n";
	log::warning() << "\t " << sys::programName() << " plugin-info ask" << std::endl;
}

void PluginInfo::exec(Irccdctl &ctl, const std::vector<std::string> &args) const
{
	if (args.size() < 1)
		throw std::invalid_argument("plugin-info requires 1 argument");

	ctl.connection().send(json::object({
		{ "command",	"plugin-info"	},
		{ "plugin",	args[0]		}
	}).toJson(0));

	json::Value obj = ctl.connection().next("plugin-info");

	if (obj.contains("error"))
		throw std::runtime_error(obj.at("error").toString());

	/* Plugin information */
	std::cout << std::boolalpha;
	std::cout << "Author         : " << obj.valueOr("author", "").toString(true) << std::endl;
	std::cout << "License        : " << obj.valueOr("license", "").toString(true) << std::endl;
	std::cout << "Summary        : " << obj.valueOr("summary", "").toString(true) << std::endl;
	std::cout << "Version        : " << obj.valueOr("version", "").toString(true) << std::endl;
}

} // !command

} // !irccd
