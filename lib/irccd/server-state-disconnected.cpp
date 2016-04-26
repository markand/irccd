/*
 * server-state-disconnected.cpp -- disconnected state
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

#include "logger.hpp"
#include "server-state-connecting.hpp"
#include "server-state-disconnected.hpp"
#include "server-private.hpp"

namespace irccd {

namespace state {

void Disconnected::prepare(Server &server, fd_set &, fd_set &, net::Handle &)
{
	const ServerInfo &info = server.info();
	ServerSettings &settings = server.settings();
	ServerCache &cache = server.cache();

	if (settings.reconnect_tries == 0) {
		log::warning() << "server " << info.name << ": reconnection disabled, skipping" << std::endl;
		server.onDie();
	} else if (settings.reconnect_tries > 0 && cache.reconnect_current > settings.reconnect_tries) {
		log::warning() << "server " << info.name << ": giving up" << std::endl;
		server.onDie();
	} else {
		if (m_timer.elapsed() > static_cast<unsigned>(settings.reconnect_timeout * 1000)) {
			irc_disconnect(server.session());

			server.cache().reconnect_current ++;
			server.next(std::make_unique<state::Connecting>());
		}
	}
}

std::string Disconnected::ident() const
{
	return "Disconnected";
}

} // !state

} // !irccd
