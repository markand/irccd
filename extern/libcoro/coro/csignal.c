/*
 * csignal.c -- coroutine watcher support for ev_signal
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

#include "csignal.h"

static void
csignal_cb(EV_P_ struct ev_signal *self, int revents)
{
	struct csignal *ev = CSIGNAL(self, signal);

	if (revents & (EV_SIGNAL)) {
#if !defined(NDEBUG)
		/*
		 * Make sure user fetched events through the csignal_wait
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
csignal_coro_entry_cb(EV_P_ struct coro *self)
{
	struct csignal_coro *evco = CORO_CONTAINER_OF(self, struct csignal_coro, coro);

	evco->entry(EV_A_ &evco->signal);
}

static void
csignal_coro_finalizer_cb(EV_P_ struct coro *self)
{
	struct csignal_coro *evco = CORO_CONTAINER_OF(self, struct csignal_coro, coro);

	/* Stop the watcher for convenience. */
	csignal_stop(EV_A_ &evco->signal);
	csignal_finish(&evco->signal);

	/* Call user as very last function. */
	if (evco->finalizer)
		evco->finalizer(EV_A_ &evco->signal);
}

void
csignal_init(struct csignal *ev)
{
	assert(ev);

	ev->revents = 0;
	ev->signal = (const struct ev_signal) {};
	ev_init(&ev->signal, csignal_cb);
}

void
csignal_start(EV_P_ struct csignal *ev)
{
	assert(ev);

	ev_signal_start(EV_A_ &ev->signal);
}

int
csignal_active(const struct csignal *ev)
{
	assert(ev);

	return ev->signal.active;
}

void
csignal_feed(EV_P_ struct csignal *ev, int events)
{
	assert(ev);

	ev_feed_event(EV_A_ &ev->signal, events);
}

void
csignal_stop(EV_P_ struct csignal *ev)
{
	assert(ev);

	ev_signal_stop(EV_A_ &ev->signal);
}

int
csignal_ready(struct csignal *ev)
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
csignal_wait(EV_P_ struct csignal *ev)
{
	assert(ev);

	int rc = 0;

	while (!(rc = csignal_ready(ev)))
		coro_yield();

	return rc;
}

void
csignal_set(struct csignal *ev, int signo)
{
	assert(ev);
	assert(ev->signal.active == 0);

	ev_signal_set(&ev->signal, signo);
}

void
csignal_finish(struct csignal *ev)
{
	assert(ev);
	assert(ev->signal.active == 0);

	ev->revents = 0;
}

void
csignal_coro_init(struct csignal_coro *evco)
{
	assert(evco);

	csignal_init(&evco->signal);

	coro_init(&evco->coro);
	coro_set_entry(&evco->coro, csignal_coro_entry_cb);
	coro_set_finalizer(&evco->coro, csignal_coro_finalizer_cb);

	evco->entry = NULL;
	evco->finalizer = NULL;
}

int
csignal_coro_spawn(EV_P_ struct csignal_coro *evco, const struct csignal_coro_def *def)
{
	assert(evco);
	assert(def);
	assert(def->entry);

	int rc;

	csignal_coro_init(evco);

	evco->entry = def->entry;
	evco->finalizer = def->finalizer;

	/*
	 * Watchers should be executed before attached coroutines to allow
	 * resuming them if an event happened.
	 */
	ev_set_priority(&evco->signal.signal, CORO_PRI_MAX - 1);
	csignal_set(&evco->signal, def->signo);

	/* Automatically start the watcher unless disabled. */
	if (!(def->flags & CORO_INACTIVE))
		csignal_start(EV_A_ &evco->signal);

	/* All other fields are available for customization. */
	coro_set_name(&evco->coro, def->name);
	coro_set_stack_size(&evco->coro, def->stack_size);
	coro_set_flags(&evco->coro, def->flags);

	if ((rc = coro_create(EV_A_ &evco->coro)) < 0)
		csignal_stop(EV_A_ &evco->signal);
	else
		coro_resume(&evco->coro);

	return rc;
}

void
csignal_coro_finish(struct csignal_coro *evco)
{
	assert(evco);

	/* Will call csignal_coro_finalizer_cb */
	coro_finish(&evco->coro);
}

