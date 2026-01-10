/*
 * cprepare.c -- coroutine watcher support for ev_prepare
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

#include "cprepare.h"

static void
cprepare_cb(EV_P_ struct ev_prepare *self, int revents)
{
	struct cprepare *ev = CPREPARE(self, prepare);

	if (revents & (EV_PREPARE)) {
#if !defined(NDEBUG)
		/*
		 * Make sure user fetched events through the cprepare_wait
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
cprepare_coro_entry_cb(EV_P_ struct coro *self)
{
	struct cprepare_coro *evco = CORO_CONTAINER_OF(self, struct cprepare_coro, coro);

	evco->entry(EV_A_ &evco->prepare);
}

static void
cprepare_coro_finalizer_cb(EV_P_ struct coro *self)
{
	struct cprepare_coro *evco = CORO_CONTAINER_OF(self, struct cprepare_coro, coro);

	/* Stop the watcher for convenience. */
	cprepare_stop(EV_A_ &evco->prepare);
	cprepare_finish(&evco->prepare);

	/* Call user as very last function. */
	if (evco->finalizer)
		evco->finalizer(EV_A_ &evco->prepare);
}

void
cprepare_init(struct cprepare *ev)
{
	assert(ev);

	ev_init(&ev->prepare, cprepare_cb);
	ev->revents = 0;
}

void
cprepare_start(EV_P_ struct cprepare *ev)
{
	assert(ev);

	ev_prepare_start(EV_A_ &ev->prepare);
}

int
cprepare_active(const struct cprepare *ev)
{
	assert(ev);

	return ev->prepare.active;
}

void
cprepare_feed(EV_P_ struct cprepare *ev, int events)
{
	assert(ev);

	ev_feed_event(EV_A_ &ev->prepare, events);
}

void
cprepare_stop(EV_P_ struct cprepare *ev)
{
	assert(ev);

	ev_prepare_stop(EV_A_ &ev->prepare);
}

int
cprepare_ready(struct cprepare *ev)
{
	assert(ev);

	int rc = 0;

	if (ev->revents & (EV_PREPARE)) {
		rc = ev->revents;
		ev->revents = 0;
	}

	return rc;
}

int
cprepare_wait(EV_P_ struct cprepare *ev)
{
	assert(ev);

	int rc = 0;

	while (!(rc = cprepare_ready(ev)))
		coro_yield();

	return rc;
}

void
cprepare_finish(struct cprepare *ev)
{
	assert(ev);
	assert(ev->prepare.active == 0);

	ev->revents = 0;
}

void
cprepare_coro_init(struct cprepare_coro *evco)
{
	assert(evco);

	cprepare_init(&evco->prepare);
	coro_init(&evco->coro);

	evco->coro.entry = cprepare_coro_entry_cb;

	if (!evco->coro.finalizer)
		evco->coro.finalizer = cprepare_coro_finalizer_cb;
}

int
cprepare_coro_spawn(EV_P_ struct cprepare_coro *evco, const struct cprepare_coro_ops *ops)
{
	assert(evco);
	assert(evco->entry);

	int rc;

	cprepare_coro_init(evco);

	/*
	 * Watchers should be executed before attached coroutines to allow
	 * resuming them if an event happened.
	 */
	ev_set_priority(&evco->prepare.prepare, CORO_PRI_MAX - 1);
	/* Automatically start the watcher unless disabled. */
	if (!(evco->coro.flags & CORO_INACTIVE))
		cprepare_start(EV_A_ &evco->prepare);

	if ((rc = coro_create(EV_A_ &evco->coro)) < 0)
		cprepare_stop(EV_A_ &evco->prepare);
	else
		coro_resume(&evco->coro);

	return rc;
}

void
cprepare_coro_finish(struct cprepare_coro *evco)
{
	assert(evco);

	/* Will call cprepare_coro_finalizer_cb */
	coro_finish(&evco->coro);
}

