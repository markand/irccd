/*
 * jsapi-chrono.c -- Irccd.Chrono API
 *
 * Copyright (c) 2013-2024 David Demelier <markand@malikania.fr>
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
#include <time.h>

#include <duktape.h>

#include <irccd/util.h>

#define SIGNATURE DUK_HIDDEN_SYMBOL("Irccd.Chrono")

struct timer {
	struct timespec start;
};

static struct timer *
self(duk_context *ctx)
{
	struct timer *self;

	duk_push_this(ctx);
	duk_get_prop_string(ctx, -1, SIGNATURE);
	self = duk_to_pointer(ctx, -1);
	duk_pop_2(ctx);

	if (!self)
		(void)duk_error(ctx, DUK_ERR_TYPE_ERROR, "not an Chrono object");

	return self;
}

static int
Chrono_prototype_elapsed(duk_context *ctx)
{
	struct timer *timer = self(ctx);
	struct timespec now = {0};

	timespec_get(&now, TIME_UTC);

	duk_push_uint(ctx,
		((now.tv_sec * 1000) - (timer->start.tv_sec * 1000)) +
		((now.tv_nsec / 1000000) - (timer->start.tv_nsec / 1000000)));

	return 1;
}

static int
Chrono_prototype_reset(duk_context *ctx)
{
	timespec_get(&self(ctx)->start, TIME_UTC);

	return 0;
}

static int
Chrono_constructor(duk_context *ctx)
{
	struct timer *timer;

	timer = irc_util_calloc(1, sizeof (*timer));
	timespec_get(&timer->start, TIME_UTC);

	duk_push_this(ctx);
	duk_push_pointer(ctx, timer);
	duk_put_prop_string(ctx, -2, SIGNATURE);

	/* this.elapsed property. */
	duk_push_string(ctx, "elapsed");
	duk_push_c_function(ctx, Chrono_prototype_elapsed, 0);
	duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_GETTER);
	duk_pop(ctx);

	return 0;
}

static int
Chrono_destructor(duk_context *ctx)
{
	duk_get_prop_string(ctx, 0, SIGNATURE);
	free(duk_to_pointer(ctx, -1));
	duk_pop(ctx);
	duk_del_prop_string(ctx, 0, SIGNATURE);

	return 0;
}

static const duk_function_list_entry methods[] = {
	{ "reset",      Chrono_prototype_reset,         0 },
	{ NULL,         NULL,                           0 }
};

void
jsapi_chrono_load(duk_context *ctx)
{
	assert(ctx);

	duk_get_global_string(ctx, "Irccd");
	duk_push_c_function(ctx, Chrono_constructor, 0);
	duk_push_object(ctx);
	duk_put_function_list(ctx, -1, methods);
	duk_push_c_function(ctx, Chrono_destructor, 1);
	duk_set_finalizer(ctx, -2);
	duk_put_prop_string(ctx, -2, "prototype");
	duk_put_prop_string(ctx, -2, "Chrono");
	duk_pop(ctx);
}
