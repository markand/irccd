/*
 * nce/child.c -- coroutine watcher support for ev_child
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
nce_child_wait(struct nce_child *ev)
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
nce_child_coro_spawn(EV_P_ struct nce_child_coro *evco, pid_t pid, int trace)
{
	assert(evco);

	int rc;

	ev_init(&evco->child.child, nce_child_cb);
	ev_set_priority(&evco->child.child, -1);

	if (!(evco->coro.flags & NCE_INACTIVE)) {
		nce_child_set(&evco->child, pid, trace);
		nce_child_start(EV_A_ &evco->child);
	}

	if ((rc = nce_coro_create(EV_A_ &evco->coro)) < 0)
		nce_child_stop(EV_A_ &evco->child);
	else
		nce_coro_resume(&evco->coro);

	return rc;
}
void
nce_child_coro_destroy(EV_P_ struct nce_child_coro *evco)
{
	assert(evco);

	nce_child_stop(EV_A_ &evco->child);
	nce_coro_destroy(&evco->coro);
}

void
nce_child_coro_terminate(EV_P_ struct nce_coro *self)
{
	struct nce_child_coro *evco = NCE_CHILD_CORO(self, coro);

	nce_child_stop(EV_A_ &evco->child);
}
