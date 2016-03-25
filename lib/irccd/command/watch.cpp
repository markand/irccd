/*
 * command-watch.cpp -- implementation of irccdctl watch
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

#include <functional>
#include <iostream>
#include <sstream>
#include <unordered_map>

#include <irccd/irccdctl.h>

#include "watch.h"

namespace irccd {

namespace command {

namespace {

void onChannelMode(const json::Value &v)
{
	std::cout << "event:       onChannelMode\n";
	std::cout << "server:      " << v.valueOr("server", "").toString() << "\n";
	std::cout << "origin:      " << v.valueOr("origin", "").toString() << "\n";
	std::cout << "mode:        " << v.valueOr("mode", "").toString() << "\n";
	std::cout << "argument:    " << v.valueOr("argument", "").toString() << "\n";
}

void onChannelNotice(const json::Value &v)
{
	std::cout << "event:       onChannelNotice\n";
	std::cout << "server:      " << v.valueOr("server", "").toString() << "\n";
	std::cout << "origin:      " << v.valueOr("origin", "").toString() << "\n";
	std::cout << "channel:     " << v.valueOr("channel", "").toString() << "\n";
	std::cout << "message:     " << v.valueOr("message", "").toString() << "\n";
}

void onConnect(const json::Value &v)
{
	std::cout << "event:       onConnect\n";
	std::cout << "server:      " << v.valueOr("server", "").toString() << "\n";
}

void onInvite(const json::Value &v)
{
	std::cout << "event:       onInvite\n";
	std::cout << "server:      " << v.valueOr("server", "").toString() << "\n";
	std::cout << "origin:      " << v.valueOr("origin", "").toString() << "\n";
	std::cout << "channel:     " << v.valueOr("channel", "").toString() << "\n";
}

void onJoin(const json::Value &v)
{
	std::cout << "event:       onJoin\n";
	std::cout << "server:      " << v.valueOr("server", "").toString() << "\n";
	std::cout << "origin:      " << v.valueOr("origin", "").toString() << "\n";
	std::cout << "channel:     " << v.valueOr("channel", "").toString() << "\n";
}

void onKick(const json::Value &v)
{
	std::cout << "event:       onKick\n";
	std::cout << "server:      " << v.valueOr("server", "").toString() << "\n";
	std::cout << "origin:      " << v.valueOr("origin", "").toString() << "\n";
	std::cout << "channel:     " << v.valueOr("channel", "").toString() << "\n";
	std::cout << "target:      " << v.valueOr("target", "").toString() << "\n";
	std::cout << "reason:      " << v.valueOr("reason", "").toString() << "\n";
}

void onMessage(const json::Value &v)
{
	std::cout << "event:       onMessage\n";
	std::cout << "server:      " << v.valueOr("server", "").toString() << "\n";
	std::cout << "origin:      " << v.valueOr("origin", "").toString() << "\n";
	std::cout << "channel:     " << v.valueOr("channel", "").toString() << "\n";
	std::cout << "message:     " << v.valueOr("message", "").toString() << "\n";
}

void onMe(const json::Value &v)
{
	std::cout << "event:       onMe\n";
	std::cout << "server:      " << v.valueOr("server", "").toString() << "\n";
	std::cout << "origin:      " << v.valueOr("origin", "").toString() << "\n";
	std::cout << "target:      " << v.valueOr("target", "").toString() << "\n";
	std::cout << "message:     " << v.valueOr("message", "").toString() << "\n";
}

void onMode(const json::Value &v)
{
	std::cout << "event:       onMode\n";
	std::cout << "server:      " << v.valueOr("server", "").toString() << "\n";
	std::cout << "origin:      " << v.valueOr("origin", "").toString() << "\n";
	std::cout << "mode:        " << v.valueOr("mode", "").toString() << "\n";
}

void onNames(const json::Value &v)
{
	std::cout << "event:       onNames\n";
	std::cout << "server:      " << v.valueOr("server", "").toString() << "\n";
	std::cout << "channel:     " << v.valueOr("channel", "").toString() << "\n";
	std::cout << "names:       " << v.valueOr("names", "").toJson(0) << "\n";
}

void onNick(const json::Value &v)
{
	std::cout << "event:       onNick\n";
	std::cout << "server:      " << v.valueOr("server", "").toString() << "\n";
	std::cout << "origin:      " << v.valueOr("origin", "").toString() << "\n";
	std::cout << "nickname:    " << v.valueOr("nickname", "").toString() << "\n";
}

void onNotice(const json::Value &v)
{
	std::cout << "event:       onNotice\n";
	std::cout << "server:      " << v.valueOr("server", "").toString() << "\n";
	std::cout << "origin:      " << v.valueOr("origin", "").toString() << "\n";
	std::cout << "message:      " << v.valueOr("message", "").toString() << "\n";
}

void onPart(const json::Value &v)
{
	std::cout << "event:       onPart\n";
	std::cout << "server:      " << v.valueOr("server", "").toString() << "\n";
	std::cout << "origin:      " << v.valueOr("origin", "").toString() << "\n";
	std::cout << "channel:     " << v.valueOr("channel", "").toString() << "\n";
	std::cout << "reason:      " << v.valueOr("reason", "").toString() << "\n";
}

void onQuery(const json::Value &v)
{
	std::cout << "event:       onQuery\n";
	std::cout << "server:      " << v.valueOr("server", "").toString() << "\n";
	std::cout << "origin:      " << v.valueOr("origin", "").toString() << "\n";
	std::cout << "message:     " << v.valueOr("message", "").toString() << "\n";
}

void onTopic(const json::Value &v)
{
	std::cout << "event:       onTopic\n";
	std::cout << "server:      " << v.valueOr("server", "").toString() << "\n";
	std::cout << "origin:      " << v.valueOr("origin", "").toString() << "\n";
	std::cout << "channel:     " << v.valueOr("channel", "").toString() << "\n";
	std::cout << "topic:       " << v.valueOr("topic", "").toString() << "\n";
}

void onWhois(const json::Value &v)
{
	std::cout << "event:       onWhois\n";
	std::cout << "server:      " << v.valueOr("server", "").toString() << "\n";
	std::cout << "nickname:    " << v.valueOr("nickname", "").toString() << "\n";
	std::cout << "username:    " << v.valueOr("username", "").toString() << "\n";
	std::cout << "host:        " << v.valueOr("host", "").toString() << "\n";
	std::cout << "realname:    " << v.valueOr("realname", "").toString() << "\n";
}

const std::unordered_map<std::string, std::function<void (const json::Value &)>> events{
	{ "onChannelMode",	onChannelMode		},
	{ "onChannelNotice",	onChannelNotice		},
	{ "onConnect",		onConnect		},
	{ "onInvite",		onInvite		},
	{ "onJoin",		onJoin			},
	{ "onKick",		onKick			},
	{ "onMessage",		onMessage		},
	{ "onMe",		onMe			},
	{ "onMode",		onMode			},
	{ "onNames",		onNames			},
	{ "onNick",		onNick			},
	{ "onNotice",		onNotice		},
	{ "onPart",		onPart			},
	{ "onQuery",		onQuery			},
	{ "onTopic",		onTopic			},
	{ "onWhois",		onWhois			}
};

} // !namespace

Watch::Watch()
	: RemoteCommand("watch", "General")
{
}

std::vector<RemoteCommand::Option> Watch::options() const
{
	return {{ "format", "f", "format", "format", "output format" }};
}

std::string Watch::help() const
{
	std::ostringstream oss;

	oss << "Start watching irccd events.\n\n";
	oss << "You can use different output formats, native which is a human readable\n";
	oss << "format or json, pretty formatted json.";

	return oss.str();
}

json::Value Watch::request(Irccdctl &ctl, const RemoteCommandRequest &request) const
{
	std::string format = request.optionOr("format", "native");

	if (format != "native" && format != "json")
		throw std::invalid_argument("invalid format given: " + format);

	while (ctl.connection().isConnected()) {
		try {
			auto object = ctl.connection().next(-1);
			auto it = events.find(object.valueOr("event", "").toString());

			/* Silently ignore to avoid breaking user output */
			if (it == events.end())
				continue;

			if (format == "json") {
				std::cout << object.toJson() << std::endl;
			} else {
				it->second(object);
				std::cout << std::endl;
			}
		} catch (...) {
		}
	}

	throw std::runtime_error("connection lost");

	return nullptr;
}

} // !command

} // !irccd
