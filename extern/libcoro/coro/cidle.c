/*
 * cidle.c -- coroutine watcher support for ev_idle
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

#include "cidle.h"

static void
cidle_cb(EV_P_ struct ev_idle *self, int revents)
{
	struct cidle *ev = CIDLE(self, idle);

	if (revents & (EV_IDLE)) {
#if !defined(NDEBUG)
		/*
		 * Make sure user fetched events through the cidle_wait
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
cidle_coro_entry_cb(EV_P_ struct coro *self)
{
	struct cidle_coro *evco = CORO_CONTAINER_OF(self, struct cidle_coro, coro);

	evco->entry(EV_A_ &evco->idle);
}

static void
cidle_coro_finalizer_cb(EV_P_ struct coro *self)
{
	struct cidle_coro *evco = CORO_CONTAINER_OF(self, struct cidle_coro, coro);

	/* Stop the watcher for convenience. */
	cidle_stop(EV_A_ &evco->idle);
	cidle_finish(&evco->idle);

	/* Call user as very last function. */
	if (evco->finalizer)
		evco->finalizer(EV_A_ &evco->idle);
}

void
cidle_init(struct cidle *ev)
{
	assert(ev);

	ev->revents = 0;
	ev->idle = (const struct ev_idle) {};
	ev_init(&ev->idle, cidle_cb);
}

void
cidle_start(EV_P_ struct cidle *ev)
{
	assert(ev);

	ev_idle_start(EV_A_ &ev->idle);
}

int
cidle_active(const struct cidle *ev)
{
	assert(ev);

	return ev->idle.active;
}

void
cidle_feed(EV_P_ struct cidle *ev, int events)
{
	assert(ev);

	ev_feed_event(EV_A_ &ev->idle, events);
}

void
cidle_stop(EV_P_ struct cidle *ev)
{
	assert(ev);

	ev_idle_stop(EV_A_ &ev->idle);
}

int
cidle_ready(struct cidle *ev)
{
	assert(ev);

	int rc = 0;

	if (ev->revents & (EV_IDLE)) {
		rc = ev->revents;
		ev->revents = 0;
	}

	return rc;
}

int
cidle_wait(EV_P_ struct cidle *ev)
{
	assert(ev);

	int rc = 0;

	while (!(rc = cidle_ready(ev)))
		coro_yield();

	return rc;
}

void
cidle_finish(struct cidle *ev)
{
	assert(ev);
	assert(ev->idle.active == 0);

	ev->revents = 0;
}

void
cidle_coro_init(struct cidle_coro *evco)
{
	assert(evco);

	cidle_init(&evco->idle);

	coro_init(&evco->coro);
	coro_set_entry(&evco->coro, cidle_coro_entry_cb);
	coro_set_finalizer(&evco->coro, cidle_coro_finalizer_cb);

	evco->entry = NULL;
	evco->finalizer = NULL;
}

int
cidle_coro_spawn(EV_P_ struct cidle_coro *evco, const struct cidle_coro_def *def)
{
	assert(evco);
	assert(def);
	assert(def->entry);

	int rc;

	cidle_coro_init(evco);

	evco->entry = def->entry;
	evco->finalizer = def->finalizer;

	/*
	 * Watchers should be executed before attached coroutines to allow
	 * resuming them if an event happened.
	 */
	ev_set_priority(&evco->idle.idle, CORO_PRI_MAX - 1);

	/* Automatically start the watcher unless disabled. */
	if (!(def->flags & CORO_INACTIVE))
		cidle_start(EV_A_ &evco->idle);

	/* All other fields are available for customization. */
	coro_set_name(&evco->coro, def->name);
	coro_set_stack_size(&evco->coro, def->stack_size);
	coro_set_flags(&evco->coro, def->flags);

	if ((rc = coro_create(EV_A_ &evco->coro)) < 0)
		cidle_stop(EV_A_ &evco->idle);
	else
		coro_resume(&evco->coro);

	return rc;
}

void
cidle_coro_finish(struct cidle_coro *evco)
{
	assert(evco);

	/* Will call cidle_coro_finalizer_cb */
	coro_finish(&evco->coro);
}

