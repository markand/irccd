/*
 * command-plugin-list.cpp -- implementation of plugin-list transport command
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

#include <irccd-config.h>

#include "cmd-plugin-list.h"
#include "irccd.h"

namespace irccd {

namespace command {

PluginList::PluginList()
	: RemoteCommand("plugin-list", "Plugins")
{
}

std::string PluginList::help() const
{
	return "Get the list of loaded plugins.";
}

json::Value PluginList::exec(Irccd &irccd, const json::Value &request) const
{
#if defined(WITH_JS)
	json::Value response = RemoteCommand::exec(irccd, request);
	json::Value list = json::array({});

	for (const auto &plugin : irccd.plugins())
		list.append(plugin.first);

	response.insert("list", std::move(list));

	return response;
#else
	(void)irccd;
	(void)tc;

	throw std::runtime_error("JavaScript disabled");
#endif
}

void PluginList::result(Irccdctl &irccdctl, const json::Value &object) const
{
	RemoteCommand::result(irccdctl, object);

	for (const auto &n : object.valueOr("list", json::Type::Array, json::array({})))
		std::cout << n.toString() << std::endl;
}

} // !command

}
