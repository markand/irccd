/*
 * ctimer.c -- coroutine watcher support for ev_timer
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

#include "ctimer.h"

static void
ctimer_cb(EV_P_ struct ev_timer *self, int revents)
{
	struct ctimer *ev = CTIMER(self, timer);

	if (revents & (EV_TIMER)) {
#if !defined(NDEBUG)
		/*
		 * Make sure user fetched events through the ctimer_wait
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
ctimer_coro_entry_cb(EV_P_ struct coro *self)
{
	struct ctimer_coro *evco = CORO_CONTAINER_OF(self, struct ctimer_coro, coro);

	evco->entry(EV_A_ &evco->timer);
}

static void
ctimer_coro_finalizer_cb(EV_P_ struct coro *self)
{
	struct ctimer_coro *evco = CORO_CONTAINER_OF(self, struct ctimer_coro, coro);

	/* Stop the watcher for convenience. */
	ctimer_stop(EV_A_ &evco->timer);
	ctimer_finish(&evco->timer);

	/* Call user as very last function. */
	if (evco->finalizer)
		evco->finalizer(EV_A_ &evco->timer);
}

void
ctimer_init(struct ctimer *ev)
{
	assert(ev);

	ev->revents = 0;
	ev->timer = (const struct ev_timer) {};
	ev_init(&ev->timer, ctimer_cb);
}

void
ctimer_start(EV_P_ struct ctimer *ev)
{
	assert(ev);

	ev_timer_start(EV_A_ &ev->timer);
}

int
ctimer_active(const struct ctimer *ev)
{
	assert(ev);

	return ev->timer.active;
}

void
ctimer_feed(EV_P_ struct ctimer *ev, int events)
{
	assert(ev);

	ev_feed_event(EV_A_ &ev->timer, events);
}

void
ctimer_stop(EV_P_ struct ctimer *ev)
{
	assert(ev);

	ev_timer_stop(EV_A_ &ev->timer);
}

int
ctimer_ready(struct ctimer *ev)
{
	assert(ev);

	int rc = 0;

	if (ev->revents & (EV_TIMER)) {
		rc = ev->revents;
		ev->revents = 0;
	}

	return rc;
}

int
ctimer_wait(EV_P_ struct ctimer *ev)
{
	assert(ev);

	int rc = 0;

	while (!(rc = ctimer_ready(ev)))
		coro_yield();

	return rc;
}

void
ctimer_set(struct ctimer *ev, ev_tstamp after, ev_tstamp repeat)
{
	assert(ev);
	assert(ev->timer.active == 0);

	ev_timer_set(&ev->timer, after, repeat);
}

void
ctimer_restart(EV_P_ struct ctimer *ev, ev_tstamp after, ev_tstamp repeat)
{
	assert(ev);

	ev_timer_stop(EV_A_ &ev->timer);
	ev_timer_set(&ev->timer, after, repeat);
	ev_timer_start(EV_A_ &ev->timer);
}

void
ctimer_again(EV_P_ struct ctimer *ev)
{
	assert(ev);

	ev_timer_again(EV_A_ &ev->timer);
}

void
ctimer_finish(struct ctimer *ev)
{
	assert(ev);
	assert(ev->timer.active == 0);

	ev->revents = 0;
}

void
ctimer_coro_init(struct ctimer_coro *evco)
{
	assert(evco);

	ctimer_init(&evco->timer);

	coro_init(&evco->coro);
	coro_set_entry(&evco->coro, ctimer_coro_entry_cb);
	coro_set_finalizer(&evco->coro, ctimer_coro_finalizer_cb);

	evco->entry = NULL;
	evco->finalizer = NULL;
}

int
ctimer_coro_spawn(EV_P_ struct ctimer_coro *evco, const struct ctimer_coro_def *def)
{
	assert(evco);
	assert(def);
	assert(def->entry);

	int rc;

	ctimer_coro_init(evco);

	evco->entry = def->entry;
	evco->finalizer = def->finalizer;

	/*
	 * Watchers should be executed before attached coroutines to allow
	 * resuming them if an event happened.
	 */
	ev_set_priority(&evco->timer.timer, CORO_PRI_MAX - 1);
	ctimer_set(&evco->timer, def->after, def->repeat);

	/* Automatically start the watcher unless disabled. */
	if (!(def->flags & CORO_INACTIVE))
		ctimer_start(EV_A_ &evco->timer);

	/* All other fields are available for customization. */
	coro_set_name(&evco->coro, def->name);
	coro_set_stack_size(&evco->coro, def->stack_size);
	coro_set_flags(&evco->coro, def->flags);

	if ((rc = coro_create(EV_A_ &evco->coro)) < 0)
		ctimer_stop(EV_A_ &evco->timer);
	else
		coro_resume(&evco->coro);

	return rc;
}

void
ctimer_coro_finish(struct ctimer_coro *evco)
{
	assert(evco);

	/* Will call ctimer_coro_finalizer_cb */
	coro_finish(&evco->coro);
}

