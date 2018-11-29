/*
 * server_api.cpp -- Irccd.Server API
 *
 * Copyright (c) 2013-2018 David Demelier <markand@malikania.fr>
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

#include <irccd/daemon/bot.hpp>
#include <irccd/daemon/server_service.hpp>
#include <irccd/daemon/server_util.hpp>

#include "irccd_api.hpp"
#include "plugin.hpp"
#include "server_api.hpp"

using irccd::daemon::bot;
using irccd::daemon::server;
using irccd::daemon::server_error;
using irccd::daemon::server_util::from_json;

namespace irccd::js {

namespace {

const std::string_view signature(DUK_HIDDEN_SYMBOL("Irccd.Server"));
const std::string_view prototype(DUK_HIDDEN_SYMBOL("Irccd.Server.prototype"));

auto self(duk_context* ctx) -> std::shared_ptr<server>
{
	duk::stack_guard sa(ctx);

	duk_push_this(ctx);
	duk_get_prop_string(ctx, -1, signature.data());
	auto ptr = duk_to_pointer(ctx, -1);
	duk_pop_2(ctx);

	if (!ptr)
		duk_error(ctx, DUK_ERR_TYPE_ERROR, "not a Server object");

	return *static_cast<std::shared_ptr<server>*>(ptr);
}

template <typename Handler>
auto wrap(duk_context* ctx, Handler handler) -> duk_ret_t
{
	try {
		return handler(ctx);
	} catch (const server_error& ex) {
		duk::raise(ctx, ex);
	} catch (const std::exception& ex) {
		duk::raise(ctx, ex);
	}

	return 0;
}

// {{{ Irccd.Server.prototype.info

/*
 * Method: Irccd.Server.prototype.info()
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
auto Server_prototype_info(duk_context* ctx) -> duk_ret_t
{
	const auto server = self(ctx);
	const auto& channels = server->get_channels();

	duk_push_object(ctx);
	duk::push(ctx, server->get_id());
	duk_put_prop_string(ctx, -2, "name");
	duk::push(ctx, server->get_hostname());
	duk_put_prop_string(ctx, -2, "hostname");
	duk_push_int(ctx, server->get_port());
	duk_put_prop_string(ctx, -2, "port");
	duk_push_boolean(ctx, (server->get_options() & server::options::ssl) == server::options::ssl);
	duk_put_prop_string(ctx, -2, "ssl");
	duk::push(ctx, server->get_command_char());
	duk_put_prop_string(ctx, -2, "commandChar");
	duk::push(ctx, server->get_realname());
	duk_put_prop_string(ctx, -2, "realname");
	duk::push(ctx, server->get_nickname());
	duk_put_prop_string(ctx, -2, "nickname");
	duk::push(ctx, server->get_username());
	duk_put_prop_string(ctx, -2, "username");
	duk::push(ctx, std::vector<std::string>(channels.begin(), channels.end()));
	duk_put_prop_string(ctx, -2, "channels");

	return 1;
}

// }}}

// {{{ Irccd.Server.prototype.invite

/*
 * Method: Irccd.Server.prototype.invite(target, channel)
 * ------------------------------------------------------------------
 *
 * Invite someone to a channel.
 *
 * Arguments:
 *   - target, the target to invite,
 *   - channel, the channel.
 * Throws:
 *   - Irccd.ServerError on server related errors,
 *   - Irccd.SystemError on other errors.
 */
auto Server_prototype_invite(duk_context* ctx) -> duk_ret_t
{
	return wrap(ctx, [] (auto ctx) {
		auto target = duk::require<std::string>(ctx, 0);
		auto channel = duk::require<std::string>(ctx, 1);

		if (target.empty())
			throw server_error(server_error::invalid_nickname);
		if (channel.empty())
			throw server_error(server_error::invalid_channel);

		self(ctx)->invite(std::move(target), std::move(channel));

		return 0;
	});
}

// }}}

