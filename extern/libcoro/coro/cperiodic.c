/*
 * cperiodic.c -- coroutine watcher support for ev_periodic
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

#include "cperiodic.h"

static void
cperiodic_cb(EV_P_ struct ev_periodic *self, int revents)
{
	struct cperiodic *ev = CPERIODIC(self, periodic);

	if (revents & (EV_PERIODIC)) {
#if !defined(NDEBUG)
		/*
		 * Make sure user fetched events through the cperiodic_wait
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
cperiodic_coro_entry_cb(EV_P_ struct coro *self)
{
	struct cperiodic_coro *evco = CORO_CONTAINER_OF(self, struct cperiodic_coro, coro);

	evco->entry(EV_A_ &evco->periodic);
}

static void
cperiodic_coro_finalizer_cb(EV_P_ struct coro *self)
{
	struct cperiodic_coro *evco = CORO_CONTAINER_OF(self, struct cperiodic_coro, coro);

	/* Stop the watcher for convenience. */
	cperiodic_stop(EV_A_ &evco->periodic);
	cperiodic_finish(&evco->periodic);

	/* Call user as very last function. */
	if (evco->finalizer)
		evco->finalizer(EV_A_ &evco->periodic);
}

void
cperiodic_init(struct cperiodic *ev)
{
	assert(ev);

	ev->revents = 0;
	ev->periodic = (const struct ev_periodic) {};
	ev_init(&ev->periodic, cperiodic_cb);
	ev->rescheduler = NULL;
}

void
cperiodic_start(EV_P_ struct cperiodic *ev)
{
	assert(ev);

	ev_periodic_start(EV_A_ &ev->periodic);
}

int
cperiodic_active(const struct cperiodic *ev)
{
	assert(ev);

	return ev->periodic.active;
}

void
cperiodic_feed(EV_P_ struct cperiodic *ev, int events)
{
	assert(ev);

	ev_feed_event(EV_A_ &ev->periodic, events);
}

void
cperiodic_stop(EV_P_ struct cperiodic *ev)
{
	assert(ev);

	ev_periodic_stop(EV_A_ &ev->periodic);
}

int
cperiodic_ready(struct cperiodic *ev)
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
cperiodic_wait(EV_P_ struct cperiodic *ev)
{
	assert(ev);

	int rc = 0;

	while (!(rc = cperiodic_ready(ev)))
		coro_yield();

	return rc;
}

static ev_tstamp
rescheduler_cb(struct ev_periodic *self, ev_tstamp now)
{
	struct cperiodic *ev = CPERIODIC(self, periodic);

	return ev->rescheduler(ev, now);
}

void
cperiodic_set(struct cperiodic *ev,
              ev_tstamp offset,
              ev_tstamp interval,
              cperiodic_rescheduler_t rescheduler
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

void
cperiodic_finish(struct cperiodic *ev)
{
	assert(ev);
	assert(ev->periodic.active == 0);

	ev->revents = 0;
	ev->rescheduler = NULL;
}

void
cperiodic_coro_init(struct cperiodic_coro *evco)
{
	assert(evco);

	cperiodic_init(&evco->periodic);

	coro_init(&evco->coro);
	coro_set_entry(&evco->coro, cperiodic_coro_entry_cb);
	coro_set_finalizer(&evco->coro, cperiodic_coro_finalizer_cb);

	evco->entry = NULL;
	evco->finalizer = NULL;
}

int
cperiodic_coro_spawn(EV_P_ struct cperiodic_coro *evco, const struct cperiodic_coro_def *def)
{
	assert(evco);
	assert(def);
	assert(def->entry);

	int rc;

	cperiodic_coro_init(evco);

	evco->entry = def->entry;
	evco->finalizer = def->finalizer;

	/*
	 * Watchers should be executed before attached coroutines to allow
	 * resuming them if an event happened.
	 */
	ev_set_priority(&evco->periodic.periodic, CORO_PRI_MAX - 1);
	cperiodic_set(&evco->periodic, def->offset, def->interval, def->rescheduler);

	/* Automatically start the watcher unless disabled. */
	if (!(def->flags & CORO_INACTIVE))
		cperiodic_start(EV_A_ &evco->periodic);

	/* All other fields are available for customization. */
	coro_set_name(&evco->coro, def->name);
	coro_set_stack_size(&evco->coro, def->stack_size);
	coro_set_flags(&evco->coro, def->flags);

	if ((rc = coro_create(EV_A_ &evco->coro)) < 0)
		cperiodic_stop(EV_A_ &evco->periodic);
	else
		coro_resume(&evco->coro);

	return rc;
}

void
cperiodic_coro_finish(struct cperiodic_coro *evco)
{
	assert(evco);

	/* Will call cperiodic_coro_finalizer_cb */
	coro_finish(&evco->coro);
}

