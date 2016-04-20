/*
 * cmd-plugin-info.cpp -- implementation of plugin-info command
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

#include <irccd/sysconfig.hpp>

#include "cmd-plugin-info.hpp"
#include "irccd.hpp"

namespace irccd {

namespace command {

PluginInfo::PluginInfo()
	: RemoteCommand("plugin-info", "Plugins")
{
}

std::string PluginInfo::help() const
{
	return "Get plugin information.";
}

std::vector<RemoteCommand::Arg> PluginInfo::args() const
{
	return {{ "plugin", true }};
}

json::Value PluginInfo::request(Irccdctl &, const RemoteCommandRequest &args) const
{
	return json::object({{ "plugin", args.arg(0) }});
}

json::Value PluginInfo::exec(Irccd &irccd, const json::Value &request) const
{
#if defined(WITH_JS)
	auto plugin = irccd.requirePlugin(request.at("plugin").toString());
	
	return json::object({
		{ "author",	plugin->info().author	},
		{ "license",	plugin->info().license	},
		{ "summary",	plugin->info().summary	},
		{ "version",	plugin->info().version	}
	});
#else
	(void)irccd;
	(void)object;

	throw std::runtime_error("JavaScript disabled");
#endif
}

void PluginInfo::result(Irccdctl &irccdctl, const json::Value &result) const
{
	RemoteCommand::result(irccdctl, result);

	/* Plugin information */
	if (result.valueOr("status", false).toBool()) {
		std::cout << std::boolalpha;
		std::cout << "Author         : " << result.valueOr("author", "").toString(true) << std::endl;
		std::cout << "License        : " << result.valueOr("license", "").toString(true) << std::endl;
		std::cout << "Summary        : " << result.valueOr("summary", "").toString(true) << std::endl;
		std::cout << "Version        : " << result.valueOr("version", "").toString(true) << std::endl;
	}
}

} // !command

} // !irccd

