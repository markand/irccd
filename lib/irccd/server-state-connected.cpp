/*
 * server-state-connected.cpp -- connected state
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

#include <format.h>

#include "logger.hpp"
#include "server-state-connected.hpp"
#include "server-state-disconnected.hpp"
#include "server-private.hpp"

using namespace fmt::literals;

namespace irccd {

namespace state {

void Connected::prepare(Server &server, fd_set &setinput, fd_set &setoutput, net::Handle &maxfd)
{
	const ServerSettings &settings = server.settings();

	if (!irc_is_connected(server.session())) {
		log::warning() << "server " << server.name() << ": disconnected" << std::endl;

		if (settings.reconnectDelay > 0) {
			log::warning("server {}: retrying in {} seconds"_format(server.name(), settings.reconnectDelay));
		}

		server.next(std::make_unique<state::Disconnected>());
	} else if (server.cache().pingTimer.elapsed() >= settings.pingTimeout * 1000) {
		log::warning() << "server " << server.name() << ": ping timeout after "
			       << (server.cache().pingTimer.elapsed() / 1000) << " seconds" << std::endl;
		server.next(std::make_unique<state::Disconnected>());
	} else {
		irc_add_select_descriptors(server.session(), &setinput, &setoutput, reinterpret_cast<int *>(&maxfd));
	}
}

std::string Connected::ident() const
{
	return "Connected";
}

} // !state

} // !irccd
