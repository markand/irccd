/*
 * js-server.cpp -- Irccd.Server API
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

#include <sstream>
#include <unordered_map>

#include "irccd.hpp"
#include "js-server.hpp"
#include "server.hpp"

namespace irccd {

namespace {

/*
 * Method: Server.cmode(channel, mode)
 * ------------------------------------------------------------------
 *
 * Change a channel mode.
 *
 * Arguments:
 *   - channel, the channel,
 *   - mode, the mode.
 */
duk::Ret cmode(duk::ContextPtr ctx)
{
	duk::self<duk::Shared<Server>>(ctx)->cmode(duk::require<std::string>(ctx, 0), duk::require<std::string>(ctx, 1));

	return 0;
}

/*
 * Method: Server.cnotice(channel, message)
 * ------------------------------------------------------------------
 *
 * Send a channel notice.
 *
 * Arguments:
 *   - channel, the channel,
 *   - message, the message.
 */
duk::Ret cnotice(duk::ContextPtr ctx)
{
	duk::self<duk::Shared<Server>>(ctx)->cnotice(duk::require<std::string>(ctx, 0), duk::require<std::string>(ctx, 1));

	return 0;
}

/*
 * Method: Server.info()
 * ------------------------------------------------------------------
 *
 * Get the server information as an object containing the following properties:
 *
 * name: the server unique name
 * host: the host name
 * port: the port number
 * ssl: true if using ssl
 * sslVerify: true if ssl was verified
 * channels: an array of all channels
 */
duk::Ret info(duk::ContextPtr ctx)
{
	auto server = duk::self<duk::Shared<Server>>(ctx);

	duk::push(ctx, duk::Object{});
	duk::putProperty(ctx, -1, "name", server->info().name);
	duk::putProperty(ctx, -1, "host", server->info().host);
	duk::putProperty<int>(ctx, -1, "port", server->info().port);
	duk::putProperty<bool>(ctx, -1, "ssl", server->info().flags & ServerInfo::Ssl);
	duk::putProperty<bool>(ctx, -1, "sslVerify", server->info().flags & ServerInfo::SslVerify);
	duk::putProperty(ctx, -1, "commandChar", server->settings().command);
	duk::putProperty(ctx, -1, "realname", server->identity().realname);
	duk::putProperty(ctx, -1, "nickname", server->identity().nickname);
	duk::putProperty(ctx, -1, "username", server->identity().username);

	/* Channels */
	duk::push(ctx, duk::Array{});

	int i = 0;
	for (const auto &channel : server->settings().channels) {
		duk::putProperty(ctx, -1, i++, channel.name);
	}

	duk::putProperty(ctx, -2, "channels");

	return 1;
}

/*
 * Method: Server.invite(target, channel)
 * ------------------------------------------------------------------
 *
 * Invite someone to a channel.
 *
 * Arguments:
 *   - target, the target to invite,
 *   - channel, the channel.
 */
duk::Ret invite(duk::ContextPtr ctx)
{
	duk::self<duk::Shared<Server>>(ctx)->invite(duk::require<std::string>(ctx, 0), duk::require<std::string>(ctx, 1));

	return 0;
}

/*
 * Method: Server.join(channel, password = undefined)
 * ------------------------------------------------------------------
 *
 * Join a channel with an optional password.
 *
 * Arguments:
 *   - channel, the channel to join,
 *   - password, the password or undefined to not use.
 */
duk::Ret join(duk::ContextPtr ctx)
{
	duk::self<duk::Shared<Server>>(ctx)->join(duk::require<std::string>(ctx, 0), duk::optional<std::string>(ctx, 1, ""));

	return 0;
}

/*
 * Method: Server.kick(target, channel, reason = undefined)
 * ------------------------------------------------------------------
 *
 * Kick someone from a channel.
 *
 * Arguments:
 *   - target, the target to kick,
 *   - channel, the channel,
 *   - reason, the optional reason or undefined to not set.
 */
