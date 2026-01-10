/*
 * cstat.c -- coroutine watcher support for ev_stat
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

#include "cstat.h"

static void
cstat_cb(EV_P_ struct ev_stat *self, int revents)
{
	struct cstat *ev = CSTAT(self, stat);

	if (revents & (EV_STAT)) {
#if !defined(NDEBUG)
		/*
		 * Make sure user fetched events through the cstat_wait
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

static void
cstat_coro_entry_cb(EV_P_ struct coro *self)
{
	struct cstat_coro *evco = CORO_CONTAINER_OF(self, struct cstat_coro, coro);

	evco->entry(EV_A_ &evco->stat);
}

static void
cstat_coro_finalizer_cb(EV_P_ struct coro *self)
{
	struct cstat_coro *evco = CORO_CONTAINER_OF(self, struct cstat_coro, coro);

	/* Stop the watcher for convenience. */
	cstat_stop(EV_A_ &evco->stat);
	cstat_finish(&evco->stat);

	/* Call user as very last function. */
	if (evco->finalizer)
		evco->finalizer(EV_A_ &evco->stat);
}

void
cstat_init(struct cstat *ev)
{
	assert(ev);

	ev_init(&ev->stat, cstat_cb);
	ev->revents = 0;
}

void
cstat_start(EV_P_ struct cstat *ev)
{
	assert(ev);

	ev_stat_start(EV_A_ &ev->stat);
}

int
cstat_active(const struct cstat *ev)
{
	assert(ev);

	return ev->stat.active;
}

void
cstat_feed(EV_P_ struct cstat *ev, int events)
{
	assert(ev);

	ev_feed_event(EV_A_ &ev->stat, events);
}

void
cstat_stop(EV_P_ struct cstat *ev)
{
	assert(ev);

	ev_stat_stop(EV_A_ &ev->stat);
}

int
cstat_ready(struct cstat *ev)
{
	assert(ev);

	int rc = 0;

	if (ev->revents & (EV_STAT)) {
		rc = ev->revents;
		ev->revents = 0;
	}

	return rc;
}

int
cstat_wait(EV_P_ struct cstat *ev)
{
	assert(ev);

	int rc = 0;

	while (!(rc = cstat_ready(ev)))
		coro_yield();

	return rc;
}

void
cstat_set(struct cstat *ev, const char *path, ev_tstamp interval)
{
	assert(ev);
	assert(path);

	ev_stat_set(&ev->stat, path, interval);
}

void
cstat_stat(EV_P_ struct cstat *ev)
{
	assert(ev);

	ev_stat_stat(EV_A_ &ev->stat);
}

void
cstat_finish(struct cstat *ev)
{
	assert(ev);
	assert(ev->stat.active == 0);

	ev->revents = 0;
}

void
cstat_coro_init(struct cstat_coro *evco)
{
	assert(evco);

	cstat_init(&evco->stat);
	coro_init(&evco->coro);

	evco->coro.entry = cstat_coro_entry_cb;

	if (!evco->coro.finalizer)
		evco->coro.finalizer = cstat_coro_finalizer_cb;
}

int
cstat_coro_spawn(EV_P_ struct cstat_coro *evco, const struct cstat_coro_ops *ops)
{
	assert(evco);
	assert(evco->entry);

	int rc;

	cstat_coro_init(evco);

	/*
	 * Watchers should be executed before attached coroutines to allow
	 * resuming them if an event happened.
	 */
	ev_set_priority(&evco->stat.stat, CORO_PRI_MAX - 1);
	if (ops)
		cstat_set(&evco->stat, ops->path, ops->interval);
	else
		evco->coro.flags |= CORO_INACTIVE;

	/* Automatically start the watcher unless disabled. */
	if (!(evco->coro.flags & CORO_INACTIVE))
		cstat_start(EV_A_ &evco->stat);

	if ((rc = coro_create(EV_A_ &evco->coro)) < 0)
		cstat_stop(EV_A_ &evco->stat);
	else
		coro_resume(&evco->coro);

	return rc;
}

void
cstat_coro_finish(struct cstat_coro *evco)
{
	assert(evco);

	/* Will call cstat_coro_finalizer_cb */
	coro_finish(&evco->coro);
}

