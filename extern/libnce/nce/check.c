/*
 * nce_check.c -- coroutine watcher support for ev_check
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

#include "check.h"

static void
nce_check_cb(EV_P_ struct ev_check *self, int revents)
{
	struct nce_check *ev = NCE_CHECK(self, check);

	if (revents & (EV_CHECK)) {
#if !defined(NDEBUG)
		/*
		 * Make sure user fetched events through the nce_check_wait
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
nce_check_coro_entry_cb(EV_P_ struct nce_coro *self)
{
	struct nce_check_coro *evco = NCE_CONTAINER_OF(self, struct nce_check_coro, coro);

	evco->entry(EV_A_ &evco->check);
}

static void
nce_check_coro_finalizer_cb(EV_P_ struct nce_coro *self)
{
	struct nce_check_coro *evco = NCE_CONTAINER_OF(self, struct nce_check_coro, coro);

	/* Stop the watcher for convenience. */
	nce_check_stop(EV_A_ &evco->check);

	/* Call user as very last function. */
	if (evco->finalizer)
		evco->finalizer(EV_A_ &evco->check);
}

void
nce_check_start(EV_P_ struct nce_check *ev)
{
	assert(ev);

	if (ev->check.active)
		return;

	ev_init(&ev->check, nce_check_cb);
	ev_check_start(EV_A_ &ev->check);
}

int
nce_check_active(const struct nce_check *ev)
{
	assert(ev);

	return ev->check.active;
}

void
nce_check_feed(EV_P_ struct nce_check *ev, int events)
{
	assert(ev);

	ev_feed_event(EV_A_ &ev->check, events);
}

void
nce_check_stop(EV_P_ struct nce_check *ev)
{
	assert(ev);

	ev->revents = 0;
	ev_check_stop(EV_A_ &ev->check);
}

int
nce_check_ready(struct nce_check *ev)
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
nce_check_wait(EV_P_ struct nce_check *ev)
{
	assert(ev);

	int rc = 0;

	while (!(rc = nce_check_ready(ev)))
		nce_coro_yield();

	return rc;
}

int
nce_check_coro_spawn(EV_P_ struct nce_check_coro *evco, const struct nce_check_coro_args *args)
{
	assert(evco);
	assert(evco->entry);

	(void)args;

	int rc;

	evco->coro.entry = nce_check_coro_entry_cb;

	if (!evco->coro.finalizer)
		evco->coro.finalizer = nce_check_coro_finalizer_cb;

	/*
	 * Watchers should be executed before attached coroutines to allow
	 * resuming them if an event happened.
	 */
	ev_set_priority(&evco->check.check, NCE_PRI_MAX - 1);
	/* Automatically start the watcher unless disabled. */
	if (!(evco->coro.flags & NCE_CORO_INACTIVE))
		nce_check_start(EV_A_ &evco->check);

	if ((rc = nce_coro_create(EV_A_ &evco->coro)) < 0)
		nce_check_stop(EV_A_ &evco->check);
	else
		nce_coro_resume(&evco->coro);

	return rc;
}

void
nce_check_coro_destroy(struct nce_check_coro *evco)
{
	assert(evco);

	/* Will call nce_check_coro_finalizer_cb */
	nce_coro_destroy(&evco->coro);
}
