/*
 * nce_periodic.c -- coroutine watcher support for ev_periodic
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

#include "periodic.h"

static void
nce_periodic_cb(EV_P_ struct ev_periodic *self, int revents)
{
	struct nce_periodic *ev = NCE_PERIODIC(self, periodic);

	if (revents & (EV_PERIODIC)) {
#if !defined(NDEBUG)
		/*
		 * Make sure user fetched events through the nce_periodic_wait
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
nce_periodic_coro_entry_cb(EV_P_ struct nce_coro *self)
{
	struct nce_periodic_coro *evco = NCE_CONTAINER_OF(self, struct nce_periodic_coro, coro);

	evco->entry(EV_A_ &evco->periodic);
}

static void
nce_periodic_coro_finalizer_cb(EV_P_ struct nce_coro *self)
{
	struct nce_periodic_coro *evco = NCE_CONTAINER_OF(self, struct nce_periodic_coro, coro);

	/* Stop the watcher for convenience. */
	nce_periodic_stop(EV_A_ &evco->periodic);

	/* Call user as very last function. */
	if (evco->finalizer)
		evco->finalizer(EV_A_ &evco->periodic);
}

void
nce_periodic_start(EV_P_ struct nce_periodic *ev)
{
	assert(ev);

	if (ev->periodic.active)
		return;

	ev_init(&ev->periodic, nce_periodic_cb);
	ev_periodic_start(EV_A_ &ev->periodic);
}

int
nce_periodic_active(const struct nce_periodic *ev)
{
	assert(ev);

	return ev->periodic.active;
}

void
nce_periodic_feed(EV_P_ struct nce_periodic *ev, int events)
{
	assert(ev);

	ev_feed_event(EV_A_ &ev->periodic, events);
}

void
nce_periodic_stop(EV_P_ struct nce_periodic *ev)
{
	assert(ev);

	ev->revents = 0;
	ev_periodic_stop(EV_A_ &ev->periodic);
}

int
nce_periodic_ready(struct nce_periodic *ev)
{
	assert(ev);

	int rc = 0;

	if (ev->revents & (EV_PERIODIC)) {
		rc = ev->revents;
		ev->revents = 0;
	}

	return rc;
}

int
nce_periodic_wait(EV_P_ struct nce_periodic *ev)
{
	assert(ev);

	int rc = 0;

	while (!(rc = nce_periodic_ready(ev)))
		nce_coro_yield();

	return rc;
}

static ev_tstamp
rescheduler_cb(struct ev_periodic *self, ev_tstamp now)
{
	struct nce_periodic *ev = NCE_PERIODIC(self, periodic);

	return ev->rescheduler(ev, now);
}

void
nce_periodic_set(struct nce_periodic *ev,
                 ev_tstamp offset,
                 ev_tstamp interval,
                 nce_periodic_rescheduler_t rescheduler
)
{
	assert(ev);
	assert(ev->periodic.active == 0);

	ev->rescheduler = rescheduler;

	if (rescheduler)
		ev_periodic_set(&ev->periodic, offset, interval, rescheduler_cb);
	else
		ev_periodic_set(&ev->periodic, offset, interval, NULL);
}

int
nce_periodic_coro_spawn(EV_P_ struct nce_periodic_coro *evco, const struct nce_periodic_coro_args *args)
{
	assert(evco);
	assert(evco->entry);

	(void)args;

	int rc;

	evco->coro.entry = nce_periodic_coro_entry_cb;

	if (!evco->coro.finalizer)
		evco->coro.finalizer = nce_periodic_coro_finalizer_cb;

	/*
	 * Watchers should be executed before attached coroutines to allow
	 * resuming them if an event happened.
	 */
	ev_set_priority(&evco->periodic.periodic, NCE_PRI_MAX - 1);

	if (args)
		nce_periodic_set(&evco->periodic,
		                  args->offset,
		                  args->interval,
		                  evco->periodic.rescheduler);
	else
		evco->coro.flags |= NCE_CORO_INACTIVE;

	/* Automatically start the watcher unless disabled. */
	if (!(evco->coro.flags & NCE_CORO_INACTIVE))
		nce_periodic_start(EV_A_ &evco->periodic);

	if ((rc = nce_coro_create(EV_A_ &evco->coro)) < 0)
		nce_periodic_stop(EV_A_ &evco->periodic);
	else
		nce_coro_resume(&evco->coro);

	return rc;
}

void
nce_periodic_coro_destroy(struct nce_periodic_coro *evco)
{
	assert(evco);

	/* Will call nce_periodic_coro_finalizer_cb */
	nce_coro_destroy(&evco->coro);
}
