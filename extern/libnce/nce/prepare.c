/*
 * nce_prepare.c -- coroutine watcher support for ev_prepare
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

static void
nce_prepare_coro_entry_cb(EV_P_ struct nce_coro *self)
{
	struct nce_prepare_coro *evco = NCE_CONTAINER_OF(self, struct nce_prepare_coro, coro);

	evco->entry(EV_A_ &evco->prepare);
}

static void
nce_prepare_coro_finalizer_cb(EV_P_ struct nce_coro *self)
{
	struct nce_prepare_coro *evco = NCE_CONTAINER_OF(self, struct nce_prepare_coro, coro);

	/* Stop the watcher for convenience. */
	nce_prepare_stop(EV_A_ &evco->prepare);

	/* Call user as very last function. */
	if (evco->finalizer)
		evco->finalizer(EV_A_ &evco->prepare);
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
nce_prepare_wait(EV_P_ struct nce_prepare *ev)
{
	assert(ev);

	int rc = 0;

	while (!(rc = nce_prepare_ready(ev)))
		nce_coro_yield();

	return rc;
}

int
nce_prepare_coro_spawn(EV_P_ struct nce_prepare_coro *evco, const struct nce_prepare_coro_args *args)
{
	assert(evco);
	assert(evco->entry);

	(void)args;

	int rc;

	evco->coro.entry = nce_prepare_coro_entry_cb;

	if (!evco->coro.finalizer)
		evco->coro.finalizer = nce_prepare_coro_finalizer_cb;

	/*
	 * Watchers should be executed before attached coroutines to allow
	 * resuming them if an event happened.
	 */
	ev_set_priority(&evco->prepare.prepare, NCE_PRI_MAX - 1);
	/* Automatically start the watcher unless disabled. */
	if (!(evco->coro.flags & NCE_CORO_INACTIVE))
		nce_prepare_start(EV_A_ &evco->prepare);

	if ((rc = nce_coro_create(EV_A_ &evco->coro)) < 0)
		nce_prepare_stop(EV_A_ &evco->prepare);
	else
		nce_coro_resume(&evco->coro);

	return rc;
}

void
nce_prepare_coro_destroy(struct nce_prepare_coro *evco)
{
	assert(evco);

	/* Will call nce_prepare_coro_finalizer_cb */
	nce_coro_destroy(&evco->coro);
}
