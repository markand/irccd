/*
 * jsapi-timer.c -- Irccd.Timer API
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
#include <pthread.h>
#include <stdatomic.h>
#include <time.h>
#include <errno.h>

#include <duktape.h>

#include <irccd/irccd.h>
#include <irccd/log.h>
#include <irccd/util.h>

#include "jsapi-system.h"

#define SIGNATURE       DUK_HIDDEN_SYMBOL("Irccd.Timer")
#define TABLE           DUK_HIDDEN_SYMBOL("Irccd.Timer.callbacks")

enum timer_type {
	TIMER_REPEAT,
	TIMER_ONESHOT,
	TIMER_NUM
};

enum timer_status {
	TIMER_INACTIVE,
	TIMER_ACTIVE,
	TIMER_MUST_STOP,
	TIMER_MUST_KILL
};

struct timer {
	enum timer_type type;
	unsigned long duration;
	pthread_t thr;
	pthread_mutex_t mtx;
	pthread_cond_t cv;
	duk_context *ctx;
	atomic_int status;
};

static void
timer_start(struct timer *);

static void
timer_clear(struct timer *tm)
{
	tm->status = TIMER_INACTIVE;

	pthread_join(tm->thr, NULL);
	pthread_cond_destroy(&tm->cv);
	pthread_mutex_destroy(&tm->mtx);
}

static void
timer_call(struct timer *tm)
{
	/* Get the function. */
	duk_push_global_stash(tm->ctx);
	duk_get_prop_string(tm->ctx, -1, TABLE);
	duk_remove(tm->ctx, -2);
	duk_push_sprintf(tm->ctx, "%p", tm);
	duk_get_prop(tm->ctx, -2);
	duk_remove(tm->ctx, -2);

	if (duk_pcall(tm->ctx, 0))
		irc_log_warn("plugin: %s", duk_to_string(tm->ctx, -1));

	duk_pop(tm->ctx);
}

static void
timer_destroy(struct timer *tm)
{
	if (tm->status == TIMER_MUST_STOP)
		timer_clear(tm);
	if (tm->status == TIMER_MUST_KILL)
		free(tm);
}

static void
timer_expired(void *data)
{
	struct timer *tm = data;

	/* Only call if I wasn't aborted (race condition) */
	if (tm->status == TIMER_ACTIVE) {
		timer_clear(tm);
		timer_call(tm);

		/* Start again. */
		if (tm->type == TIMER_REPEAT)
			timer_start(tm);
	} else
		timer_destroy(tm);
}

static void
timer_aborted(void *data)
{
	timer_destroy(data);
}

static void *
timer_routine(void *data)
{
	struct timer *tm = data;
	struct timespec ts = {0};
	int rc = 0;

	/* Prepare maximum time to wait. */
	timespec_get(&ts, TIME_UTC);

	ts.tv_sec  += tm->duration / 1000;
	ts.tv_nsec += (tm->duration % 1000) * 1000000;

	/* Readjust to avoid EINVAL. */
	ts.tv_sec  += ts.tv_nsec / 1000000000;
	ts.tv_nsec %= 1000000000;

	if (pthread_mutex_lock(&tm->mtx) != 0)
		tm->status = TIMER_MUST_STOP;

	/* Wait at most time unless I'm getting kill. */
	while (tm->status == TIMER_ACTIVE && rc == 0)
		rc = pthread_cond_timedwait(&tm->cv, &tm->mtx, &ts);

	/*
	 * When the thread ends, there are several possibilities:
	 *
	 * 1. It has completed without being aborted.
	 * 2. It has been stopped by the user.
	 * 3. The plugin is shutting down.
	 */
	if (rc == ETIMEDOUT && tm->status == TIMER_ACTIVE)
		irc_bot_post(timer_expired, tm);
	else
		irc_bot_post(timer_aborted, tm);

	return NULL;
}

