/*
 * command-plugin-list.cpp -- implementation of irccdctl plugin-list
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

#include "command-plugin-list.h"

namespace irccd {

namespace command {

void PluginList::usage(Irccdctl &) const
{
	log::warning() << "usage: " << sys::programName() << " plugin-list\n\n";
	log::warning() << "Get the list of all loaded plugins.\n\n";
	log::warning() << "Example:\n";
	log::warning() << "\t" << sys::programName() << " plugin-list" << std::endl;
}

void PluginList::exec(Irccdctl &ctl, const std::vector<std::string> &) const
{
	ctl.connection().send(json::object({
		{ "command", "plugin-list" }
	}).toJson(0));

	json::Value object = ctl.connection().next("plugin-list");
	json::Value list = object.valueOr("list", json::Type::Array, json::array({}));

	for (const auto &n : list)
		std::cout << n.toString() << std::endl;
}

} // !command

} // !irccd