duk::Ret kick(duk::ContextPtr ctx)
{
	duk::self<duk::Shared<Server>>(ctx)->kick(
		duk::require<std::string>(ctx, 0),
		duk::require<std::string>(ctx, 1),
		duk::optional<std::string>(ctx, 2, "")
	);

	return 0;
}

/*
 * Method: Server.me(target, message)
 * ------------------------------------------------------------------
 *
 * Send a CTCP Action.
 *
 * Arguments:
 *   - target, the target or a channel,
 *   - message, the message.
 */
duk::Ret me(duk::ContextPtr ctx)
{
	duk::self<duk::Shared<Server>>(ctx)->me(duk::require<std::string>(ctx, 0), duk::require<std::string>(ctx, 1));

	return 0;
}

/*
 * Method: Server.message(target, message)
 * ------------------------------------------------------------------
 *
 * Send a message.
 *
 * Arguments:
 *   - target, the target or a channel,
 *   - message, the message.
 */
duk::Ret message(duk::ContextPtr ctx)
{
	duk::self<duk::Shared<Server>>(ctx)->message(duk::require<std::string>(ctx, 0), duk::require<std::string>(ctx, 1));

	return 0;
}

/*
 * Method: Server.mode(mode)
 * ------------------------------------------------------------------
 *
 * Change your mode.
 *
 * Arguments:
 *   - mode, the new mode.
 */
duk::Ret mode(duk::ContextPtr ctx)
{
	duk::self<duk::Shared<Server>>(ctx)->mode(duk::require<std::string>(ctx, 0));

	return 0;
}

/*
 * Method: Server.names(channel)
 * ------------------------------------------------------------------
 *
 * Get the list of names from a channel.
 *
 * Arguments:
 *   - channel, the channel.
 */
duk::Ret names(duk::ContextPtr ctx)
{
	duk::self<duk::Shared<Server>>(ctx)->names(duk::require<std::string>(ctx, 0));

	return 0;
}

/*
 * Method: Server.nick(nickname)
 * ------------------------------------------------------------------
 *
 * Change the nickname.
 *
 * Arguments:
 *   - nickname, the nickname.
 */
duk::Ret nick(duk::ContextPtr ctx)
{
	duk::self<duk::Shared<Server>>(ctx)->nick(duk::require<std::string>(ctx, 0));

	return 0;
}

/*
 * Method: Server.notice(target, message)
 * ------------------------------------------------------------------
 *
 * Send a private notice.
 *
 * Arguments:
 *   - target, the target,
 *   - message, the notice message.
 */
duk::Ret notice(duk::ContextPtr ctx)
{
	duk::self<duk::Shared<Server>>(ctx)->notice(duk::require<std::string>(ctx, 0), duk::require<std::string>(ctx, 1));

	return 0;
}

/*
 * Method: Server.part(channel, reason = undefined)
 * ------------------------------------------------------------------
 *
 * Leave a channel.
 *
 * Arguments:
 *   - channel, the channel to leave,
 *   - reason, the optional reason, keep undefined for portability.
 */
duk::Ret part(duk::ContextPtr ctx)
{
	duk::self<duk::Shared<Server>>(ctx)->part(duk::require<std::string>(ctx, 0), duk::optional<std::string>(ctx, 1, ""));

	return 0;
}

/*
 * Method: Server.send(raw)
 * ------------------------------------------------------------------
 *
 * Send a raw message to the IRC server.
 *
 * Arguments:
 *   - raw, the raw message (without terminators).
 */
duk::Ret send(duk::ContextPtr ctx)
{
	duk::self<duk::Shared<Server>>(ctx)->send(duk::require<std::string>(ctx, 0));

	return 0;
}

/*
 * Method: Server.topic(channel, topic)
 * ------------------------------------------------------------------
 *
 * Change a channel topic.
 *
 * Arguments:
 *   - channel, the channel,
 *   - topic, the new topic.
 */
