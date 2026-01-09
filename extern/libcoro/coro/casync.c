/*
 * casync.c -- coroutine watcher support for ev_async
 *
 * Copyright (c) 2025-2026 David Demelier <markand@malikania.fr>
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
#include <stdio.h>
#include <stdlib.h>

#include <ev.h>

#include "casync.h"

static void
casync_cb(EV_P_ struct ev_async *self, int revents)
{
	struct casync *ev = CASYNC(self, async);

	if (revents & (EV_ASYNC)) {
#if !defined(NDEBUG)
		/*
		 * Make sure user fetched events through the casync_wait
		 * function in the meantime.
		 */
		if (ev->revents) {
			fprintf(stderr, "abort: pending events not cleared\n");
			abort();
		}
#endif
		ev->revents = revents;
	}
}

static inline void
casync_coro_finalizer(struct casync_coro *coro)
{
#if EV_MULTIPLICITY
	if (coro->coro.loop)
		casync_stop(coro->coro.loop, &coro->async);
#else
	casync_stop(&coro->async);
#endif

	casync_finish(&coro->async);
}

static void
casync_coro_entry_cb(EV_P_ struct coro *self)
{
	struct casync_coro *coro = CORO_CONTAINER_OF(self, struct casync_coro, coro);

	coro->entry(EV_A_ &coro->async);
}

static void
casync_coro_finalizer_cb(EV_P_ struct coro *self)
{
	struct casync_coro *coro = CORO_CONTAINER_OF(self, struct casync_coro, coro);

	casync_coro_finalizer(coro);
}

void
casync_init(struct casync *ev)
{
	assert(ev);

	ev->revents = 0;
	ev->async = (const struct ev_async) {};
	ev_init(&ev->async, casync_cb);
}

void
casync_start(EV_P_ struct casync *ev)
{
	assert(ev);

	ev_async_start(EV_A_ &ev->async);
}

int
casync_active(const struct casync *ev)
{
	assert(ev);

	return ev->async.active;
}

void
casync_feed(EV_P_ struct casync *ev, int events)
{
	assert(ev);

	ev_feed_event(EV_A_ &ev->async, events);
}

void
casync_stop(EV_P_ struct casync *ev)
{
	assert(ev);

	ev_async_stop(EV_A_ &ev->async);
}

int
casync_ready(struct casync *ev)
{
	assert(ev);

	int rc = 0;

	if (ev->revents & (EV_ASYNC)) {
		rc = ev->revents;
		ev->revents = 0;
	}

	return rc;
}

int
casync_wait(EV_P_ struct casync *ev)
{
	assert(ev);

	int rc = 0;

	while (!(rc = casync_ready(ev)))
		coro_yield();

	return rc;
}

void
casync_finish(struct casync *ev)
{
	assert(ev);
	assert(ev->async.active == 0);

	ev->revents = 0;
}

int
casync_coro_spawn(EV_P_ struct casync_coro *coro, const struct casync_coro_def *def)
{
	assert(coro);
	assert(def);

	int rc;

	coro->entry = def->entry;
	casync_init(&coro->async);

	/*
	 * Watchers should be executed before attached coroutines to allow
	 * resuming them if an event happened.
	 */
	ev_set_priority(&coro->async.async, CORO_PRI_MAX - 1);

	/* Automatically start the watcher unless disabled. */
	if (!(def->flags & CORO_INACTIVE))
		casync_start(EV_A_ &coro->async);

	coro_init(&coro->coro);

	/* All other fields are available for customization. */
	coro_set_name(&coro->coro, def->name);
	coro_set_stack_size(&coro->coro, def->stack_size);
	coro_set_flags(&coro->coro, def->flags);
	coro_set_entry(&coro->coro, casync_coro_entry_cb);

	/*
	 * Use a default finalizer if NULL to conveniently stop and destroy the
	 * underlying watcher.
	 */
	if (!def->finalizer)
		coro_set_finalizer(&coro->coro, casync_coro_finalizer_cb);
	else
		coro_set_finalizer(&coro->coro, def->finalizer);

	if ((rc = coro_create(EV_A_ &coro->coro)) < 0)
		casync_stop(EV_A_ &coro->async);
	else
		coro_resume(&coro->coro);

	return rc;
}
