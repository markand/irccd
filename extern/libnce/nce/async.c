/*
 * nce/async.c -- coroutine watcher support for ev_async
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

#include "async.h"

static void
nce_async_cb(EV_P_ struct ev_async *self, int revents)
{
	struct nce_async *ev = NCE_ASYNC(self, async);

	if (revents & (EV_ASYNC)) {
#if !defined(NDEBUG)
		/*
		 * Make sure user fetched events through the nce_async_wait
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
nce_async_start(EV_P_ struct nce_async *ev)
{
	assert(ev);

	if (ev->async.active)
		return;

	ev_init(&ev->async, nce_async_cb);
	ev_async_start(EV_A_ &ev->async);
}

int
nce_async_active(const struct nce_async *ev)
{
	assert(ev);

	return ev->async.active;
}

void
nce_async_feed(EV_P_ struct nce_async *ev, int events)
{
	assert(ev);

	ev_feed_event(EV_A_ &ev->async, events);
}

void
nce_async_stop(EV_P_ struct nce_async *ev)
{
	assert(ev);

	ev->revents = 0;
	ev_async_stop(EV_A_ &ev->async);
}

int
nce_async_ready(struct nce_async *ev)
{
	assert(ev);

	int rc = 0;

	if (ev->revents & (EV_ASYNC)) {
		rc = ev->revents;
		ev->revents = 0;
	}

	return rc;
}

int
nce_async_wait(struct nce_async *ev)
{
	assert(ev);

	int rc = 0;

	while (!(rc = nce_async_ready(ev)))
		nce_coro_yield();

	return rc;
}

int
nce_async_coro_spawn(EV_P_ struct nce_async_coro *evco)
{
	assert(evco);

	int rc;

	ev_init(&evco->async.async, nce_async_cb);
	ev_set_priority(&evco->async.async, -1);

	if (!(evco->coro.flags & NCE_INACTIVE))
		nce_async_start(EV_A_ &evco->async);

	if ((rc = nce_coro_create(EV_A_ &evco->coro)) < 0)
		nce_async_stop(EV_A_ &evco->async);
	else
		nce_coro_resume(&evco->coro);

	return rc;
}

void
nce_async_coro_destroy(EV_P_ struct nce_async_coro *evco)
{
	assert(evco);

	nce_async_stop(EV_A_ &evco->async);
	nce_coro_destroy(&evco->coro);
}

void
nce_async_coro_terminate(EV_P_ struct nce_coro *self)
{
	struct nce_async_coro *evco = NCE_ASYNC_CORO(self, coro);

	nce_async_stop(EV_A_ &evco->async);
}