// {{{ Irccd.Server.prototype.isSelf

/*
 * Method: Irccd.Server.prototype.isSelf(nickname)
 * ------------------------------------------------------------------
 *
 * Arguments:
 *   - nickname, the nickname to check.
 * Returns:
 *   True if the nickname targets this server.
 * Throws:
 *   - Irccd.SystemError on errors.
 */
auto Server_prototype_isSelf(duk_context* ctx) -> duk_ret_t
{
	return wrap(ctx, [] (auto ctx) {
		return duk::push(ctx, self(ctx)->is_self(duk::require<std::string>(ctx, 0)));
	});
}

// }}}

// {{{ Irccd.Server.prototype.join

/*
 * Method: Irccd.Server.prototype.join(channel, password = undefined)
 * ------------------------------------------------------------------
 *
 * Join a channel with an optional password.
 *
 * Arguments:
 *   - channel, the channel to join,
 *   - password, the password or undefined to not use.
 * Throws:
 *   - Irccd.ServerError on server related errors,
 *   - Irccd.SystemError on other errors.
 */
auto Server_prototype_join(duk_context* ctx) -> duk_ret_t
{
	return wrap(ctx, [] (auto ctx) {
		auto channel = duk::require<std::string>(ctx, 0);
		auto password = duk::get<std::string>(ctx, 1);

		if (channel.empty())
			throw server_error(server_error::invalid_channel);

		self(ctx)->join(std::move(channel), std::move(password));

		return 0;
	});
}

// }}}

// {{{ Irccd.Server.prototype.kick

/*
 * Method: Irccd.Server.prototype.kick(target, channel, reason = undefined)
 * ------------------------------------------------------------------
 *
 * Kick someone from a channel.
 *
 * Arguments:
 *   - target, the target to kick,
 *   - channel, the channel,
 *   - reason, the optional reason or undefined to not set.
 * Throws:
 *   - Irccd.ServerError on server related errors,
 *   - Irccd.SystemError on other errors.
 */
auto Server_prototype_kick(duk_context* ctx) -> duk_ret_t
{
	return wrap(ctx, [] (auto ctx) {
		auto target = duk::require<std::string>(ctx, 0);
		auto channel = duk::require<std::string>(ctx, 1);
		auto reason = duk::get<std::string>(ctx, 2);

		if (target.empty())
			throw server_error(server_error::invalid_nickname);
		if (channel.empty())
			throw server_error(server_error::invalid_channel);

		self(ctx)->kick(std::move(target), std::move(channel), std::move(reason));

		return 0;
	});
}

// }}}

// {{{ Irccd.Server.prototype.me

/*
 * Method: Irccd.Server.prototype.me(target, message)
 * ------------------------------------------------------------------
 *
 * Send a CTCP Action.
 *
 * Arguments:
 *   - target, the target or a channel,
 *   - message, the message.
 * Throws:
 *   - Irccd.ServerError on server related errors,
 *   - Irccd.SystemError on other errors.
 */
auto Server_prototype_me(duk_context* ctx) -> duk_ret_t
{
	return wrap(ctx, [] (auto ctx) {
		auto target = duk::require<std::string>(ctx, 0);
		auto message = duk::get<std::string>(ctx, 1);

		if (target.empty())
			throw server_error(server_error::invalid_nickname);

		self(ctx)->me(std::move(target), std::move(message));

		return 0;
	});
}

// }}}

// {{{ Irccd.Server.prototype.message

/*
 * Method: Irccd.Server.prototype.message(target, message)
 * ------------------------------------------------------------------
 *
 * Send a message.
 *
 * Arguments:
 *   - target, the target or a channel,
 *   - message, the message.
 * Throws:
 *   - Irccd.ServerError on server related errors,
 *   - Irccd.SystemError on other errors.
 */
