/*
 * nce/prepare.c -- coroutine watcher support for ev_prepare
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

#include "prepare.h"

static void
nce_prepare_cb(EV_P_ struct ev_prepare *self, int revents)
{
	struct nce_prepare *ev = NCE_PREPARE(self, prepare);

	if (revents & (EV_PREPARE)) {
#if !defined(NDEBUG)
		/*
		 * Make sure user fetched events through the nce_prepare_wait
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
nce_prepare_start(EV_P_ struct nce_prepare *ev)
{
	assert(ev);

	if (ev->prepare.active)
		return;

	ev_init(&ev->prepare, nce_prepare_cb);
	ev_prepare_start(EV_A_ &ev->prepare);
}

int
nce_prepare_active(const struct nce_prepare *ev)
{
	assert(ev);

	return ev->prepare.active;
}

void
nce_prepare_feed(EV_P_ struct nce_prepare *ev, int events)
{
	assert(ev);

	ev_feed_event(EV_A_ &ev->prepare, events);
}

void
nce_prepare_stop(EV_P_ struct nce_prepare *ev)
{
	assert(ev);

	ev->revents = 0;
	ev_prepare_stop(EV_A_ &ev->prepare);
}

int
nce_prepare_ready(struct nce_prepare *ev)
{
	assert(ev);

	int rc = 0;

	if (ev->revents & (EV_PREPARE)) {
		rc = ev->revents;
		ev->revents = 0;
	}

	return rc;
}

int
nce_prepare_wait(struct nce_prepare *ev)
{
	assert(ev);

	int rc = 0;

	while (!(rc = nce_prepare_ready(ev)))
		nce_coro_yield();

	return rc;
}

int
nce_prepare_coro_spawn(EV_P_ struct nce_prepare_coro *evco)
{
	assert(evco);

	int rc;

	ev_init(&evco->prepare.prepare, nce_prepare_cb);
	ev_set_priority(&evco->prepare.prepare, -1);

	if (!(evco->coro.flags & NCE_INACTIVE))
		nce_prepare_start(EV_A_ &evco->prepare);

	if ((rc = nce_coro_create(EV_A_ &evco->coro)) < 0)
		nce_prepare_stop(EV_A_ &evco->prepare);
	else
		nce_coro_resume(&evco->coro);

	return rc;
}
void
nce_prepare_coro_destroy(EV_P_ struct nce_prepare_coro *evco)
{
	assert(evco);

	nce_prepare_stop(EV_A_ &evco->prepare);
	nce_coro_destroy(&evco->coro);
}

void
nce_prepare_coro_terminate(EV_P_ struct nce_coro *self)
{
	struct nce_prepare_coro *evco = NCE_PREPARE_CORO(self, coro);

	nce_prepare_stop(EV_A_ &evco->prepare);
}
