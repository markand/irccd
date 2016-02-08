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

#include "command-plugin-list.h"

namespace irccd {

namespace command {

void PluginList::exec(Irccd &irccd, TransportClient &tc, const json::Value &) const
{
#if defined(WITH_JS)
	std::ostringstream oss;

	oss << "{"
	    <<   "\"response\":\"plugin-list\","
	    <<   "\"status\":\"ok\","
	    <<   "\"list\":[";

	auto it = irccd.plugins().begin();
	auto end = irccd.plugins().end();

	while (it != end) {
		oss << "\"" << it->first << "\"";

		if (++it != end)
			oss << ",";
	}

	oss << "]}";

	tc.send(oss.str());
#else
	throw std::runtime_error("JavaScript disabled");
#endif
}

} // !command

}
