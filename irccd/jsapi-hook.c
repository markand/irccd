/*
 * jsapi-hook.c -- Irccd.Hook API
 *
 * Copyright (c) 2013-2026 David Demelier <markand@malikania.fr>
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

#include <irccd/hook.h>
#include <irccd/irccd.h>

#include "jsapi-hook.h"

static int
Hook_add(duk_context *ctx)
{
	const char *name = duk_require_string(ctx, 0);
	const char *path = duk_require_string(ctx, 1);

	if (irc_bot_hook_get(name))
		return duk_error(ctx, DUK_ERR_ERROR, "hook %s already exists", name);

	irc_bot_hook_add(irc_hook_new(name, path));

	return 0;
}

static int
Hook_list(duk_context *ctx)
{
	struct irc_hook *h;
	size_t i = 0;

	duk_push_array(ctx);

	LL_FOREACH(irccd->hooks, h) {
		duk_push_object(ctx);
		duk_push_string(ctx, h->name);
		duk_put_prop_string(ctx, -2, "name");
		duk_push_string(ctx, h->path);
		duk_put_prop_string(ctx, -2, "path");
		duk_put_prop_index(ctx, -2, i++);
	}

	return 1;
}

static int
Hook_remove(duk_context *ctx)
{
	irc_bot_hook_remove(duk_require_string(ctx, 0));

	return 0;
}

static const duk_function_list_entry functions[] = {
	{ "add",        Hook_add,       2 },
	{ "list",       Hook_list,      0 },
	{ "remove",     Hook_remove,    1 },
	{ NULL,         NULL,           0 }
};

void
jsapi_hook_load(duk_context *ctx)
{
	assert(ctx);

	duk_get_global_string(ctx, "Irccd");
	duk_push_object(ctx);
	duk_put_function_list(ctx, -1, functions);
	duk_put_prop_string(ctx, -2, "Hook");
	duk_pop(ctx);
}
