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

static inline void
cidle_coro_finalizer(struct cidle_coro *coro)
{
#if EV_MULTIPLICITY
	if (coro->coro.loop)
		cidle_stop(coro->coro.loop, &coro->idle);
#else
	cidle_stop(&coro->idle);
#endif

	cidle_finish(&coro->idle);
}

static void
cidle_coro_entry_cb(EV_P_ struct coro *self)
{
	struct cidle_coro *coro = CORO_CONTAINER_OF(self, struct cidle_coro, coro);

	coro->entry(EV_A_ &coro->idle);
}

static void
cidle_coro_finalizer_cb(EV_P_ struct coro *self)
{
	struct cidle_coro *coro = CORO_CONTAINER_OF(self, struct cidle_coro, coro);

	cidle_coro_finalizer(coro);
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

int
cidle_coro_spawn(EV_P_ struct cidle_coro *coro, const struct cidle_coro_def *def)
{
	assert(coro);
	assert(def);

	int rc;

	coro->entry = def->entry;
	cidle_init(&coro->idle);

	/*
	 * Watchers should be executed before attached coroutines to allow
	 * resuming them if an event happened.
	 */
	ev_set_priority(&coro->idle.idle, CORO_PRI_MAX - 1);

	/* Automatically start the watcher unless disabled. */
	if (!(def->flags & CORO_INACTIVE))
		cidle_start(EV_A_ &coro->idle);

	coro_init(&coro->coro);

	/* All other fields are available for customization. */
	coro_set_name(&coro->coro, def->name);
	coro_set_stack_size(&coro->coro, def->stack_size);
	coro_set_flags(&coro->coro, def->flags);
	coro_set_entry(&coro->coro, cidle_coro_entry_cb);

	/*
	 * Use a default finalizer if NULL to conveniently stop and destroy the
	 * underlying watcher.
	 */
	if (!def->finalizer)
		coro_set_finalizer(&coro->coro, cidle_coro_finalizer_cb);
	else
		coro_set_finalizer(&coro->coro, def->finalizer);

	if ((rc = coro_create(EV_A_ &coro->coro)) < 0)
		cidle_stop(EV_A_ &coro->idle);
	else
		coro_resume(&coro->coro);

	return rc;
}
