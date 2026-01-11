/*
 * nce_child.c -- coroutine watcher support for ev_child
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

#include "child.h"

static void
nce_child_cb(EV_P_ struct ev_child *self, int revents)
{
	struct nce_child *ev = NCE_CHILD(self, child);

	if (revents & (EV_CHILD)) {
#if !defined(NDEBUG)
		/*
		 * Make sure user fetched events through the nce_child_wait
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
nce_child_coro_entry_cb(EV_P_ struct nce_coro *self)
{
	struct nce_child_coro *evco = NCE_CONTAINER_OF(self, struct nce_child_coro, coro);

	evco->entry(EV_A_ &evco->child);
}

static void
nce_child_coro_finalizer_cb(EV_P_ struct nce_coro *self)
{
	struct nce_child_coro *evco = NCE_CONTAINER_OF(self, struct nce_child_coro, coro);

	/* Stop the watcher for convenience. */
	nce_child_stop(EV_A_ &evco->child);

	/* Call user as very last function. */
	if (evco->finalizer)
		evco->finalizer(EV_A_ &evco->child);
}

void
nce_child_start(EV_P_ struct nce_child *ev)
{
	assert(ev);

	if (ev->child.active)
		return;

	ev_init(&ev->child, nce_child_cb);
	ev_child_start(EV_A_ &ev->child);
}

int
nce_child_active(const struct nce_child *ev)
{
	assert(ev);

	return ev->child.active;
}

void
nce_child_feed(EV_P_ struct nce_child *ev, int events)
{
	assert(ev);

	ev_feed_event(EV_A_ &ev->child, events);
}

void
nce_child_stop(EV_P_ struct nce_child *ev)
{
	assert(ev);

	ev->revents = 0;
	ev_child_stop(EV_A_ &ev->child);
}

int
nce_child_ready(struct nce_child *ev)
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
nce_child_wait(EV_P_ struct nce_child *ev)
{
	assert(ev);

	int rc = 0;

	while (!(rc = nce_child_ready(ev)))
		nce_coro_yield();

	return rc;
}

void
nce_child_set(struct nce_child *ev, pid_t pid, int trace)
{
	assert(ev);

	ev_child_set(&ev->child, pid, trace);
}

int
nce_child_coro_spawn(EV_P_ struct nce_child_coro *evco, const struct nce_child_coro_args *args)
{
	assert(evco);
	assert(evco->entry);

	(void)args;

	int rc;

	evco->coro.entry = nce_child_coro_entry_cb;

	if (!evco->coro.finalizer)
		evco->coro.finalizer = nce_child_coro_finalizer_cb;

	/*
	 * Watchers should be executed before attached coroutines to allow
	 * resuming them if an event happened.
	 */
	ev_set_priority(&evco->child.child, NCE_PRI_MAX - 1);
	if (args)
		nce_child_set(&evco->child, args->pid, args->trace);
	else
		evco->coro.flags |= NCE_CORO_INACTIVE;

	/* Automatically start the watcher unless disabled. */
	if (!(evco->coro.flags & NCE_CORO_INACTIVE))
		nce_child_start(EV_A_ &evco->child);

	if ((rc = nce_coro_create(EV_A_ &evco->coro)) < 0)
		nce_child_stop(EV_A_ &evco->child);
	else
		nce_coro_resume(&evco->coro);

	return rc;
}

void
nce_child_coro_destroy(struct nce_child_coro *evco)
{
	assert(evco);

	/* Will call nce_child_coro_finalizer_cb */
	nce_coro_destroy(&evco->coro);
}