auto Server_prototype_message(duk_context* ctx) -> duk_ret_t
{
	return wrap(ctx, [] (auto ctx) {
		auto target = duk::require<std::string>(ctx, 0);
		auto message = duk::get<std::string>(ctx, 1);

		if (target.empty())
			throw server_error(server_error::invalid_nickname);

		self(ctx)->message(std::move(target), std::move(message));

		return 0;
	});
}

// }}}

// {{{ Irccd.Server.prototype.mode

/*
 * Method: Irccd.Server.prototype.mode(channel, mode, limit, user, mask)
 * ------------------------------------------------------------------
 *
 * Change your mode.
 *
 * Arguments:
 *   - mode, the new mode.
 * Throws:
 *   - Irccd.ServerError on server related errors,
 *   - Irccd.SystemError on other errors.
 */
auto Server_prototype_mode(duk_context* ctx) -> duk_ret_t
{
	return wrap(ctx, [] (auto ctx) {
		auto channel = duk::require<std::string>(ctx, 0);
		auto mode = duk::require<std::string>(ctx, 1);
		auto limit = duk::get<std::string>(ctx, 2);
		auto user = duk::get<std::string>(ctx, 3);
		auto mask = duk::get<std::string>(ctx, 4);

		if (channel.empty())
			throw server_error(server_error::invalid_channel);
		if (mode.empty())
			throw server_error(server_error::invalid_mode);

		self(ctx)->mode(
			std::move(channel),
			std::move(mode),
			std::move(limit),
			std::move(user),
			std::move(mask)
		);

		return 0;
	});
}

// }}}

// {{{ Irccd.Server.prototype.names

/*
 * Method: Irccd.Server.prototype.names(channel)
 * ------------------------------------------------------------------
 *
 * Get the list of names from a channel.
 *
 * Arguments:
 *   - channel, the channel.
 * Throws:
 *   - Irccd.ServerError on server related errors,
 *   - Irccd.SystemError on other errors.
 */
auto Server_prototype_names(duk_context* ctx) -> duk_ret_t
{
	return wrap(ctx, [] (auto ctx) {
		auto channel = duk::require<std::string>(ctx, 0);

		if (channel.empty())
			throw server_error(server_error::invalid_channel);

		self(ctx)->names(std::move(channel));

		return 0;
	});
}

// }}}

// {{{ Irccd.Server.prototype.nick

/*
 * Method: Irccd.Server.prototype.nick(nickname)
 * ------------------------------------------------------------------
 *
 * Change the nickname.
 *
 * Arguments:
 *   - nickname, the nickname.
 * Throws:
 *   - Irccd.ServerError on server related errors,
 *   - Irccd.SystemError on other errors.
 */
auto Server_prototype_nick(duk_context* ctx) -> duk_ret_t
{
	return wrap(ctx, [] (auto ctx) {
		auto nickname = duk::require<std::string>(ctx, 0);

		if (nickname.empty())
			throw server_error(server_error::invalid_nickname);

		self(ctx)->set_nickname(std::move(nickname));

		return 0;
	});
}

// }}}

// {{{ Irccd.Server.prototype.notice

/*
 * Method: Irccd.Server.prototype.notice(target, message)
 * ------------------------------------------------------------------
 *
 * Send a private notice.
 *
 * Arguments:
 *   - target, the target,
 *   - message, the notice message.
 * Throws:
 *   - Irccd.ServerError on server related errors,
 *   - Irccd.SystemError on other errors.
 */
auto Server_prototype_notice(duk_context* ctx) -> duk_ret_t
{
	return wrap(ctx, [] (auto ctx) {
		auto target = duk::require<std::string>(ctx, 0);
		auto message = duk::get<std::string>(ctx, 1);

		if (target.empty())
			throw server_error(server_error::invalid_nickname);

		self(ctx)->notice(std::move(target), std::move(message));

		return 0;
	});
}

// }}}

// {{{ Irccd.Server.prototype.part

