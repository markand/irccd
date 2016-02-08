/*
 * command-server-info.cpp -- implementation of server-info transport command
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

#include "command-server-info.h"

namespace irccd {

namespace command {

void ServerInfo::exec(Irccd &irccd, TransportClient &tc, const json::Value &object) const
{
	std::shared_ptr<Server> server = irccd.requireServer(object.at("server").toString());
	json::Value result = json::object({
		{ "response",	"server-info"			},
		{ "name",	server->info().name		},
		{ "host",	server->info().host		},
		{ "port",	server->info().port		},
		{ "nickname",	server->identity().nickname	},
		{ "username",	server->identity().username	},
		{ "realname",	server->identity().realname	}
	});

	if (server->info().flags & irccd::ServerInfo::Ipv6)
		result.insert("ipv6", true);
	if (server->info().flags & irccd::ServerInfo::Ssl)
		result.insert("ssl", true);
	if (server->info().flags & irccd::ServerInfo::SslVerify)
		result.insert("sslVerify", true);

	json::Value channels = json::array({});

	for (const auto &c : server->settings().channels)
		channels.append(c.name);

	result.insert("channels", std::move(channels));
	tc.send(result.toJson(0));
}

} // !command

} // !irccd
