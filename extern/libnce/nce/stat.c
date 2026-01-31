/*
 * nce/stat.c -- coroutine watcher support for ev_stat
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

#include "stat.h"

static void
nce_stat_cb(EV_P_ struct ev_stat *self, int revents)
{
	struct nce_stat *ev = NCE_STAT(self, stat);

	if (revents & (EV_STAT)) {
#if !defined(NDEBUG)
		/*
		 * Make sure user fetched events through the nce_stat_wait
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
nce_stat_start(EV_P_ struct nce_stat *ev)
{
	assert(ev);

	if (ev->stat.active)
		return;

	ev_init(&ev->stat, nce_stat_cb);
	ev_stat_start(EV_A_ &ev->stat);
}

int
nce_stat_active(const struct nce_stat *ev)
{
	assert(ev);

	return ev->stat.active;
}

void
nce_stat_feed(EV_P_ struct nce_stat *ev, int events)
{
	assert(ev);

	ev_feed_event(EV_A_ &ev->stat, events);
}

void
nce_stat_stop(EV_P_ struct nce_stat *ev)
{
	assert(ev);

	ev->revents = 0;
	ev_stat_stop(EV_A_ &ev->stat);
}

int
nce_stat_ready(struct nce_stat *ev)
{
	assert(ev);

	int rc = 0;

	if (ev->revents & (EV_STAT)) {
		rc = ev->revents;
		ev->revents = 0;
	}

	return rc;
}

int
nce_stat_wait(struct nce_stat *ev)
{
	assert(ev);

	int rc = 0;

	while (!(rc = nce_stat_ready(ev)))
		nce_coro_yield();

	return rc;
}

void
nce_stat_set(struct nce_stat *ev, const char *path, ev_tstamp interval)
{
	assert(ev);
	assert(path);

	ev_stat_set(&ev->stat, path, interval);
}

void
nce_stat_stat(EV_P_ struct nce_stat *ev)
{
	assert(ev);

	ev_stat_stat(EV_A_ &ev->stat);
}

int
nce_stat_coro_spawn(EV_P_ struct nce_stat_coro *evco,
                          const char *path,
                          ev_tstamp interval)
{
	assert(evco);
	assert(path);

	int rc;

	ev_init(&evco->stat.stat, nce_stat_cb);
	ev_set_priority(&evco->stat.stat, -1);

	if (!(evco->coro.flags & NCE_INACTIVE)) {
		nce_stat_set(&evco->stat, path, interval);
		nce_stat_start(EV_A_ &evco->stat);
	}

	if ((rc = nce_coro_create(EV_A_ &evco->coro)) < 0)
		nce_stat_stop(EV_A_ &evco->stat);
	else
		nce_coro_resume(&evco->coro);

	return rc;
}

void
nce_stat_coro_destroy(EV_P_ struct nce_stat_coro *evco)
{
	assert(evco);

	nce_stat_stop(EV_A_ &evco->stat);
	nce_coro_destroy(&evco->coro);
}

void
nce_stat_coro_terminate(EV_P_ struct nce_coro *self)
{
	struct nce_stat_coro *evco = NCE_STAT_CORO(self, coro);

	nce_stat_stop(EV_A_ &evco->stat);
}