/*
 * Method: Irccd.Server.prototype.part(channel, reason = undefined)
 * ------------------------------------------------------------------
 *
 * Leave a channel.
 *
 * Arguments:
 *   - channel, the channel to leave,
 *   - reason, the optional reason, keep undefined for portability.
 * Throws:
 *   - Irccd.ServerError on server related errors,
 *   - Irccd.SystemError on other errors.
 */
auto Server_prototype_part(duk_context* ctx) -> duk_ret_t
{
	return wrap(ctx, [] (auto ctx) {
		auto channel = duk::require<std::string>(ctx, 0);
		auto reason = duk::get<std::string>(ctx, 1);

		if (channel.empty())
			throw server_error(server_error::invalid_channel);

		self(ctx)->part(std::move(channel), std::move(reason));

		return 0;
	});
}

// }}}

// {{{ Irccd.Server.prototype.send

/*
 * Method: Irccd.Server.prototype.send(raw)
 * ------------------------------------------------------------------
 *
 * Send a raw message to the IRC server.
 *
 * Arguments:
 *   - raw, the raw message (without terminators).
 * Throws:
 *   - Irccd.ServerError on server related errors,
 *   - Irccd.SystemError on other errors.
 */
auto Server_prototype_send(duk_context* ctx) -> duk_ret_t
{
	return wrap(ctx, [] (auto ctx) {
		auto raw = duk::require<std::string>(ctx, 0);

		if (raw.empty())
			throw server_error(server_error::invalid_message);

		self(ctx)->send(std::move(raw));

		return 0;
	});
}

// }}}

// {{{ Irccd.Server.prototype.topic

/*
 * Method: Server.prototype.topic(channel, topic)
 * ------------------------------------------------------------------
 *
 * Change a channel topic.
 *
 * Arguments:
 *   - channel, the channel,
 *   - topic, the new topic.
 * Throws:
 *   - Irccd.ServerError on server related errors,
 *   - Irccd.SystemError on other errors.
 */
auto Server_prototype_topic(duk_context* ctx) -> duk_ret_t
{
	return wrap(ctx, [] (auto ctx) {
		auto channel = duk::require<std::string>(ctx, 0);
		auto topic = duk::get<std::string>(ctx, 1);

		if (channel.empty())
			throw server_error(server_error::invalid_channel);

		self(ctx)->topic(std::move(channel), std::move(topic));

		return 0;
	});
}

// }}}

// {{{ Irccd.Server.prototype.whois

/*
 * Method: Irccd.Server.prototype.whois(target)
 * ------------------------------------------------------------------
 *
 * Get whois information.
 *
 * Arguments:
 *   - target, the target.
 * Throws:
 *   - Irccd.ServerError on server related errors,
 *   - Irccd.SystemError on other errors.
 */
auto Server_prototype_whois(duk_context* ctx) -> duk_ret_t
{
	return wrap(ctx, [] (auto ctx) {
		auto target = duk::require<std::string>(ctx, 0);

		if (target.empty())
			throw server_error(server_error::invalid_nickname);

		self(ctx)->whois(std::move(target));

		return 0;
	});
}

// }}}

// {{{ Irccd.Server.prototype.toString

/*
 * Method: Irccd.Server.prototype.toString()
 * ------------------------------------------------------------------
 *
 * Convert the object to std::string, convenience for adding the object
 * as property key.
 *
 * Returns:
 *   The server name (unique).
 * Throws:
 *   - Irccd.SystemError on errors.
 */
auto Server_prototype_toString(duk_context* ctx) -> duk_ret_t
{
	return wrap(ctx, [] (auto ctx) {
		duk::push(ctx, self(ctx)->get_id());

		return 1;
	});
}

// }}}

// {{{ Irccd.Server [constructor]

