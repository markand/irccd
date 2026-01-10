/*
 * ccheck.c -- coroutine watcher support for ev_check
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

#include "ccheck.h"

static void
ccheck_cb(EV_P_ struct ev_check *self, int revents)
{
	struct ccheck *ev = CCHECK(self, check);

	if (revents & (EV_CHECK)) {
#if !defined(NDEBUG)
		/*
		 * Make sure user fetched events through the ccheck_wait
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
ccheck_coro_entry_cb(EV_P_ struct coro *self)
{
	struct ccheck_coro *evco = CORO_CONTAINER_OF(self, struct ccheck_coro, coro);

	evco->entry(EV_A_ &evco->check);
}

static void
ccheck_coro_finalizer_cb(EV_P_ struct coro *self)
{
	struct ccheck_coro *evco = CORO_CONTAINER_OF(self, struct ccheck_coro, coro);

	/* Stop the watcher for convenience. */
	ccheck_stop(EV_A_ &evco->check);
	ccheck_finish(&evco->check);

	/* Call user as very last function. */
	if (evco->finalizer)
		evco->finalizer(EV_A_ &evco->check);
}

void
ccheck_init(struct ccheck *ev)
{
	assert(ev);

	ev->revents = 0;
	ev->check = (const struct ev_check) {};
	ev_init(&ev->check, ccheck_cb);
}

void
ccheck_start(EV_P_ struct ccheck *ev)
{
	assert(ev);

	ev_check_start(EV_A_ &ev->check);
}

int
ccheck_active(const struct ccheck *ev)
{
	assert(ev);

	return ev->check.active;
}

void
ccheck_feed(EV_P_ struct ccheck *ev, int events)
{
	assert(ev);

	ev_feed_event(EV_A_ &ev->check, events);
}

void
ccheck_stop(EV_P_ struct ccheck *ev)
{
	assert(ev);

	ev_check_stop(EV_A_ &ev->check);
}

int
ccheck_ready(struct ccheck *ev)
{
	assert(ev);

	int rc = 0;

	if (ev->revents & (EV_CHECK)) {
		rc = ev->revents;
		ev->revents = 0;
	}

	return rc;
}

int
ccheck_wait(EV_P_ struct ccheck *ev)
{
	assert(ev);

	int rc = 0;

	while (!(rc = ccheck_ready(ev)))
		coro_yield();

	return rc;
}

void
ccheck_finish(struct ccheck *ev)
{
	assert(ev);
	assert(ev->check.active == 0);

	ev->revents = 0;
}

void
ccheck_coro_init(struct ccheck_coro *evco)
{
	assert(evco);

	ccheck_init(&evco->check);

	coro_init(&evco->coro);
	coro_set_entry(&evco->coro, ccheck_coro_entry_cb);
	coro_set_finalizer(&evco->coro, ccheck_coro_finalizer_cb);

	evco->entry = NULL;
	evco->finalizer = NULL;
}

int
ccheck_coro_spawn(EV_P_ struct ccheck_coro *evco, const struct ccheck_coro_def *def)
{
	assert(evco);
	assert(def);
	assert(def->entry);

	int rc;

	ccheck_coro_init(evco);

	evco->entry = def->entry;
	evco->finalizer = def->finalizer;

	/*
	 * Watchers should be executed before attached coroutines to allow
	 * resuming them if an event happened.
	 */
	ev_set_priority(&evco->check.check, CORO_PRI_MAX - 1);

	/* Automatically start the watcher unless disabled. */
	if (!(def->flags & CORO_INACTIVE))
		ccheck_start(EV_A_ &evco->check);

	/* All other fields are available for customization. */
	coro_set_name(&evco->coro, def->name);
	coro_set_stack_size(&evco->coro, def->stack_size);
	coro_set_flags(&evco->coro, def->flags);

	if ((rc = coro_create(EV_A_ &evco->coro)) < 0)
		ccheck_stop(EV_A_ &evco->check);
	else
		coro_resume(&evco->coro);

	return rc;
}

void
ccheck_coro_finish(struct ccheck_coro *evco)
{
	assert(evco);

	/* Will call ccheck_coro_finalizer_cb */
	coro_finish(&evco->coro);
}

