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

#include <cassert>
#include <sstream>
#include <unordered_map>

#include "irccd.hpp"
#include "mod-irccd.hpp"
#include "mod-server.hpp"
#include "plugin-js.hpp"
#include "server.hpp"
#include "service-server.hpp"

namespace irccd {

namespace {

const char *Signature("\xff""\xff""irccd-server-ptr");
const char *Prototype("\xff""\xff""irccd-server-prototype");

std::shared_ptr<Server> self(duk_context *ctx)
{
	StackAssert sa(ctx);

	duk_push_this(ctx);
	duk_get_prop_string(ctx, -1, Signature);
	auto ptr = duk_to_pointer(ctx, -1);
	duk_pop_2(ctx);

	if (!ptr)
		duk_error(ctx, DUK_ERR_TYPE_ERROR, "not a Server object");

	return *static_cast<std::shared_ptr<Server> *>(ptr);
}

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
duk_ret_t cmode(duk_context *ctx)
{
	self(ctx)->cmode(duk_require_string(ctx, 0), duk_require_string(ctx, 1));

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
duk_ret_t cnotice(duk_context *ctx)
{
	self(ctx)->cnotice(duk_require_string(ctx, 0), duk_require_string(ctx, 1));

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
duk_ret_t info(duk_context *ctx)
{
	auto server = self(ctx);

	duk_push_object(ctx);
	duk_push_stdstring(ctx, server->name());
	duk_put_prop_string(ctx, -2, "name");
	duk_push_stdstring(ctx, server->info().host);
	duk_put_prop_string(ctx, -2, "host");
	duk_push_int(ctx, server->info().port);
	duk_put_prop_string(ctx, -2, "port");
	duk_push_boolean(ctx, server->info().flags & ServerInfo::Ssl);
	duk_put_prop_string(ctx, -2, "ssl");
	duk_push_boolean(ctx, server->info().flags & ServerInfo::SslVerify);
	duk_put_prop_string(ctx, -2, "sslVerify");
	duk_push_stdstring(ctx, server->settings().command);
	duk_put_prop_string(ctx, -2, "commandChar");
	duk_push_stdstring(ctx, server->identity().realname);
	duk_put_prop_string(ctx, -2, "realname");
	duk_push_stdstring(ctx, server->identity().nickname);
	duk_put_prop_string(ctx, -2, "nickname");
	duk_push_stdstring(ctx, server->identity().username);
	duk_put_prop_string(ctx, -2, "username");

	// Channels.
	duk_push_array(ctx);

	int i = 0;
	for (const auto &channel : server->settings().channels) {
		duk_push_stdstring(ctx, channel.name);
		duk_put_prop_index(ctx, -1, i++);
	}

	duk_put_prop_string(ctx, -2, "channels");

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
duk_ret_t invite(duk_context *ctx)
{
	self(ctx)->invite(duk_require_string(ctx, 0), duk_require_string(ctx, 1));

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
duk_ret_t join(duk_context *ctx)
{
	self(ctx)->join(duk_require_string(ctx, 0), duk_get_string(ctx, 1));

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
duk_ret_t kick(duk_context *ctx)
{
	self(ctx)->kick(duk_require_string(ctx, 0), duk_require_string(ctx, 1), duk_get_string(ctx, 2));

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
duk_ret_t me(duk_context *ctx)
{
	self(ctx)->me(duk_require_string(ctx, 0), duk_require_string(ctx, 1));

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
duk_ret_t message(duk_context *ctx)
{
	self(ctx)->message(duk_require_string(ctx, 0), duk_require_string(ctx, 1));

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
duk_ret_t mode(duk_context *ctx)
{
	self(ctx)->mode(duk_require_string(ctx, 0));

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
duk_ret_t names(duk_context *ctx)
{
	self(ctx)->names(duk_require_string(ctx, 0));

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
duk_ret_t nick(duk_context *ctx)
{
	self(ctx)->nick(duk_require_string(ctx, 0));

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
duk_ret_t notice(duk_context *ctx)
{
	self(ctx)->notice(duk_require_string(ctx, 0), duk_require_string(ctx, 1));

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
duk_ret_t part(duk_context *ctx)
{
	self(ctx)->part(duk_require_string(ctx, 0), duk_get_string(ctx, 1));

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
duk_ret_t send(duk_context *ctx)
{
	self(ctx)->send(duk_require_string(ctx, 0));

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
duk_ret_t topic(duk_context *ctx)
{
	self(ctx)->topic(duk_require_string(ctx, 0), duk_require_string(ctx, 1));

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
duk_ret_t whois(duk_context *ctx)
{
	self(ctx)->whois(duk_require_string(ctx, 0));

	return 0;
}

/*
 * Method: Server.toString()
 * ------------------------------------------------------------------
 *
 * Convert the object to std::string, convenience for adding the object
 * as property key.
 *
 * duk_ret_turns:
 *   The server name (unique).
 */
duk_ret_t toString(duk_context *ctx)
{
	duk_push_stdstring(ctx, self(ctx)->name());

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
duk_ret_t constructor(duk_context *ctx)
{
#if 0
	if (!isConstructorCall(ctx))
		return 0;

	std::string name;
	ServerInfo info;
	ServerIdentity identity;
	ServerSettings settings;

	// Information part.
	name = getProperty<std::string>(ctx, 0, "name");
	info.host = getProperty<std::string>(ctx, 0, "host");
	info.port = optionalProperty<int>(ctx, 0, "port", static_cast<int>(info.port));
	info.password = optionalProperty<std::string>(ctx, 0, "password", "");

	if (optionalProperty<bool>(ctx, 0, "ipv6", false))
		info.flags |= ServerInfo::Ipv6;

	// Identity part.
	identity.nickname = optionalProperty<std::string>(ctx, 0, "nickname", identity.nickname);
	identity.username = optionalProperty<std::string>(ctx, 0, "username", identity.username);
	identity.realname = optionalProperty<std::string>(ctx, 0, "realname", identity.realname);
	identity.ctcpversion = optionalProperty<std::string>(ctx, 0, "version", identity.ctcpversion);

	// Settings part.
	for (const auto &chan: getProperty<std::vector<std::string>>(ctx, 0, "channels"))
		settings.channels.push_back(Server::splitChannel(chan));

	settings.reconnectTries = optionalProperty<int>(ctx, 0, "recoTries", (int)settings.reconnectTries);
	settings.reconnectDelay = optionalProperty<int>(ctx, 0, "recoTimeout", (int)settings.reconnectDelay);

	if (optionalProperty<bool>(ctx, 0, "joinInvite", false))
		settings.flags |= ServerSettings::JoinInvite;
	if (optionalProperty<bool>(ctx, 0, "autoRejoin", false))
		settings.flags |= ServerSettings::AutoRejoin;

	try {
		push(ctx, This());
		pushPointer(ctx, new std::shared_ptr<Server>(std::move(name), std::move(info), std::move(identity), std::move(settings)));
		putProperty(ctx, -2, Signature);
		pop(ctx);
	} catch (const std::exception &ex) {
		raise(ctx, Error(ex.what()));
	}
#endif

	return 0;
}

/*
 * Function: Irccd.Server() [destructor]
 * ------------------------------------------------------------------
 *
 * Delete the property.
 */
duk_ret_t destructor(duk_context *ctx)
{
	duk_get_prop_string(ctx, 0, Signature);
	delete static_cast<std::shared_ptr<Server> *>(duk_to_pointer(ctx, -1));
	duk_pop(ctx);
	duk_del_prop_string(ctx, 0, Signature);

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
duk_ret_t add(duk_context *ctx)
{
	duk_get_irccd(ctx).serverService().add(duk_require_server(ctx, 0));

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
 * duk_ret_turns:
 *   The server object or undefined if not found.
 */
duk_ret_t find(duk_context *ctx)
{
	auto server = duk_get_irccd(ctx).serverService().get(duk_require_string(ctx, 0));

	if (!server)
		return 0;

	duk_push_server(ctx, server);

	return 1;
}

/*
 * Function: Irccd.Server.list()
 * ------------------------------------------------------------------
 *
 * Get the map of all loaded servers.
 *
 * duk_ret_turns:
 *   An object with string-to-servers pairs.
 */
duk_ret_t list(duk_context *ctx)
{
	duk_push_object(ctx);

	for (const auto &server : duk_get_irccd(ctx).serverService().servers()) {
		duk_push_server(ctx, server);
		duk_put_prop_string(ctx, -2, server->name().c_str());
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
duk_ret_t remove(duk_context *ctx)
{
	duk_get_irccd(ctx).serverService().remove(duk_require_string(ctx, 0));

	return 0;
}

const duk_function_list_entry methods[] = {
	{ "cmode",	cmode,		2		},
	{ "cnotice",	cnotice,	2		},
	{ "info",	info,		0		},
	{ "invite",	invite,		2		},
	{ "join",	join,		DUK_VARARGS	},
	{ "kick",	kick,		DUK_VARARGS	},
	{ "me",		me,		2		},
	{ "message",	message,	2		},
	{ "mode",	mode,		1		},
	{ "names",	names,		1		},
	{ "nick",	nick,		1		},
	{ "notice",	notice,		2		},
	{ "part",	part,		DUK_VARARGS	},
	{ "send",	send,		1		},
	{ "topic",	topic,		2		},
	{ "whois",	whois,		1		},
	{ "toString",	toString,	0		},
	{ nullptr,	nullptr,	0		}
};

const duk_function_list_entry functions[] = {
	{ "add",	add,		1		},
	{ "find",	find,		1		},
	{ "list",	list,		0		},
	{ "remove",	remove,		1		},
	{ nullptr,	nullptr,	0		}
};

} // !namespace

ServerModule::ServerModule() noexcept
	: Module("Irccd.Server")
{
}

void ServerModule::load(Irccd &, JsPlugin &plugin)
{
	StackAssert sa(plugin.context());

	duk_get_global_string(plugin.context(), "Irccd");
	duk_push_c_function(plugin.context(), constructor, 1);
	duk_put_function_list(plugin.context(), -1, functions);
	duk_push_object(plugin.context());
	duk_put_function_list(plugin.context(), -1, methods);
	duk_push_c_function(plugin.context(), destructor, 1);
	duk_set_finalizer(plugin.context(), -2);
	duk_dup_top(plugin.context());
	duk_get_global_string(plugin.context(), Prototype);
	duk_put_prop_string(plugin.context(), -2, "prototype");
	duk_put_prop_string(plugin.context(), -2, "Server");
	duk_pop(plugin.context());
}

void duk_push_server(duk_context *ctx, std::shared_ptr<Server> server)
{
	assert(ctx);
	assert(server);

	StackAssert sa(ctx, 1);

	duk_push_object(ctx);
	duk_push_pointer(ctx, new std::shared_ptr<Server>(std::move(server)));
	duk_put_prop_string(ctx, -2, Signature);
	duk_get_global_string(ctx, Prototype);
	duk_set_finalizer(ctx, -2);
}

std::shared_ptr<Server> require(duk_context *ctx, duk_idx_t index)
{
	if (!duk_is_object(ctx, index) || !duk_has_prop_string(ctx, index, Signature))
		duk_error(ctx, DUK_ERR_TYPE_ERROR, "not a Server object");

	duk_get_prop_string(ctx, index, Signature);
	auto file = *static_cast<std::shared_ptr<Server> *>(duk_to_pointer(ctx, -1));
	duk_pop(ctx);
	
	return file;
}

} // !irccd
