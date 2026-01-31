/*
 * nce/idle.c -- coroutine watcher support for ev_idle
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

#include "idle.h"

static void
nce_idle_cb(EV_P_ struct ev_idle *self, int revents)
{
	struct nce_idle *ev = NCE_IDLE(self, idle);

	if (revents & (EV_IDLE)) {
#if !defined(NDEBUG)
		/*
		 * Make sure user fetched events through the nce_idle_wait
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
nce_idle_start(EV_P_ struct nce_idle *ev)
{
	assert(ev);

	if (ev->idle.active)
		return;

	ev_init(&ev->idle, nce_idle_cb);
	ev_idle_start(EV_A_ &ev->idle);
}

int
nce_idle_active(const struct nce_idle *ev)
{
	assert(ev);

	return ev->idle.active;
}

void
nce_idle_feed(EV_P_ struct nce_idle *ev, int events)
{
	assert(ev);

	ev_feed_event(EV_A_ &ev->idle, events);
}

void
nce_idle_stop(EV_P_ struct nce_idle *ev)
{
	assert(ev);

	ev->revents = 0;
	ev_idle_stop(EV_A_ &ev->idle);
}

int
nce_idle_ready(struct nce_idle *ev)
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
nce_idle_wait(struct nce_idle *ev)
{
	assert(ev);

	int rc = 0;

	while (!(rc = nce_idle_ready(ev)))
		nce_coro_yield();

	return rc;
}

int
nce_idle_coro_spawn(EV_P_ struct nce_idle_coro *evco)
{
	assert(evco);

	int rc;

	ev_init(&evco->idle.idle, nce_idle_cb);
	ev_set_priority(&evco->idle.idle, -1);

	if (!(evco->coro.flags & NCE_INACTIVE))
		nce_idle_start(EV_A_ &evco->idle);

	if ((rc = nce_coro_create(EV_A_ &evco->coro)) < 0)
		nce_idle_stop(EV_A_ &evco->idle);
	else
		nce_coro_resume(&evco->coro);

	return rc;
}
void
nce_idle_coro_destroy(EV_P_ struct nce_idle_coro *evco)
{
	assert(evco);

	nce_idle_stop(EV_A_ &evco->idle);
	nce_coro_destroy(&evco->coro);
}

void
nce_idle_coro_terminate(EV_P_ struct nce_coro *self)
{
	struct nce_idle_coro *evco = NCE_IDLE_CORO(self, coro);

	nce_idle_stop(EV_A_ &evco->idle);
}
