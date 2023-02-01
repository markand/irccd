/*
 * jsapi-plugin.c -- Irccd.Plugin API
 *
 * Copyright (c) 2013-2023 David Demelier <markand@malikania.fr>
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

#include <utlist.h>

#include <irccd/irccd.h>
#include <irccd/plugin.h>

#include "jsapi-plugin.h"

#define SIGNATURE DUK_HIDDEN_SYMBOL("Irccd.Plugin")

/*
 * set
 * ------------------------------------------------------------------
 *
 * This setter is used to replace the Irccd.Plugin.(config|templates|paths)
 * property when the plugin assign a new one.
 *
 * Because the plugin configuration always has higher priority, when a new
 * object is assigned to 'config' or to the 'templates' property, the plugin
 * configuration is merged to the assigned one, adding or replacing any values.
 *
 * Example:
 *
 * Plugin 'xyz' does:
 *
 * Irccd.Plugin.config = {
 *     mode: "simple",
 *     level: "123"
 * };
 *
 * The user configuration sets:
 *
 *     mode = "hard"
 *     path = "/var"
 *
 * The final user table looks like this:
 *
 * Irccd.Plugin.config = {
 *     mode: "hard",
 *     level: "123",
 *     path: "/var"
 * };
 */
static int
set(duk_context *ctx, const char *name)
{
	/* This is the object received in argument from the property setter. */
	if (!duk_is_object(ctx, 0))
		return duk_error(ctx, DUK_ERR_TYPE_ERROR, "'%s' property must be object", name);

	/* Merge old table with new one. */
	duk_get_global_string(ctx, name);
	duk_enum(ctx, -1, 0);

	while (duk_next(ctx, -1, 1))
		duk_put_prop(ctx, 0);

	/* Pop enum and old table. */
	duk_pop_2(ctx);

	/* Replace the old table with the new assigned one. */
	duk_put_global_string(ctx, name);

	return 0;
}

static int
get(duk_context *ctx, const char *name)
{
	duk_get_global_string(ctx, name);

	return 1;
}

static int
set_config(duk_context *ctx)
{
	return set(ctx, JSAPI_PLUGIN_PROP_OPTIONS);
}

static int
get_config(duk_context *ctx)
{
	return get(ctx, JSAPI_PLUGIN_PROP_OPTIONS);
}

static int
set_template(duk_context *ctx)
{
	return set(ctx, JSAPI_PLUGIN_PROP_TEMPLATES);
}

static int
get_template(duk_context *ctx)
{
	return get(ctx, JSAPI_PLUGIN_PROP_TEMPLATES);
}

static int
set_paths(duk_context *ctx)
{
	return set(ctx, JSAPI_PLUGIN_PROP_PATHS);
}

static int
get_paths(duk_context *ctx)
{
	return get(ctx, JSAPI_PLUGIN_PROP_PATHS);
}

static struct irc_plugin *
find(duk_context *ctx)
{
	return irc_bot_plugin_get(duk_require_string(ctx, 0));
}

static int
Plugin_info(duk_context *ctx)
{
	struct irc_plugin *p;

	if (duk_get_top(ctx) >= 1)
		p = find(ctx);
	else
		p = jsapi_plugin_self(ctx);

	if (!p)
		return 0;

	duk_push_object(ctx);
	duk_push_string(ctx, p->name);
	duk_put_prop_string(ctx, -2, "name");
	duk_push_string(ctx, p->author);
	duk_put_prop_string(ctx, -2, "author");
	duk_push_string(ctx, p->license);
	duk_put_prop_string(ctx, -2, "license");
	duk_push_string(ctx, p->description);
	duk_put_prop_string(ctx, -2, "summary");
	duk_push_string(ctx, p->version);
	duk_put_prop_string(ctx, -2, "version");

	return 1;
}

static int
Plugin_list(duk_context *ctx)
{
	size_t i = 0;
	struct irc_plugin *p;

	duk_push_array(ctx);

	LL_FOREACH(irc.plugins, p) {
		duk_push_string(ctx, p->name);
		duk_put_prop_index(ctx, -2, i++);
	}

	return 1;
}

static int
Plugin_load(duk_context *ctx)
{
	(void)ctx;

	return 0;
}

static int
Plugin_reload(duk_context *ctx)
{
	irc_plugin_reload(find(ctx));

	return 0;
}

static int
Plugin_unload(duk_context *ctx)
{
	/* Use find so it can raise ReferenceError if not found. */
	irc_bot_plugin_remove(find(ctx)->name);

	return 0;
}

static const duk_function_list_entry functions[] = {
	{ "info",       Plugin_info,    DUK_VARARGS     },
	{ "list",       Plugin_list,    0               },
	{ "load",       Plugin_load,    1               },
	{ "reload",     Plugin_reload,  1               },
	{ "unload",     Plugin_unload,  1               },
	{ NULL,         NULL,           0               }
};

void
jsapi_plugin_load(duk_context *ctx, struct irc_plugin *p)
{
	assert(ctx);
	assert(p);

	/* Store plugin. */
	duk_push_pointer(ctx, p);
	duk_put_global_string(ctx, SIGNATURE);

	duk_get_global_string(ctx, "Irccd");
	duk_push_object(ctx);
	duk_put_function_list(ctx, -1, functions);

	/* 'config' property. */
	duk_push_string(ctx, "config");
	duk_push_c_function(ctx, get_config, 0);
	duk_push_c_function(ctx, set_config, 1);
	duk_def_prop(ctx, -4, DUK_DEFPROP_HAVE_GETTER | DUK_DEFPROP_HAVE_SETTER);

	/* 'templates' property. */
	duk_push_string(ctx, "templates");
	duk_push_c_function(ctx, get_template, 0);
	duk_push_c_function(ctx, set_template, 1);
	duk_def_prop(ctx, -4, DUK_DEFPROP_HAVE_GETTER | DUK_DEFPROP_HAVE_SETTER);

	/* 'paths' property. */
	duk_push_string(ctx, "paths");
	duk_push_c_function(ctx, get_paths, 0);
	duk_push_c_function(ctx, set_paths, 1);
	duk_def_prop(ctx, -4, DUK_DEFPROP_HAVE_GETTER | DUK_DEFPROP_HAVE_SETTER);

	duk_put_prop_string(ctx, -2, "Plugin");
	duk_pop(ctx);
}

struct irc_plugin *
jsapi_plugin_self(duk_context *ctx)
{
	assert(ctx);

	struct irc_plugin *p;

	duk_get_global_string(ctx, SIGNATURE);
	p = duk_to_pointer(ctx, -1);
	duk_pop(ctx);

	return p;
}
