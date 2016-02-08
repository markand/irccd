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

#include "irccd.h"
#include "js.h"
#include "server.h"

namespace irccd {

namespace {

/*
 * Method: Server.cmode(channel, mode)
 * ------------------------------------------------------------------
 *
 * Change a channel mode.
 *
 * Arguments:
 *   - channel, the channel
 *   - mode, the mode
 */
int cmode(js::Context &ctx)
{
	ctx.self<js::Shared<Server>>()->cmode(ctx.require<std::string>(0), ctx.require<std::string>(1));

	return 0;
}

/*
 * Method: Server.cnotice(channel, message)
 * ------------------------------------------------------------------
 *
 * Send a channel notice.
 *
 * Arguments:
 *   - channel, the channel
 *   - message, the message
 */
int cnotice(js::Context &ctx)
{
	ctx.self<js::Shared<Server>>()->cnotice(ctx.require<std::string>(0), ctx.require<std::string>(1));

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
int info(js::Context &ctx)
{
	auto server = ctx.self<js::Shared<Server>>();

	ctx.push(js::Object{});
	ctx.putProperty(-1, "name", server->info().name);
	ctx.putProperty(-1, "host", server->info().host);
	ctx.putProperty<int>(-1, "port", server->info().port);
	ctx.putProperty<bool>(-1, "ssl", server->info().flags & ServerInfo::Ssl);
	ctx.putProperty<bool>(-1, "sslVerify", server->info().flags & ServerInfo::SslVerify);
	ctx.putProperty(-1, "commandChar", server->settings().command);
	ctx.putProperty(-1, "realname", server->identity().realname);
	ctx.putProperty(-1, "nickname", server->identity().nickname);
	ctx.putProperty(-1, "username", server->identity().username);

	/* Channels */
	ctx.push(js::Array{});

	int i = 0;
	for (const auto &channel : server->settings().channels)
		ctx.putProperty(-1, i++, channel.name);

	ctx.putProperty(-2, "channels");

	return 1;
}

/*
 * Method: Server.invite(target, channel)
 * ------------------------------------------------------------------
 *
 * Invite someone to a channel.
 *
 * Arguments:
 *   - target, the target to invite
 *   - channel, the channel
 */
int invite(js::Context &ctx)
{
	ctx.self<js::Shared<Server>>()->invite(ctx.require<std::string>(0), ctx.require<std::string>(1));

	return 0;
}

/*
 * Method: Server.join(channel, password = undefined)
 * ------------------------------------------------------------------
 *
 * Join a channel with an optional password.
 *
 * Arguments:
 *   - channel, the channel to join
 *   - password, the password or undefined to not use
 */
int join(js::Context &ctx)
{
	ctx.self<js::Shared<Server>>()->join(ctx.require<std::string>(0), ctx.optional<std::string>(1, ""));

	return 0;
}

/*
 * Method: Server.kick(target, channel, reason = undefined)
 * ------------------------------------------------------------------
 *
 * Kick someone from a channel.
 *
 * Arguments:
 *   - target, the target to kick
 *   - channel, the channel
 *   - reason, the optional reason or undefined to not set
 */
int kick(js::Context &ctx)
{
	ctx.self<js::Shared<Server>>()->kick(
		ctx.require<std::string>(0),
		ctx.require<std::string>(1),
		ctx.optional<std::string>(2, "")
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
 *   - target, the target or a channel
 *   - message, the message
 */
int me(js::Context &ctx)
{
	ctx.self<js::Shared<Server>>()->me(ctx.require<std::string>(0), ctx.require<std::string>(1));

	return 0;
}

/*
 * Method: Server.message(target, message)
 * ------------------------------------------------------------------
 *
 * Send a message.
 *
 * Arguments:
 *   - target, the target or a channel
 *   - message, the message
 */
int message(js::Context &ctx)
{
	ctx.self<js::Shared<Server>>()->message(ctx.require<std::string>(0), ctx.require<std::string>(1));

	return 0;
}

/*
 * Method: Server.mode(mode)
 * ------------------------------------------------------------------
 *
 * Change your mode.
 *
 * Arguments:
 *   - mode, the new mode
 */
int mode(js::Context &ctx)
{
	ctx.self<js::Shared<Server>>()->mode(ctx.require<std::string>(0));

	return 0;
}

/*
 * Method: Server.names(channel)
 * ------------------------------------------------------------------
 *
 * Get the list of names from a channel.
 *
 * Arguments:
 *   - channel, the channel
 */
int names(js::Context &ctx)
{
	ctx.self<js::Shared<Server>>()->names(ctx.require<std::string>(0));

	return 0;
}

/*
 * Method: Server.nick(nickname)
 * ------------------------------------------------------------------
 *
 * Change the nickname.
 *
 * Arguments:
 *   - nickname, the nickname
 */
int nick(js::Context &ctx)
{
	ctx.self<js::Shared<Server>>()->nick(ctx.require<std::string>(0));

	return 0;
}

/*
 * Method: Server.notice(target, message)
 * ------------------------------------------------------------------
 *
 * Send a private notice.
 *
 * Arguments:
 *   - target, the target
 *   - message, the notice message
 */
int notice(js::Context &ctx)
{
	ctx.self<js::Shared<Server>>()->notice(ctx.require<std::string>(0), ctx.require<std::string>(1));

	return 0;
}

/*
 * Method: Server.part(channel, reason = undefined)
 * ------------------------------------------------------------------
 *
 * Leave a channel.
 *
 * Arguments:
 *   - channel, the channel to leave
 *   - reason, the optional reason, keep undefined for portability
 */
int part(js::Context &ctx)
{
	ctx.self<js::Shared<Server>>()->part(ctx.require<std::string>(0), ctx.optional<std::string>(1, ""));

	return 0;
}

/*
 * Method: Server.send(raw)
 * ------------------------------------------------------------------
 *
 * Send a raw message to the IRC server.
 *
 * Arguments:
 *   - raw, the raw message (without terminators)
 */
int send(js::Context &ctx)
{
	ctx.self<js::Shared<Server>>()->send(ctx.require<std::string>(0));

	return 0;
}

/*
 * Method: Server.topic(channel, topic)
 * ------------------------------------------------------------------
 *
 * Change a channel topic.
 *
 * Arguments:
 *   - channel, the channel
 *   - topic, the new topic
 */
int topic(js::Context &ctx)
{
	ctx.self<js::Shared<Server>>()->topic(ctx.require<std::string>(0), ctx.require<std::string>(1));

	return 0;
}

/*
 * Method: Server.whois(target)
 * ------------------------------------------------------------------
 *
 * Get whois information.
 *
 * Arguments:
 *   - target, the target
 */
int whois(js::Context &ctx)
{
	ctx.self<js::Shared<Server>>()->whois(ctx.require<std::string>(0));

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
 *   - the server name (unique)
 */
int toString(js::Context &ctx)
{
	ctx.push(ctx.self<js::Shared<Server>>()->info().name);

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
int constructor(js::Context &ctx)
{
	if (!duk_is_constructor_call(ctx))
		return 0;

	ServerInfo info;
	ServerIdentity identity;
	ServerSettings settings;

	/* Information part */
	info.name = ctx.getProperty<std::string>(0, "name");
	info.host = ctx.getProperty<std::string>(0, "host");
	info.port = ctx.optionalProperty<int>(0, "port", info.port);
	info.password = ctx.optionalProperty<std::string>(0, "password", "");

	if (ctx.optionalProperty<bool>(0, "ipv6", false))
		info.flags |= ServerInfo::Ipv6;

	/* Identity part */
	identity.nickname = ctx.optionalProperty<std::string>(0, "nickname", identity.nickname);
	identity.username = ctx.optionalProperty<std::string>(0, "username", identity.username);
	identity.realname = ctx.optionalProperty<std::string>(0, "realname", identity.realname);
	identity.ctcpversion = ctx.optionalProperty<std::string>(0, "version", identity.ctcpversion);

	/* Settings part */
	for (const auto &chan: ctx.getProperty<std::vector<std::string>>(0, "channels"))
		settings.channels.push_back(Server::splitChannel(chan));

	settings.recotries = ctx.optionalProperty<int>(0, "recoTries", settings.recotries);
	settings.recotimeout = ctx.optionalProperty<int>(0, "recoTimeout", settings.recotimeout);

	if (ctx.optionalProperty<bool>(0, "joinInvite", false))
		settings.flags |= ServerSettings::JoinInvite;
	if (ctx.optionalProperty<bool>(0, "autoRejoin", false))
		settings.flags |= ServerSettings::AutoRejoin;

	try {
		ctx.construct(js::Shared<Server>{std::make_shared<Server>(std::move(info), std::move(identity), std::move(settings))});
	} catch (const std::exception &ex) {
		ctx.raise(js::Error{ex.what()});
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
 *   - s, the server to add
 */
int add(js::Context &ctx)
{
	std::shared_ptr<Server> server = ctx.get<js::Shared<Server>>(0);

	if (server)
		ctx.getGlobal<js::RawPointer<Irccd>>("\xff""\xff""irccd")->addServer(server);

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
 *   - the server object or undefined if not found
 */
int find(js::Context &ctx)
{
	const auto name = ctx.require<std::string>(0);
	const auto irccd = ctx.getGlobal<js::RawPointer<Irccd>>("\xff""\xff""irccd");

	try {
		ctx.push(js::Shared<Server>{irccd->requireServer(name)});
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
 *   - an object with string-to-servers pairs.
 */
int list(js::Context &ctx)
{
	ctx.push(js::Object{});

	for (const auto &pair : ctx.getGlobal<js::RawPointer<Irccd>>("\xff""\xff""irccd")->servers())
		ctx.putProperty(-1, pair.first, js::Shared<Server>{pair.second});

	return 1;
}

/*
 * Function: Irccd.Server.remove(name)
 * ------------------------------------------------------------------
 *
 * Remove a server from the irccd instance. You can pass the server object since it's coercible to a string.
 *
 * Arguments:
 *   - name the server name
 */
int remove(js::Context &ctx)
{
	ctx.getGlobal<js::RawPointer<Irccd>>("\xff""\xff""irccd")->removeServer(ctx.require<std::string>(0));

	return 0;
}

const js::FunctionMap methods{
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

const js::FunctionMap functions{
	{ "add",	{ add,		1		} },
	{ "find",	{ find,		1		} },
	{ "list",	{ list,		0		} },
	{ "remove",	{ remove,	1		} }
};

} // !namespace

void loadJsServer(js::Context &ctx)
{
	/* Server prototype for both constructing and passing */
	ctx.push(js::Object{});
	ctx.push(methods);
	ctx.putGlobal("\xff""\xff""Server-proto");

	/* Server object */
	ctx.getGlobal<void>("Irccd");
	ctx.push(js::Function{constructor, 1});
	ctx.push(functions);

	/* Prototype */
	ctx.getGlobal<void>("\xff""\xff""Server-proto");
	ctx.push(methods);
	ctx.putProperty(-1, "\xff""\xff""Server", true);
	ctx.putProperty(-2, "prototype");

	/* Put Server */
	ctx.putProperty(-2, "Server");
	ctx.pop();
}

} // !irccd