duk::Ret topic(duk::ContextPtr ctx)
{
	duk::self<duk::Shared<Server>>(ctx)->topic(duk::require<std::string>(ctx, 0), duk::require<std::string>(ctx, 1));

	return 0;
}

/*
 * Method: Server.whois(target)
 * ------------------------------------------------------------------
 *
 * Get whois information.
 *
 * Arguments:
 *   - target, the target.
 */
duk::Ret whois(duk::ContextPtr ctx)
{
	duk::self<duk::Shared<Server>>(ctx)->whois(duk::require<std::string>(ctx, 0));

	return 0;
}

/*
 * Method: Server.toString()
 * ------------------------------------------------------------------
 *
 * Convert the object to std::string, convenience for adding the object
 * as property key.
 *
 * Returns:
 *   The server name (unique).
 */
duk::Ret toString(duk::ContextPtr ctx)
{
	duk::push(ctx, duk::self<duk::Shared<Server>>(ctx)->info().name);

	return 1;
}

/*
 * Function: Irccd.Server(params) [constructor]
 * ------------------------------------------------------------------
 *
 * Construct a new server.
 *
 * Params must be filled with the following properties:
 *
 * name: the name,
 * host: the host,
 * ipv6: true to use ipv6,	(Optional: default false)
 * port: the port number,	(Optional: default 6667)
 * password: the password,	(Optional: default none)
 * channels: array of channels	(Optiona: default empty)
 * ssl: true to use ssl,	(Optional: default false)
 * sslVerify: true to verify	(Optional: default true)
 * nickname: "nickname",	(Optional, default: irccd)
 * username: "user name",	(Optional, default: irccd)
 * realname: "real name",	(Optional, default: IRC Client Daemon)
 * commandChar: "!",		(Optional, the command char, default: "!")
 */
duk::Ret constructor(duk::ContextPtr ctx)
{
	if (!duk_is_constructor_call(ctx))
		return 0;

	ServerInfo info;
	ServerIdentity identity;
	ServerSettings settings;

	/* Information part */
	info.name = duk::getProperty<std::string>(ctx, 0, "name");
	info.host = duk::getProperty<std::string>(ctx, 0, "host");
	info.port = duk::optionalProperty<int>(ctx, 0, "port", (int)info.port);
	info.password = duk::optionalProperty<std::string>(ctx, 0, "password", "");

	if (duk::optionalProperty<bool>(ctx, 0, "ipv6", false)) {
		info.flags |= ServerInfo::Ipv6;
	}

	/* Identity part */
	identity.nickname = duk::optionalProperty<std::string>(ctx, 0, "nickname", identity.nickname);
	identity.username = duk::optionalProperty<std::string>(ctx, 0, "username", identity.username);
	identity.realname = duk::optionalProperty<std::string>(ctx, 0, "realname", identity.realname);
	identity.ctcpversion = duk::optionalProperty<std::string>(ctx, 0, "version", identity.ctcpversion);

	/* Settings part */
	for (const auto &chan: duk::getProperty<std::vector<std::string>>(ctx, 0, "channels")) {
		settings.channels.push_back(Server::splitChannel(chan));
	}

	settings.reconnectTries = duk::optionalProperty<int>(ctx, 0, "recoTries", (int)settings.reconnectTries);
	settings.reconnectDelay = duk::optionalProperty<int>(ctx, 0, "recoTimeout", (int)settings.reconnectDelay);

	if (duk::optionalProperty<bool>(ctx, 0, "joinInvite", false)) {
		settings.flags |= ServerSettings::JoinInvite;
	}
	if (duk::optionalProperty<bool>(ctx, 0, "autoRejoin", false)) {
		settings.flags |= ServerSettings::AutoRejoin;
	}

	try {
		duk::construct(ctx, duk::Shared<Server>{std::make_shared<Server>(std::move(info), std::move(identity), std::move(settings))});
	} catch (const std::exception &ex) {
		duk::raise(ctx, duk::Error(ex.what()));
	}

	return 0;
}

