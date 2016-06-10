/*
 * cmd-plugin-load.cpp -- implementation of plugin-load transport command
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

#include "cmd-plugin-load.hpp"
#include "irccd.hpp"
#include "service-plugin.hpp"
#include "util.hpp"

namespace irccd {

namespace command {

PluginLoad::PluginLoad()
	: Command("plugin-load", "Plugins")
{
}

std::string PluginLoad::help() const
{
	return "Load a plugin.";
}

std::vector<Command::Arg> PluginLoad::args() const
{
	return {{ "plugin", true }};
}

std::vector<Command::Property> PluginLoad::properties() const
{
	return {{ "plugin", { json::Type::String }}};
}

json::Value PluginLoad::exec(Irccd &irccd, const json::Value &request) const
{
	Command::exec(irccd, request);

	irccd.pluginService().load(request.at("plugin").toString());

	return json::object();
}

} // !command

} // !irccd