/*
 * Function: Irccd.Server(params) [constructor]
 * ------------------------------------------------------------------
 *
 * Construct a new server.
 *
 * Params must be filled with the following properties:
 *
 * name: the name,
 * hostname: the hostname,
 * ipv6: true to use ipv6,      (Optional: default false)
 * port: the port number,       (Optional: default 6667)
 * password: the password,      (Optional: default none)
 * channels: array of channels  (Optiona: default empty)
 * ssl: true to use ssl,        (Optional: default false)
 * nickname: "nickname",        (Optional, default: irccd)
 * username: "user name",       (Optional, default: irccd)
 * realname: "real name",       (Optional, default: IRC Client Daemon)
 * commandChar: "!",            (Optional, the command char, default: "!")
 *
 * Arguments:
 *   - params, the server properties
 * Throws:
 *   - Irccd.ServerError on server related errors,
 *   - Irccd.SystemError on other errors.
 */
auto Server_constructor(duk_context* ctx) -> duk_ret_t
{
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
}

// }}}

// {{{ Irccd.Server [destructor]

/*
 * Function: Irccd.Server() [destructor]
 * ------------------------------------------------------------------
 *
 * Delete the property.
 */
auto Server_destructor(duk_context* ctx) -> duk_ret_t
{
	duk_get_prop_string(ctx, 0, signature.data());
	delete static_cast<std::shared_ptr<server>*>(duk_to_pointer(ctx, -1));
	duk_pop(ctx);
	duk_del_prop_string(ctx, 0, signature.data());

	return 0;
}

// }}}

// {{{ Irccd.Server.add

/*
 * Function: Irccd.Server.add(s)
 * ------------------------------------------------------------------
 *
 * Register a new server to the irccd instance.
 *
 * Arguments:
 *   - s, the server to add.
 * Throws:
 *   - Irccd.SystemError on errors.
 */
auto Server_add(duk_context* ctx) -> duk_ret_t
{
	return wrap(ctx, [] (auto ctx) {
		duk::type_traits<bot>::self(ctx).servers().add(
			duk::require<std::shared_ptr<server>>(ctx, 0));

		return 0;
	});
}

// }}}

// {{{ Irccd.Server.find

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
 * Throws:
 *   - Irccd.SystemError on errors.
 */
auto Server_find(duk_context* ctx) -> duk_ret_t
{
	return wrap(ctx, [] (auto ctx) {
		auto id = duk::require<std::string>(ctx, 0);
		auto server = duk::type_traits<bot>::self(ctx).servers().get(id);

		if (!server)
			return 0;

		duk::push(ctx, server);

		return 1;
	});
}

// }}}

// {{{ Irccd.Server.list

/*
 * Function: Irccd.Server.list()
 * ------------------------------------------------------------------
 *
 * Get the map of all loaded servers.
 *
 * Returns:
 *   An object with string-to-servers pairs.
 */
auto Server_list(duk_context* ctx) -> duk_ret_t
{
	duk_push_object(ctx);

	for (const auto& server : duk::type_traits<bot>::self(ctx).servers().list()) {
		duk::push(ctx, server);
		duk_put_prop_string(ctx, -2, server->get_id().c_str());
	}

	return 1;
}

// }}}

// {{{ Irccd.Server.remove

/*
 * Function: Irccd.Server.remove(name)
 * ------------------------------------------------------------------
 *
 * Remove a server from the irccd instance. You can pass the server object since
 * it's coercible to a string.
 *
 * Arguments:
 *   - name the server name.
 */
auto Server_remove(duk_context* ctx) -> duk_ret_t
{
	duk::type_traits<bot>::self(ctx).servers().remove(duk_require_string(ctx, 0));

	return 0;
}

// }}}

// {{{ Irccd.ServerError

/*
 * Function: Irccd.ServerError(code, message)
 * ------------------------------------------------------------------
 *
 * Create an Irccd.ServerError object.
 *
 * Arguments:
 *   - code, the error code,
 *   - message, the error message.
 */
