/*
 * nce_signal.c -- coroutine watcher support for ev_signal
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

#include "signal.h"

static void
nce_signal_cb(EV_P_ struct ev_signal *self, int revents)
{
	struct nce_signal *ev = NCE_SIGNAL(self, signal);

	if (revents & (EV_SIGNAL)) {
#if !defined(NDEBUG)
		/*
		 * Make sure user fetched events through the nce_signal_wait
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
nce_signal_coro_entry_cb(EV_P_ struct nce_coro *self)
{
	struct nce_signal_coro *evco = NCE_CONTAINER_OF(self, struct nce_signal_coro, coro);

	evco->entry(EV_A_ &evco->signal);
}

static void
nce_signal_coro_finalizer_cb(EV_P_ struct nce_coro *self)
{
	struct nce_signal_coro *evco = NCE_CONTAINER_OF(self, struct nce_signal_coro, coro);

	/* Stop the watcher for convenience. */
	nce_signal_stop(EV_A_ &evco->signal);

	/* Call user as very last function. */
	if (evco->finalizer)
		evco->finalizer(EV_A_ &evco->signal);
}

void
nce_signal_start(EV_P_ struct nce_signal *ev)
{
	assert(ev);

	if (ev->signal.active)
		return;

	ev_init(&ev->signal, nce_signal_cb);
	ev_signal_start(EV_A_ &ev->signal);
}

int
nce_signal_active(const struct nce_signal *ev)
{
	assert(ev);

	return ev->signal.active;
}

void
nce_signal_feed(EV_P_ struct nce_signal *ev, int events)
{
	assert(ev);

	ev_feed_event(EV_A_ &ev->signal, events);
}

void
nce_signal_stop(EV_P_ struct nce_signal *ev)
{
	assert(ev);

	ev->revents = 0;
	ev_signal_stop(EV_A_ &ev->signal);
}

int
nce_signal_ready(struct nce_signal *ev)
{
	assert(ev);

	int rc = 0;

	if (ev->revents & (EV_SIGNAL)) {
		rc = ev->revents;
		ev->revents = 0;
	}

	return rc;
}

int
nce_signal_wait(EV_P_ struct nce_signal *ev)
{
	assert(ev);

	int rc = 0;

	while (!(rc = nce_signal_ready(ev)))
		nce_coro_yield();

	return rc;
}

void
nce_signal_set(struct nce_signal *ev, int signo)
{
	assert(ev);
	assert(ev->signal.active == 0);

	ev_signal_set(&ev->signal, signo);
}

int
nce_signal_coro_spawn(EV_P_ struct nce_signal_coro *evco, const struct nce_signal_coro_args *args)
{
	assert(evco);
	assert(evco->entry);

	(void)args;

	int rc;

	evco->coro.entry = nce_signal_coro_entry_cb;

	if (!evco->coro.finalizer)
		evco->coro.finalizer = nce_signal_coro_finalizer_cb;

	/*
	 * Watchers should be executed before attached coroutines to allow
	 * resuming them if an event happened.
	 */
	ev_set_priority(&evco->signal.signal, NCE_PRI_MAX - 1);
	if (args)
		nce_signal_set(&evco->signal, args->signo);
	else
		evco->coro.flags |= NCE_CORO_INACTIVE;

	/* Automatically start the watcher unless disabled. */
	if (!(evco->coro.flags & NCE_CORO_INACTIVE))
		nce_signal_start(EV_A_ &evco->signal);

	if ((rc = nce_coro_create(EV_A_ &evco->coro)) < 0)
		nce_signal_stop(EV_A_ &evco->signal);
	else
		nce_coro_resume(&evco->coro);

	return rc;
}

void
nce_signal_coro_destroy(struct nce_signal_coro *evco)
{
	assert(evco);

	/* Will call nce_signal_coro_finalizer_cb */
	nce_coro_destroy(&evco->coro);
}
