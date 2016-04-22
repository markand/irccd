/*
 * cmd-plugin-unload.cpp -- implementation of plugin-unload transport command
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

#include "cmd-plugin-unload.hpp"
#include "irccd.hpp"
#include "sysconfig.hpp"

namespace irccd {

namespace command {

PluginUnload::PluginUnload()
	: RemoteCommand("plugin-unload", "Plugins")
{
}

std::string PluginUnload::help() const
{
	return "Unload a plugin.";
}

std::vector<RemoteCommand::Arg> PluginUnload::args() const
{
	return {{ "plugin", true }};
}

json::Value PluginUnload::exec(Irccd &irccd, const json::Value &request) const
{
#if defined(WITH_JS)
	irccd.unloadPlugin(request.at("plugin").toString());

	return RemoteCommand::exec(irccd, request);
#else
	(void)irccd;
	(void)tc;
	(void)object;

	throw std::runtime_error("JavaScript disabled");
#endif
}

} // !command

} // !irccd
