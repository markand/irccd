/*
 * server_jsapi.cpp -- Irccd.Server API
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

#include <irccd/daemon/irccd.hpp>
#include <irccd/daemon/server_util.hpp>

#include <irccd/daemon/service/server_service.hpp>

#include "duktape_vector.hpp"
#include "irccd_jsapi.hpp"
#include "js_plugin.hpp"
#include "server_jsapi.hpp"

namespace irccd {

namespace {

const char *signature("\xff""\xff""irccd-server-ptr");
const char *prototype("\xff""\xff""irccd-server-prototype");

std::shared_ptr<server> self(duk_context* ctx)
{
    dukx_stack_assert sa(ctx);

    duk_push_this(ctx);
    duk_get_prop_string(ctx, -1, signature);
    auto ptr = duk_to_pointer(ctx, -1);
    duk_pop_2(ctx);

    if (!ptr)
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "not a Server object");

    return *static_cast<std::shared_ptr<server>*>(ptr);
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
duk_ret_t info(duk_context* ctx)
{
    auto server = self(ctx);

    duk_push_object(ctx);
    dukx_push(ctx, server->get_name());
    duk_put_prop_string(ctx, -2, "name");
    dukx_push(ctx, server->get_host());
    duk_put_prop_string(ctx, -2, "host");
    duk_push_int(ctx, server->get_port());
    duk_put_prop_string(ctx, -2, "port");
    duk_push_boolean(ctx, server->get_flags() & server::ssl);
    duk_put_prop_string(ctx, -2, "ssl");
    duk_push_boolean(ctx, server->get_flags() & server::ssl_verify);
    duk_put_prop_string(ctx, -2, "sslVerify");
    dukx_push(ctx, server->get_command_char());
    duk_put_prop_string(ctx, -2, "commandChar");
    dukx_push(ctx, server->get_realname());
    duk_put_prop_string(ctx, -2, "realname");
    dukx_push(ctx, server->get_nickname());
    duk_put_prop_string(ctx, -2, "nickname");
    dukx_push(ctx, server->get_username());
    duk_put_prop_string(ctx, -2, "username");
    dukx_push(ctx, server->get_channels());
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
duk_ret_t invite(duk_context* ctx)
{
    self(ctx)->invite(duk_require_string(ctx, 0), duk_require_string(ctx, 1));

    return 0;
}

/*
 * Method: Server.isSelf(nickname)
 * ------------------------------------------------------------------
 */
