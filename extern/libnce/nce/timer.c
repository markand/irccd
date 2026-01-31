/*
 * nce/timer.c -- coroutine watcher support for ev_timer
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

#include "timer.h"

static void
nce_timer_cb(EV_P_ struct ev_timer *self, int revents)
{
	struct nce_timer *ev = NCE_TIMER(self, timer);

	if (revents & (EV_TIMER)) {
#if !defined(NDEBUG)
		/*
		 * Make sure user fetched events through the nce_timer_wait
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
nce_timer_start(EV_P_ struct nce_timer *ev)
{
	assert(ev);

	if (ev->timer.active)
		return;

	ev_init(&ev->timer, nce_timer_cb);
	ev_timer_start(EV_A_ &ev->timer);
}

int
nce_timer_active(const struct nce_timer *ev)
{
	assert(ev);

	return ev->timer.active;
}

void
nce_timer_feed(EV_P_ struct nce_timer *ev, int events)
{
	assert(ev);

	ev_feed_event(EV_A_ &ev->timer, events);
}

void
nce_timer_stop(EV_P_ struct nce_timer *ev)
{
	assert(ev);

	ev->revents = 0;
	ev_timer_stop(EV_A_ &ev->timer);
}

int
nce_timer_ready(struct nce_timer *ev)
{
	assert(ev);

	int rc = 0;

	if (ev->revents & (EV_TIMER)) {
		rc = ev->revents;
		ev->revents = 0;
	}

	return rc;
}

int
nce_timer_wait(struct nce_timer *ev)
{
	assert(ev);

	int rc = 0;

	while (!(rc = nce_timer_ready(ev)))
		nce_coro_yield();

	return rc;
}

void
nce_timer_set(struct nce_timer *ev, ev_tstamp after, ev_tstamp repeat)
{
	assert(ev);
	assert(ev->timer.active == 0);

	ev_timer_set(&ev->timer, after, repeat);
}

void
nce_timer_restart(EV_P_ struct nce_timer *ev, ev_tstamp after, ev_tstamp repeat)
{
	assert(ev);

	nce_timer_stop(EV_A_ ev);
	nce_timer_set(ev, after, repeat);
	nce_timer_start(EV_A_ ev);
}

void
nce_timer_again(EV_P_ struct nce_timer *ev)
{
	assert(ev);

	ev_timer_again(EV_A_ &ev->timer);
}

int
nce_timer_coro_spawn(EV_P_ struct nce_timer_coro *evco,
                           ev_tstamp after,
                           ev_tstamp repeat)
{
	assert(evco);

	int rc;

	ev_init(&evco->timer.timer, nce_timer_cb);
	ev_set_priority(&evco->timer.timer, -1);

	if (!(evco->coro.flags & NCE_INACTIVE)) {
		nce_timer_set(&evco->timer, after, repeat);
		nce_timer_start(EV_A_ &evco->timer);
	}

	if ((rc = nce_coro_create(EV_A_ &evco->coro)) < 0)
		nce_timer_stop(EV_A_ &evco->timer);
	else
		nce_coro_resume(&evco->coro);

	return rc;
}
void
nce_timer_coro_destroy(EV_P_ struct nce_timer_coro *evco)
{
	assert(evco);

	nce_timer_stop(EV_A_ &evco->timer);
	nce_coro_destroy(&evco->coro);
}

void
nce_timer_coro_terminate(EV_P_ struct nce_coro *self)
{
	struct nce_timer_coro *evco = NCE_TIMER_CORO(self, coro);

	nce_timer_stop(EV_A_ &evco->timer);
}
