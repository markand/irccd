/*
 * jsapi-server.c -- Irccd.Server API
 *
 * Copyright (c) 2013-2021 David Demelier <markand@malikania.fr>
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

#include <assert.h>

#include <duktape.h>

#include "channel.h"
#include "irccd.h"
#include "server.h"

#define SIGNATURE DUK_HIDDEN_SYMBOL("Irccd.Server")
#define PROTOTYPE DUK_HIDDEN_SYMBOL("Irccd.Server.prototype")

static struct irc_server *
self(duk_context *ctx)
{
	/*
	 * Server are stored using their identifiers and searched in the
	 * registry as they may be removed at runtime.
	 */
	struct irc_server *sv;

	duk_push_this(ctx);
	duk_get_prop_string(ctx, -1, SIGNATURE);
	sv = irc_find_server(duk_to_string(ctx, -1));
	duk_pop_2(ctx);

	if (!sv)
		duk_error(ctx, DUK_ERR_TYPE_ERROR, "not a Server object");

	return sv;
}

static struct irc_server *
require(duk_context *ctx, duk_idx_t index)
{
	struct irc_server *sv;

	if (!duk_is_object(ctx, index) || !duk_has_prop_string(ctx, index, SIGNATURE))
		duk_error(ctx, DUK_ERR_TYPE_ERROR, "not a Server object");

	duk_get_prop_string(ctx, index, SIGNATURE);
	sv = irc_find_server(duk_to_string(ctx, -1));
	duk_pop(ctx);

	return sv;
}

static duk_ret_t
Server_prototype_info(duk_context *ctx)
{
	const struct irc_server *s = self(ctx);

	duk_push_object(ctx);
	duk_push_string(ctx, s->name);
	duk_put_prop_string(ctx, -2, "name");
	duk_push_string(ctx, s->hostname);
	duk_put_prop_string(ctx, -2, "hostname");
	duk_push_uint(ctx, s->port);
	duk_put_prop_string(ctx, -2, "port");
	duk_push_boolean(ctx, s->flags & IRC_SERVER_FLAGS_SSL);
	duk_put_prop_string(ctx, -2, "ssl");
	duk_push_string(ctx, s->commandchar);
	duk_put_prop_string(ctx, -2, "commandChar");
	duk_push_string(ctx, s->realname);
	duk_put_prop_string(ctx, -2, "realname");
	duk_push_string(ctx, s->nickname);
	duk_put_prop_string(ctx, -2, "nickname");
	duk_push_string(ctx, s->username);
	duk_put_prop_string(ctx, -2, "username");

	duk_push_array(ctx);

	for (size_t i = 0; i < s->channelsz; ++i) {
		duk_push_string(ctx, s->channels[i].name);
		duk_put_prop_index(ctx, -2, i);
	}

	duk_put_prop_string(ctx, -2, "channels");

	return 1;
}

static duk_ret_t
Server_prototype_invite(duk_context *ctx)
{
	struct irc_server *s = self(ctx);
	const char *target = duk_require_string(ctx, 0);
	const char *channel = duk_require_string(ctx, 1);

#if 0
	if (!*target || !*channel)
		throw server_error(server_error::invalid_nickname);
	if (channel.empty())
		throw server_error(server_error::invalid_channel);
#endif

	duk_push_boolean(ctx, irc_server_invite(s, target, channel));

	return 1;
}

static duk_ret_t
Server_prototype_isSelf(duk_context *ctx)
{
#if 0
	return wrap(ctx, [] (auto ctx) {
		return duk::push(ctx, self(ctx)->is_self(duk::require<std::string>(ctx, 0)));
	});
#endif

	return 0;
}

static duk_ret_t
Server_prototype_join(duk_context *ctx)
{
	struct irc_server *s = self(ctx);
	const char *channel = duk_require_string(ctx, 0);
	const char *password = duk_opt_string(ctx, 1, NULL);

#if 0
	if (channel.empty())
		throw server_error(server_error::invalid_channel);
#endif

	duk_push_boolean(ctx, irc_server_join(s, channel, password));

	return 1;
}

static duk_ret_t
Server_prototype_kick(duk_context *ctx)
{
	struct irc_server *s = self(ctx);
	const char *target = duk_require_string(ctx, 0);
	const char *channel = duk_require_string(ctx, 1);
	const char *reason = duk_opt_string(ctx, 2, NULL);

#if 0
	if (target.empty())
		throw server_error(server_error::invalid_nickname);
	if (channel.empty())
		throw server_error(server_error::invalid_channel);
#endif

	duk_push_boolean(ctx, irc_server_kick(s, target, channel, reason));

	return 1;
}

