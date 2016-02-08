/*
 * command-plugin-load.cpp -- implementation of plugin-load transport command
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

#include "command-plugin-load.h"

namespace irccd {

namespace command {

void PluginLoad::exec(Irccd &irccd, TransportClient &tc, const json::Value &object) const
{
#if defined(WITH_JS)
	std::string name = object.at("plugin").toString();

	irccd.loadPlugin(name, name, true);
	tc.ok("plugin-load");
#else
	throw std::runtime_error("JavaScript disabled");
#endif
}

} // !command

} // !irccd
