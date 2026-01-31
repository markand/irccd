/*
 * nce/io.c -- coroutine watcher support for ev_io
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

#include "io.h"

static void
nce_io_cb(EV_P_ struct ev_io *self, int revents)
{
	struct nce_io *ev = NCE_IO(self, io);

	if (revents & (EV_READ | EV_WRITE)) {
#if !defined(NDEBUG)
		/*
		 * Make sure user fetched events through the nce_io_wait
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
nce_io_start(EV_P_ struct nce_io *ev)
{
	assert(ev);

	if (ev->io.active)
		return;

	ev_init(&ev->io, nce_io_cb);
	ev_io_start(EV_A_ &ev->io);
}

int
nce_io_active(const struct nce_io *ev)
{
	assert(ev);

	return ev->io.active;
}

void
nce_io_feed(EV_P_ struct nce_io *ev, int events)
{
	assert(ev);

	ev_feed_event(EV_A_ &ev->io, events);
}

void
nce_io_stop(EV_P_ struct nce_io *ev)
{
	assert(ev);

	ev->revents = 0;
	ev_io_stop(EV_A_ &ev->io);
}

int
nce_io_ready(struct nce_io *ev)
{
	assert(ev);

	int rc = 0;

	if (ev->revents & (EV_READ | EV_WRITE)) {
		rc = ev->revents;
		ev->revents = 0;
	}

	return rc;
}

int
nce_io_wait(struct nce_io *ev)
{
	assert(ev);

	int rc = 0;

	while (!(rc = nce_io_ready(ev)))
		nce_coro_yield();

	return rc;
}
int
nce_io_fd(const struct nce_io *ev)
{
	assert(ev);

	return ev->io.fd;
}

void
nce_io_set(struct nce_io *ev, int fd, int events)
{
	assert(ev);
	assert(ev->io.active == 0);

	ev_io_set(&ev->io, fd, events);
}

void
nce_io_reset(EV_P_ struct nce_io *ev, int fd, int events)
{
	assert(ev);

	nce_io_stop(EV_A_ ev);
	nce_io_set(ev, fd, events);
	nce_io_start(EV_A_ ev);
}

int
nce_io_coro_spawn(EV_P_ struct nce_io_coro *evco, int fd, int events)
{
	assert(evco);

	int rc;

	ev_init(&evco->io.io, nce_io_cb);
	ev_set_priority(&evco->io.io, -1);

	if (!(evco->coro.flags & NCE_INACTIVE)) {
		nce_io_set(&evco->io, fd, events);
		nce_io_start(EV_A_ &evco->io);
	}

	if ((rc = nce_coro_create(EV_A_ &evco->coro)) < 0)
		nce_io_stop(EV_A_ &evco->io);
	else
		nce_coro_resume(&evco->coro);

	return rc;
}
void
nce_io_coro_destroy(EV_P_ struct nce_io_coro *evco)
{
	assert(evco);

	nce_io_stop(EV_A_ &evco->io);
	nce_coro_destroy(&evco->coro);
}

void
nce_io_coro_terminate(EV_P_ struct nce_coro *self)
{
	struct nce_io_coro *evco = NCE_IO_CORO(self, coro);

	nce_io_stop(EV_A_ &evco->io);
}