/*
 * Function: Irccd.Server.add(s)
 * ------------------------------------------------------------------
 *
 * Register a new server to the irccd instance.
 *
 * Arguments:
 *   - s, the server to add.
 */
duk::Ret add(duk::ContextPtr ctx)
{
	auto server = duk::get<duk::Shared<Server>>(ctx, 0);

	if (server) {
		duk::getGlobal<duk::RawPointer<Irccd>>(ctx, "\xff""\xff""irccd")->addServer(server);
	}

	return 0;
}

/*
 * Function: Irccd.Server.find(name)
 * ------------------------------------------------------------------
 *
 * Find a server by name.
 *
 * Arguments:
 *   - name, the server name
 * Returns:
 *   The server object or undefined if not found.
 */
duk::Ret find(duk::ContextPtr ctx)
{
	const auto name = duk::require<std::string>(ctx, 0);
	const auto irccd = duk::getGlobal<duk::RawPointer<Irccd>>(ctx, "\xff""\xff""irccd");

	try {
		duk::push(ctx, duk::Shared<Server>{irccd->requireServer(name)});
	} catch (...) {
		return 0;
	}

	return 1;
}

/*
 * Function: Irccd.Server.list()
 * ------------------------------------------------------------------
 *
 * Get the map of all loaded servers.
 *
 * Returns:
 *   An object with string-to-servers pairs.
 */
duk::Ret list(duk::ContextPtr ctx)
{
	duk::push(ctx, duk::Object{});

	for (const auto &pair : duk::getGlobal<duk::RawPointer<Irccd>>(ctx, "\xff""\xff""irccd")->servers()) {
		duk::putProperty(ctx, -1, pair.first, duk::Shared<Server>{pair.second});
	}

	return 1;
}

/*
 * Function: Irccd.Server.remove(name)
 * ------------------------------------------------------------------
 *
 * Remove a server from the irccd instance. You can pass the server object since it's coercible to a string.
 *
 * Arguments:
 *   - name the server name.
 */
duk::Ret remove(duk::ContextPtr ctx)
{
	duk::getGlobal<duk::RawPointer<Irccd>>(ctx, "\xff""\xff""irccd")->removeServer(duk::require<std::string>(ctx, 0));

	return 0;
}

const duk::FunctionMap methods{
	{ "cmode",	{ cmode,	2		} },
	{ "cnotice",	{ cnotice,	2		} },
	{ "info",	{ info,		0		} },
	{ "invite",	{ invite,	2		} },
	{ "join",	{ join,		DUK_VARARGS	} },
	{ "kick",	{ kick,		DUK_VARARGS	} },
	{ "me",		{ me,		2		} },
	{ "message",	{ message,	2		} },
	{ "mode",	{ mode,		1		} },
	{ "names",	{ names,	1		} },
	{ "nick",	{ nick,		1		} },
	{ "notice",	{ notice,	2		} },
	{ "part",	{ part,		DUK_VARARGS	} },
	{ "send",	{ send,		1		} },
	{ "topic",	{ topic,	2		} },
	{ "whois",	{ whois,	1		} },
	{ "toString",	{ toString,	0		} }
};

const duk::FunctionMap functions{
	{ "add",	{ add,		1		} },
	{ "find",	{ find,		1		} },
	{ "list",	{ list,		0		} },
	{ "remove",	{ remove,	1		} }
};

} // !namespace

void loadJsServer(duk::ContextPtr ctx)
{
	duk::StackAssert sa(ctx);

	duk::getGlobal<void>(ctx, "Irccd");
	duk::push(ctx, duk::Function{constructor, 1});
	duk::push(ctx, functions);
	duk::push(ctx, duk::Object());
	duk::push(ctx, methods);
	duk::putProperty(ctx, -2, "prototype");
	duk::putProperty(ctx, -2, "Server");
	duk::pop(ctx);
}

} // !irccd