static duk_ret_t
Server_prototype_me(duk_context *ctx)
{
	struct irc_server *s = self(ctx);
	const char *target = duk_require_string(ctx, 0);
	const char *message = duk_require_string(ctx, 1);

#if 0
	if (target.empty())
		throw server_error(server_error::invalid_nickname);
#endif

	duk_push_boolean(ctx, irc_server_me(s, target, message));

	return 1;
}

static duk_ret_t
Server_prototype_message(duk_context *ctx)
{
	struct irc_server *s = self(ctx);
	const char *target = duk_require_string(ctx, 0);
	const char *message = duk_require_string(ctx, 1);

#if 0
	if (target.empty())
		throw server_error(server_error::invalid_nickname);
#endif

	duk_push_boolean(ctx, irc_server_message(s, target, message));

	return 1;
}

static duk_ret_t
Server_prototype_mode(duk_context *ctx)
{
	struct irc_server *s = self(ctx);
	const char *channel = duk_require_string(ctx, 0);
	const char *mode = duk_require_string(ctx, 1);
	const char *limit = duk_opt_string(ctx, 2, NULL);
	const char *user = duk_opt_string(ctx, 3, NULL);
	const char *mask = duk_opt_string(ctx, 4, NULL);

#if 0
	if (channel.empty())
		throw server_error(server_error::invalid_channel);
	if (mode.empty())
		throw server_error(server_error::invalid_mode);
#endif

	duk_push_boolean(ctx, irc_server_mode(s, channel, mode, limit, user, mask));

	return 1;
}

static duk_ret_t
Server_prototype_names(duk_context *ctx)
{
	struct irc_server *s = self(ctx);
	const char *channel = duk_require_string(ctx, 0);

#if 0
	if (channel.empty())
		throw server_error(server_error::invalid_channel);
#endif

#if 0
	duk_push_boolean(ctx, irc_server_names(channel));
#endif

	return 1;
}

