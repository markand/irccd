/*
 * command-plugin-info.cpp -- implementation of plugin-info transport command
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

#include <irccd-config.h>

#include "command-plugin-info.h"

namespace irccd {

namespace command {

void PluginInfo::exec(Irccd &irccd, TransportClient &tc, const json::Value &object) const
{
#if defined(WITH_JS)
	std::shared_ptr<Plugin> plugin = irccd.requirePlugin(object.at("plugin").toString());
	json::Value result = json::object({
		{ "response",	"plugin-info"		},
		{ "author",	plugin->info().author	},
		{ "license",	plugin->info().license	},
		{ "summary",	plugin->info().summary	},
		{ "version",	plugin->info().version	}
	});

	tc.send(result.toJson(0));
#else
	(void)irccd;
	(void)tc;
	(void)object;

	throw std::runtime_error("JavaScript disabled");
#endif
}

} // !command

} // !irccd