auto ServerError_constructor(duk_context* ctx) -> duk_ret_t
{
	duk_push_this(ctx);
	duk_push_int(ctx, duk_require_int(ctx, 0));
	duk_put_prop_string(ctx, -2, "code");
	duk_push_string(ctx, duk_require_string(ctx, 1));
	duk_put_prop_string(ctx, -2, "message");
	duk_push_string(ctx, "ServerError");
	duk_put_prop_string(ctx, -2, "name");
	duk_pop(ctx);

	return 0;
}

// }}}

const duk_function_list_entry methods[] = {
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
	{ nullptr,      nullptr,                        0               }
};

const duk_function_list_entry functions[] = {
	{ "add",        Server_add,                     1               },
	{ "find",       Server_find,                    1               },
	{ "list",       Server_list,                    0               },
	{ "remove",     Server_remove,                  1               },
	{ nullptr,      nullptr,                        0               }
};

} // !namespace

auto server_api::get_name() const noexcept -> std::string_view
{
	return "Irccd.Server";
}

void server_api::load(bot&, std::shared_ptr<plugin> plugin)
{
	duk::stack_guard sa(plugin->get_context());

	duk_get_global_string(plugin->get_context(), "Irccd");

	// ServerError function.
	duk_push_c_function(plugin->get_context(), ServerError_constructor, 2);
	duk_push_object(plugin->get_context());
	duk_get_global_string(plugin->get_context(), "Error");
	duk_get_prop_string(plugin->get_context(), -1, "prototype");
	duk_remove(plugin->get_context(), -2);
	duk_set_prototype(plugin->get_context(), -2);
	duk_put_prop_string(plugin->get_context(), -2, "prototype");
	duk_put_prop_string(plugin->get_context(), -2, "ServerError");

	// Server constructor.
	duk_push_c_function(plugin->get_context(), Server_constructor, 1);
	duk_put_function_list(plugin->get_context(), -1, functions);
	duk_push_object(plugin->get_context());
	duk_put_function_list(plugin->get_context(), -1, methods);
	duk_push_c_function(plugin->get_context(), Server_destructor, 1);
	duk_set_finalizer(plugin->get_context(), -2);
	duk_dup_top(plugin->get_context());
	duk_put_global_string(plugin->get_context(), prototype.data());
	duk_put_prop_string(plugin->get_context(), -2, "prototype");
	duk_put_prop_string(plugin->get_context(), -2, "Server");
	duk_pop(plugin->get_context());
}

namespace duk {

void type_traits<std::shared_ptr<server>>::push(duk_context* ctx, std::shared_ptr<server> server)
{
	assert(ctx);
	assert(server);

	duk::stack_guard sa(ctx, 1);

	duk_push_object(ctx);
	duk_push_pointer(ctx, new std::shared_ptr<class server>(std::move(server)));
	duk_put_prop_string(ctx, -2, signature.data());
	duk_get_global_string(ctx, prototype.data());
	duk_set_prototype(ctx, -2);
}

auto type_traits<std::shared_ptr<server>>::require(duk_context* ctx, duk_idx_t index) -> std::shared_ptr<server>
{
	if (!duk_is_object(ctx, index) || !duk_has_prop_string(ctx, index, signature.data()))
		duk_error(ctx, DUK_ERR_TYPE_ERROR, "not a Server object");

	duk_get_prop_string(ctx, index, signature.data());
	auto file = *static_cast<std::shared_ptr<server>*>(duk_to_pointer(ctx, -1));
	duk_pop(ctx);

	return file;
}

void type_traits<server_error>::raise(duk_context* ctx, const server_error& ex)
{
	duk::stack_guard sa(ctx, 1);

	duk_get_global_string(ctx, "Irccd");
	duk_get_prop_string(ctx, -1, "ServerError");
	duk_remove(ctx, -2);
	duk::push(ctx, ex.code().value());
	duk::push(ctx, ex.code().message());
	duk_new(ctx, 2);

	(void)duk_throw(ctx);
}

} // !duk

} // !irccd::js
