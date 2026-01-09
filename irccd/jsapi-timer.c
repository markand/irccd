/*
 * jsapi-timer.c -- Irccd.Timer API
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

#include <ev.h>
#include <duktape.h>

#include <irccd/irccd.h>
#include <irccd/log.h>
#include <irccd/plugin.h>
#include <irccd/util.h>

#include "jsapi-plugin.h"

#define SIGNATURE       DUK_HIDDEN_SYMBOL("Irccd.Timer")
#define PROP_CALLBACK   DUK_HIDDEN_SYMBOL("Irccd.Timer.callback")

enum stimer_type {
	STIMER_TYPE_REPEAT,
	STIMER_TYPE_ONESHOT,
	STIMER_TYPE_LAST
};

struct stimer {
	duk_context *ctx;       /* parent duktape context */
	void *addr;             /* reference to the Javascript timer object */
	struct ev_timer timer;  /* and the timer itself */
};

static void
stimer_cb(struct ev_timer *self, int)
{
	const struct irc_plugin *plg;
	struct stimer *st;

	st = IRC_UTIL_CONTAINER_OF(self, struct stimer, timer);
	plg = jsapi_plugin_self(st->ctx);

	duk_push_heapptr(st->ctx, st->addr);
	duk_push_string(st->ctx, PROP_CALLBACK);

	if (duk_pcall_prop(st->ctx, -2, 0) != DUK_EXEC_SUCCESS)
		irc_log_warn("plugin %s: %s", plg->name, duk_to_string(st->ctx, -1));

	duk_pop_n(st->ctx, 2);
}

static inline void
stimer_init(struct stimer *st, enum stimer_type type, unsigned int duration)
{
	ev_tstamp after, repeat;

	after  = duration / 1000.0;
	repeat = type == STIMER_TYPE_REPEAT ? after : 0.0;

	ev_timer_init(&st->timer, stimer_cb, after, repeat);
}

static inline void
stimer_start(struct stimer *st)
{
	ev_timer_start(&st->timer);
}

static inline void
stimer_restart(struct stimer *st)
{
	ev_timer_again(&st->timer);
}

static inline void
stimer_stop(struct stimer *st)
{
	ev_timer_stop(&st->timer);
}

static struct stimer *
stimer_self(duk_context *ctx)
{
	struct stimer *st;

	duk_push_this(ctx);
	duk_get_prop_string(ctx, -1, SIGNATURE);
	st = duk_to_pointer(ctx, -1);
	duk_pop_2(ctx);

	if (!st)
		(void)duk_error(ctx, DUK_ERR_TYPE_ERROR, "not a Timer object");

	return st;
}

static int
Timer_prototype_restart(duk_context *ctx)
{
	stimer_restart(stimer_self(ctx));

	return 0;
}

static int
Timer_prototype_start(duk_context *ctx)
{
	stimer_start(stimer_self(ctx));

	return 0;
}

static int
Timer_prototype_stop(duk_context *ctx)
{
	stimer_stop(stimer_self(ctx));

	return 0;
}

static int
Timer_destructor(duk_context *ctx)
{
	struct stimer *st;

	/* Remove timer property. */
	duk_get_prop_string(ctx, 0, SIGNATURE);
	st = duk_to_pointer(ctx, -1);
	duk_pop(ctx);
	duk_del_prop_string(ctx, -2, SIGNATURE);

	if (st) {
		stimer_stop(st);
		free(st);
	}

	return 0;
}

static int
Timer_constructor(duk_context *ctx)
{
	struct stimer *st;
	unsigned int type, duration;

	if (!duk_is_constructor_call(ctx))
		return 0;

	type = duk_require_int(ctx, 0);
	duration = duk_require_uint(ctx, 1);

	if (type >= STIMER_TYPE_LAST)
		return duk_error(ctx, DUK_ERR_TYPE_ERROR, "invalid timer type");
	if (!duk_is_callable(ctx, 2))
		return duk_error(ctx, DUK_ERR_TYPE_ERROR, "missing callback function");

	st = irc_util_calloc(1, sizeof (*st));
	st->ctx = ctx;
	stimer_init(st, type, duration);

	/* Create this. */
	duk_push_this(ctx);
	duk_push_pointer(ctx, st);
	duk_put_prop_string(ctx, -2, SIGNATURE);
	duk_push_c_function(ctx, Timer_destructor, 1);
	duk_set_finalizer(ctx, -2);

	/* Reference this object itself into the timer to retrieve it later on. */
	st->addr = duk_get_heapptr(ctx, -1);

	/* Duplicate the callback internally as this.callback property. */
	duk_dup(ctx, 2);
	duk_put_prop_string(ctx, -2, PROP_CALLBACK);

	duk_pop(ctx);

	return 0;
}

static const duk_function_list_entry methods[] = {
	{ "restart", Timer_prototype_restart, 0 },
	{ "start",   Timer_prototype_start,   0 },
	{ "stop",    Timer_prototype_stop,    0 },
	{ NULL,      NULL,                    0 }
};

static const duk_number_list_entry constants[] = {
	{ "Single", STIMER_TYPE_ONESHOT },
	{ "Repeat", STIMER_TYPE_REPEAT  },
	{ NULL,     0                   }
};

void
jsapi_timer_load(duk_context *ctx)
{
	assert(ctx);

	duk_get_global_string(ctx, "Irccd");
	duk_push_c_function(ctx, Timer_constructor, 3);
	duk_put_number_list(ctx, -1, constants);
	duk_push_object(ctx);
	duk_put_function_list(ctx, -1, methods);
	duk_put_prop_string(ctx, -2, "prototype");
	duk_put_prop_string(ctx, -2, "Timer");
	duk_pop(ctx);
}
