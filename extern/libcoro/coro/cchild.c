/*
 * cchild.c -- coroutine watcher support for ev_child
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

#include "cchild.h"

static void
cchild_cb(EV_P_ struct ev_child *self, int revents)
{
	struct cchild *ev = CCHILD(self, child);

	if (revents & (EV_CHILD)) {
#if !defined(NDEBUG)
		/*
		 * Make sure user fetched events through the cchild_wait
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
cchild_coro_entry_cb(EV_P_ struct coro *self)
{
	struct cchild_coro *evco = CORO_CONTAINER_OF(self, struct cchild_coro, coro);

	evco->entry(EV_A_ &evco->child);
}

static void
cchild_coro_finalizer_cb(EV_P_ struct coro *self)
{
	struct cchild_coro *evco = CORO_CONTAINER_OF(self, struct cchild_coro, coro);

	/* Stop the watcher for convenience. */
	cchild_stop(EV_A_ &evco->child);
	cchild_finish(&evco->child);

	/* Call user as very last function. */
	if (evco->finalizer)
		evco->finalizer(EV_A_ &evco->child);
}

void
cchild_init(struct cchild *ev)
{
	assert(ev);

	ev->revents = 0;
	ev->child = (const struct ev_child) {};
	ev_init(&ev->child, cchild_cb);
}

void
cchild_start(EV_P_ struct cchild *ev)
{
	assert(ev);

	ev_child_start(EV_A_ &ev->child);
}

int
cchild_active(const struct cchild *ev)
{
	assert(ev);

	return ev->child.active;
}

void
cchild_feed(EV_P_ struct cchild *ev, int events)
{
	assert(ev);

	ev_feed_event(EV_A_ &ev->child, events);
}

void
cchild_stop(EV_P_ struct cchild *ev)
{
	assert(ev);

	ev_child_stop(EV_A_ &ev->child);
}

int
cchild_ready(struct cchild *ev)
{
	assert(ev);

	int rc = 0;

	if (ev->revents & (EV_CHILD)) {
		rc = ev->revents;
		ev->revents = 0;
	}

	return rc;
}

int
cchild_wait(EV_P_ struct cchild *ev)
{
	assert(ev);

	int rc = 0;

	while (!(rc = cchild_ready(ev)))
		coro_yield();

	return rc;
}

void
cchild_set(struct cchild *ev, pid_t pid, int trace)
{
	assert(ev);

	ev_child_set(&ev->child, pid, trace);
}

void
cchild_finish(struct cchild *ev)
{
	assert(ev);
	assert(ev->child.active == 0);

	ev->revents = 0;
}

void
cchild_coro_init(struct cchild_coro *evco)
{
	assert(evco);

	cchild_init(&evco->child);

	coro_init(&evco->coro);
	coro_set_entry(&evco->coro, cchild_coro_entry_cb);
	coro_set_finalizer(&evco->coro, cchild_coro_finalizer_cb);

	evco->entry = NULL;
	evco->finalizer = NULL;
}

int
cchild_coro_spawn(EV_P_ struct cchild_coro *evco, const struct cchild_coro_def *def)
{
	assert(evco);
	assert(def);
	assert(def->entry);

	int rc;

	cchild_coro_init(evco);

	evco->entry = def->entry;
	evco->finalizer = def->finalizer;

	/*
	 * Watchers should be executed before attached coroutines to allow
	 * resuming them if an event happened.
	 */
	ev_set_priority(&evco->child.child, CORO_PRI_MAX - 1);
	cchild_set(&evco->child, def->pid, def->trace);

	/* Automatically start the watcher unless disabled. */
	if (!(def->flags & CORO_INACTIVE))
		cchild_start(EV_A_ &evco->child);

	/* All other fields are available for customization. */
	coro_set_name(&evco->coro, def->name);
	coro_set_stack_size(&evco->coro, def->stack_size);
	coro_set_flags(&evco->coro, def->flags);

	if ((rc = coro_create(EV_A_ &evco->coro)) < 0)
		cchild_stop(EV_A_ &evco->child);
	else
		coro_resume(&evco->coro);

	return rc;
}

void
cchild_coro_finish(struct cchild_coro *evco)
{
	assert(evco);

	/* Will call cchild_coro_finalizer_cb */
	coro_finish(&evco->coro);
}