static duk_ret_t
Server_prototype_nick(duk_context *ctx)
{
	struct irc_server *s = self(ctx);
	const char *nickname = duk_require_string(ctx, 0);

#if 0
	if (nickname.empty())
		throw server_error(server_error::invalid_nickname);
#endif

#if 0
	duk_push_boolean(set_nickname(std::move(nickname));
#endif

	return 0;
}

static duk_ret_t
Server_prototype_notice(duk_context *ctx)
{
	struct irc_server *s = self(ctx);
	const char *target = duk_require_string(ctx, 0);
	const char *message = duk_opt_string(ctx, 1, NULL);

#if 0
	if (target.empty())
		throw server_error(server_error::invalid_nickname);
#endif

	duk_push_boolean(ctx, irc_server_notice(s, target, message));

	return 1;
}

static duk_ret_t
Server_prototype_part(duk_context *ctx)
{
	struct irc_server *s = self(ctx);
	const char *channel = duk_require_string(ctx, 0);
	const char *reason = duk_opt_string(ctx, 1, NULL);

#if 0
	if (channel.empty())
		throw server_error(server_error::invalid_channel);
#endif

	duk_push_boolean(ctx, irc_server_part(s, channel, reason));

	return 1;
}

static duk_ret_t
Server_prototype_send(duk_context *ctx)
{
	struct irc_server *s = self(ctx);
	const char *raw = duk_require_string(ctx, 0);

#if 0
	if (raw.empty())
		throw server_error(server_error::invalid_message);
#endif

	duk_push_boolean(ctx, irc_server_send(s, raw));

	return 1;
}

static duk_ret_t
Server_prototype_topic(duk_context *ctx)
{
	struct irc_server *s = self(ctx);
	const char *channel = duk_require_string(ctx, 0);
	const char *topic = duk_require_string(ctx, 1);

#if 0
	if (channel.empty())
		throw server_error(server_error::invalid_channel);
#endif

	duk_push_boolean(ctx, irc_server_topic(s, channel, topic));

	return 1;
}

static duk_ret_t
Server_prototype_whois(duk_context *ctx)
{
	struct irc_server *s = self(ctx);
	const char *target = duk_require_string(ctx, 0);

#if 0
	if (target.empty())
		throw server_error(server_error::invalid_nickname);
#endif

#if 0
	duk_push_boolean(ctx, irc_server_whois(s, target));
#endif

	return 1;
}

static duk_ret_t
Server_prototype_toString(duk_context *ctx)
{
	duk_push_string(ctx, self(ctx)->name);

	return 1;
}

static duk_ret_t
Server_constructor(duk_context *ctx)
{
#if 0
	return wrap(ctx, [] (auto ctx) {
		if (!duk_is_constructor_call(ctx))
			return 0;

		duk_check_type(ctx, 0, DUK_TYPE_OBJECT);

		auto json = nlohmann::json::parse(duk_json_encode(ctx, 0));
		auto s = from_json(duk::type_traits<bot>::self(ctx).get_service(), json);

		duk_push_this(ctx);
		duk_push_pointer(ctx, new std::shared_ptr<server>(std::move(s)));
		duk_put_prop_string(ctx, -2, signature.data());
		duk_pop(ctx);

		return 0;
	});
#endif
	return 0;
}

#if 0

static duk_ret_t
Server_add(duk_context *ctx)
{
	return wrap(ctx, [] (auto ctx) {
		duk::type_traits<bot>::self(ctx).get_servers().add(
			duk::require<std::shared_ptr<server>>(ctx, 0));

		return 0;
	});
}

#endif

#if 0

static duk_ret_t
Server_find(duk_context *ctx)
{
	return wrap(ctx, [] (auto ctx) {
		auto id = duk::require<std::string>(ctx, 0);
		auto server = duk::type_traits<bot>::self(ctx).get_servers().get(id);

		if (!server)
			return 0;

		duk::push(ctx, server);

		return 1;
	});
}

#endif

#if 0

static duk_ret_t
Server_list(duk_context *ctx)
{
	duk_push_object(ctx);

	for (const auto& server : duk::type_traits<bot>::self(ctx).get_servers().list()) {
		duk::push(ctx, server);
		duk_put_prop_string(ctx, -2, server->get_id().c_str());
	}

	return 1;
}

#endif

#if 0

static duk_ret_t
Server_remove(duk_context *ctx)
{
	duk::type_traits<bot>::self(ctx).get_servers().remove(duk_require_string(ctx, 0));

	return 0;
}

#endif

static const duk_function_list_entry methods[] = {
	{ "info",       Server_prototype_info,          0               },
	{ "invite",     Server_prototype_invite,        2               },
	{ "isSelf",     Server_prototype_isSelf,        1               },
	{ "join",       Server_prototype_join,          DUK_VARARGS     },
	{ "kick",       Server_prototype_kick,          DUK_VARARGS     },
	{ "me",         Server_prototype_me,            2               },
	{ "message",    Server_prototype_message,       2               },
	{ "mode",       Server_prototype_mode,          1               },
	{ "names",      Server_prototype_names,         1               },
	{ "nick",       Server_prototype_nick,          1               },
	{ "notice",     Server_prototype_notice,        2               },
	{ "part",       Server_prototype_part,          DUK_VARARGS     },
	{ "send",       Server_prototype_send,          1               },
	{ "topic",      Server_prototype_topic,         2               },
	{ "toString",   Server_prototype_toString,      0               },
	{ "whois",      Server_prototype_whois,         1               },
	{ NULL,         NULL,                           0               }
};

static const duk_function_list_entry functions[] = {
#if 0
	{ "add",        Server_add,                     1               },
	{ "find",       Server_find,                    1               },
	{ "list",       Server_list,                    0               },
	{ "remove",     Server_remove,                  1               },
#endif
	{ NULL,         NULL,                           0               }
};

void
irc_jsapi_server_load(duk_context *ctx)
{
	assert(ctx);

	duk_get_global_string(ctx, "Irccd");

	duk_push_c_function(ctx, Server_constructor, 1);
	duk_put_function_list(ctx, -1, functions);
	duk_push_object(ctx);
	duk_put_function_list(ctx, -1, methods);
	duk_dup_top(ctx);
	duk_put_global_string(ctx, PROTOTYPE);
	duk_put_prop_string(ctx, -2, "prototype");
	duk_put_prop_string(ctx, -2, "Server");
	duk_pop(ctx);
}

void
irc_jsapi_server_push(duk_context *ctx, struct irc_server *s)
{
	assert(ctx);
	assert(s);

	duk_push_object(ctx);
	duk_push_string(ctx, s->name);
	duk_put_prop_string(ctx, -2, SIGNATURE);
	duk_get_global_string(ctx, PROTOTYPE);
	duk_set_prototype(ctx, -2);
}
