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

static inline void
cchild_coro_finalizer(struct cchild_coro *coro)
{
#if EV_MULTIPLICITY
	if (coro->coro.loop)
		cchild_stop(coro->coro.loop, &coro->child);
#else
	cchild_stop(&coro->child);
#endif

	cchild_finish(&coro->child);
}

static void
cchild_coro_entry_cb(EV_P_ struct coro *self)
{
	struct cchild_coro *coro = CORO_CONTAINER_OF(self, struct cchild_coro, coro);

	coro->entry(EV_A_ &coro->child);
}

static void
cchild_coro_finalizer_cb(EV_P_ struct coro *self)
{
	struct cchild_coro *coro = CORO_CONTAINER_OF(self, struct cchild_coro, coro);

	cchild_coro_finalizer(coro);
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

int
cchild_coro_spawn(EV_P_ struct cchild_coro *coro, const struct cchild_coro_def *def)
{
	assert(coro);
	assert(def);

	int rc;

	coro->entry = def->entry;
	cchild_init(&coro->child);

	/*
	 * Watchers should be executed before attached coroutines to allow
	 * resuming them if an event happened.
	 */
	ev_set_priority(&coro->child.child, CORO_PRI_MAX - 1);
	cchild_set(&coro->child, def->pid, def->trace);

	/* Automatically start the watcher unless disabled. */
	if (!(def->flags & CORO_INACTIVE))
		cchild_start(EV_A_ &coro->child);

	coro_init(&coro->coro);

	/* All other fields are available for customization. */
	coro_set_name(&coro->coro, def->name);
	coro_set_stack_size(&coro->coro, def->stack_size);
	coro_set_flags(&coro->coro, def->flags);
	coro_set_entry(&coro->coro, cchild_coro_entry_cb);

	/*
	 * Use a default finalizer if NULL to conveniently stop and destroy the
	 * underlying watcher.
	 */
	if (!def->finalizer)
		coro_set_finalizer(&coro->coro, cchild_coro_finalizer_cb);
	else
		coro_set_finalizer(&coro->coro, def->finalizer);

	if ((rc = coro_create(EV_A_ &coro->coro)) < 0)
		cchild_stop(EV_A_ &coro->child);
	else
		coro_resume(&coro->coro);

	return rc;
}