static void
timer_start(struct timer *tm)
{
	if (tm->status != TIMER_INACTIVE)
		return;

	tm->status = TIMER_ACTIVE;

	if (pthread_mutex_init(&tm->mtx, NULL) != 0)
		goto mutex_err;
	if (pthread_cond_init(&tm->cv, NULL) != 0)
		goto cond_err;
	if (pthread_create(&tm->thr, NULL, timer_routine, tm) != 0)
		goto thread_err;

	return;

thread_err:
	pthread_cond_destroy(&tm->cv);

cond_err:
	pthread_mutex_destroy(&tm->mtx);

mutex_err:
	tm->status = TIMER_INACTIVE;
	jsapi_system_raise(tm->ctx);
}

static void
timer_stop(struct timer *tm, enum timer_status st)
{
	if (tm->status == TIMER_INACTIVE)
		return;

	tm->status = st;
	pthread_cond_signal(&tm->cv);
}

static struct timer *
self(duk_context *ctx)
{
	struct timer *tm;

	duk_push_this(ctx);
	duk_get_prop_string(ctx, -1, SIGNATURE);
	tm = duk_to_pointer(ctx, -1);
	duk_pop_2(ctx);

	if (!tm)
		(void)duk_error(ctx, DUK_ERR_TYPE_ERROR, "not a Timer object");

	return tm;
}

static int
Timer_prototype_start(duk_context *ctx)
{
	timer_start(self(ctx));

	return 0;
}

static int
Timer_prototype_stop(duk_context *ctx)
{
	timer_stop(self(ctx), TIMER_MUST_STOP);

	return 0;
}

static int
Timer_destructor(duk_context *ctx)
{
	struct timer *tm;

	/* Remove timer property. */
	duk_get_prop_string(ctx, 0, SIGNATURE);
	tm = duk_to_pointer(ctx, -1);
	duk_pop(ctx);
	duk_del_prop_string(ctx, -2, SIGNATURE);

	/* Remove callback from timer table. */
	duk_push_global_stash(tm->ctx);
	duk_get_prop_string(tm->ctx, -1, TABLE);
	duk_remove(tm->ctx, -2);
	duk_push_sprintf(tm->ctx, "%p", tm);
	duk_del_prop(tm->ctx, -2);
	duk_pop(tm->ctx);

	/*
	 * Do not delete the timer itself here because the thread is
	 * referencing it. Stop it and let it kill itself from the main thread.
	 */
	if (tm)
		timer_stop(tm, TIMER_MUST_KILL);

	return 0;
}

static int
Timer_constructor(duk_context *ctx)
{
	struct timer *ptr, tm = {
		.ctx = ctx,
	};

	if (!duk_is_constructor_call(ctx))
		return 0;

	tm.type = duk_require_int(ctx, 0);
	tm.duration = duk_require_uint(ctx, 1);

	if (tm.type < 0 || tm.type >= TIMER_NUM)
		return duk_error(ctx, DUK_ERR_TYPE_ERROR, "invalid timer type");
	if (!duk_is_callable(ctx, 2))
		return duk_error(ctx, DUK_ERR_TYPE_ERROR, "missing callback function");

	/* Create this. */
	duk_push_this(ctx);
	duk_push_pointer(ctx, (ptr = irc_util_memdup(&tm, sizeof (tm))));
	duk_put_prop_string(ctx, -2, SIGNATURE);
	duk_push_c_function(ctx, Timer_destructor, 1);
	duk_set_finalizer(ctx, -2);
	duk_pop(ctx);

	/* Store the function into the global table */
	duk_push_global_stash(ctx);
	duk_get_prop_string(ctx, -1, TABLE);
	duk_remove(ctx, -2);
	duk_push_sprintf(ctx, "%p", ptr);
	duk_dup(ctx, 2);
	duk_put_prop(ctx, -3);
	duk_pop(ctx);

	return 0;
}

static const duk_function_list_entry methods[] = {
	{ "start",      Timer_prototype_start,  0               },
	{ "stop",       Timer_prototype_stop,   0               },
	{ NULL,         NULL,                   0               }
};

static const duk_number_list_entry constants[] = {
	{ "Single",     TIMER_ONESHOT                           },
	{ "Repeat",     TIMER_REPEAT                            },
	{ NULL,         0                                       }
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
	duk_push_global_stash(ctx);
	duk_push_object(ctx);
	duk_put_prop_string(ctx, -2, TABLE);
	duk_pop(ctx);
}
