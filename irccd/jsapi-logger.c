/*
 * jsapi-logger.c -- Irccd.Logger API
 *
 * Copyright (c) 2013-2022 David Demelier <markand@malikania.fr>
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

#include <irccd/plugin.h>
#include <irccd/log.h>

#include "jsapi-logger.h"
#include "jsapi-plugin.h"

#define LOG(c, f)                                                       \
do {                                                                    \
        const struct irc_plugin *p = jsapi_plugin_self(c);              \
        const char *message = duk_require_string(c, 0);                 \
                                                                        \
        f("plugin %s: %s", p->name, message);                           \
} while (0)                                                             \

static int
Logger_info(duk_context *ctx)
{
	LOG(ctx, irc_log_info);

	return 0;
}

static int
Logger_warning(duk_context *ctx)
{
	LOG(ctx, irc_log_warn);

	return 0;
}

static int
Logger_debug(duk_context *ctx)
{
	LOG(ctx, irc_log_debug);

	return 0;
}

static const duk_function_list_entry functions[] = {
	{ "info",       Logger_info,    1 },
	{ "warning",    Logger_warning, 1 },
	{ "debug",      Logger_debug,   1 },
	{ NULL,         NULL,           0 }
};

void
jsapi_logger_load(duk_context *ctx)
{
	assert(ctx);

	duk_get_global_string(ctx, "Irccd");
	duk_push_object(ctx);
	duk_put_function_list(ctx, -1, functions);
	duk_put_prop_string(ctx, -2, "Logger");
	duk_pop(ctx);
}