duk_ret_t isSelf(duk_context* ctx)
{
    return dukx_push(ctx, self(ctx)->is_self(duk_require_string(ctx, 0)));
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
duk_ret_t join(duk_context* ctx)
{
    self(ctx)->join(duk_require_string(ctx, 0), duk_require_string(ctx, 1));

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
duk_ret_t kick(duk_context* ctx)
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
duk_ret_t me(duk_context* ctx)
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
duk_ret_t message(duk_context* ctx)
{
    self(ctx)->message(duk_require_string(ctx, 0), duk_require_string(ctx, 1));

    return 0;
}

/*
 * Method: Server.mode(channel, mode, limit, user, mask)
 * ------------------------------------------------------------------
 *
 * Change your mode.
 *
 * Arguments:
 *   - mode, the new mode.
 */
duk_ret_t mode(duk_context* ctx)
{
    self(ctx)->mode(
        duk_require_string(ctx, 0),
        duk_require_string(ctx, 1),
        duk_opt_string(ctx, 2, ""),
        duk_opt_string(ctx, 3, ""),
        duk_opt_string(ctx, 4, "")
    );

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
duk_ret_t names(duk_context* ctx)
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
duk_ret_t nick(duk_context* ctx)
{
    self(ctx)->set_nickname(duk_require_string(ctx, 0));

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
duk_ret_t notice(duk_context* ctx)
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
duk_ret_t part(duk_context* ctx)
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
duk_ret_t send(duk_context* ctx)
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
duk_ret_t topic(duk_context* ctx)
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
duk_ret_t whois(duk_context* ctx)
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
duk_ret_t toString(duk_context* ctx)
{
    dukx_push(ctx, self(ctx)->get_name());

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
 * ipv6: true to use ipv6,      (Optional: default false)
 * port: the port number,       (Optional: default 6667)
 * password: the password,      (Optional: default none)
 * channels: array of channels  (Optiona: default empty)
 * ssl: true to use ssl,        (Optional: default false)
 * sslVerify: true to verify    (Optional: default true)
 * nickname: "nickname",        (Optional, default: irccd)
 * username: "user name",       (Optional, default: irccd)
 * realname: "real name",       (Optional, default: IRC Client Daemon)
 * commandChar: "!",            (Optional, the command char, default: "!")
 */
duk_ret_t constructor(duk_context* ctx)
{
    if (!duk_is_constructor_call(ctx))
        return 0;

    duk_check_type(ctx, 0, DUK_TYPE_OBJECT);

    try {
        auto json = duk_json_encode(ctx, 0);
        auto s = server_util::from_json(dukx_get_irccd(ctx).get_service(), nlohmann::json::parse(json));

        duk_push_this(ctx);
        duk_push_pointer(ctx, new std::shared_ptr<server>(std::move(s)));
        duk_put_prop_string(ctx, -2, signature);
        duk_pop(ctx);
    } catch (const std::exception& ex) {
        duk_error(ctx, DUK_ERR_ERROR, "%s", ex.what());
    }

    return 0;
}

/*
 * Function: Irccd.Server() [destructor]
 * ------------------------------------------------------------------
 *
 * Delete the property.
 */
duk_ret_t destructor(duk_context* ctx)
{
    duk_get_prop_string(ctx, 0, signature);
    delete static_cast<std::shared_ptr<server>*>(duk_to_pointer(ctx, -1));
    duk_pop(ctx);
    duk_del_prop_string(ctx, 0, signature);

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
duk_ret_t add(duk_context* ctx)
{
    dukx_get_irccd(ctx).servers().add(dukx_require<std::shared_ptr<server>>(ctx, 0));

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
duk_ret_t find(duk_context* ctx)
{
    auto server = dukx_get_irccd(ctx).servers().get(duk_require_string(ctx, 0));

    if (!server)
        return 0;

    dukx_push(ctx, server);

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
duk_ret_t list(duk_context* ctx)
{
    duk_push_object(ctx);

    for (const auto &server : dukx_get_irccd(ctx).servers().servers()) {
        dukx_push(ctx, server);
        duk_put_prop_string(ctx, -2, server->get_name().c_str());
    }

    return 1;
}

/*
 * Function: irccd.Server.remove(name)
 * ------------------------------------------------------------------
 *
 * Remove a server from the irccd instance. You can pass the server object since
 * it's coercible to a string.
 *
 * Arguments:
 *   - name the server name.
 */
duk_ret_t remove(duk_context* ctx)
{
    dukx_get_irccd(ctx).servers().remove(duk_require_string(ctx, 0));

    return 0;
}

const duk_function_list_entry methods[] = {
    { "info",       info,       0           },
    { "invite",     invite,     2           },
    { "isSelf",     isSelf,     1           },
    { "join",       join,       DUK_VARARGS },
    { "kick",       kick,       DUK_VARARGS },
    { "me",         me,         2           },
    { "message",    message,    2           },
    { "mode",       mode,       1           },
    { "names",      names,      1           },
    { "nick",       nick,       1           },
    { "notice",     notice,     2           },
    { "part",       part,       DUK_VARARGS },
    { "send",       send,       1           },
    { "topic",      topic,      2           },
    { "whois",      whois,      1           },
    { "toString",   toString,   0           },
    { nullptr,      nullptr,    0           }
};

const duk_function_list_entry functions[] = {
    { "add",        add,        1           },
    { "find",       find,       1           },
    { "list",       list,       0           },
    { "remove",     remove,     1           },
    { nullptr,      nullptr,    0           }
};

} // !namespace

std::string server_jsapi::name() const
{
    return "Irccd.Server";
}

void server_jsapi::load(irccd&, std::shared_ptr<js_plugin> plugin)
{
    dukx_stack_assert sa(plugin->context());

    duk_get_global_string(plugin->context(), "Irccd");
    duk_push_c_function(plugin->context(), constructor, 1);
    duk_put_function_list(plugin->context(), -1, functions);
    duk_push_object(plugin->context());
    duk_put_function_list(plugin->context(), -1, methods);
    duk_push_c_function(plugin->context(), destructor, 1);
    duk_set_finalizer(plugin->context(), -2);
    duk_dup_top(plugin->context());
    duk_put_global_string(plugin->context(), prototype);
    duk_put_prop_string(plugin->context(), -2, "prototype");
    duk_put_prop_string(plugin->context(), -2, "Server");
    duk_pop(plugin->context());
}

using server_traits = dukx_type_traits<std::shared_ptr<server>>;

void server_traits::push(duk_context* ctx, std::shared_ptr<server> server)
{
    assert(ctx);
    assert(server);

    dukx_stack_assert sa(ctx, 1);

    duk_push_object(ctx);
    duk_push_pointer(ctx, new std::shared_ptr<class server>(std::move(server)));
    duk_put_prop_string(ctx, -2, signature);
    duk_get_global_string(ctx, prototype);
    duk_set_prototype(ctx, -2);
}

std::shared_ptr<server> server_traits::require(duk_context* ctx, duk_idx_t index)
{
    if (!duk_is_object(ctx, index) || !duk_has_prop_string(ctx, index, signature))
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "not a Server object");

    duk_get_prop_string(ctx, index, signature);
    auto file = *static_cast<std::shared_ptr<server> *>(duk_to_pointer(ctx, -1));
    duk_pop(ctx);

    return file;
}

} // !irccd
