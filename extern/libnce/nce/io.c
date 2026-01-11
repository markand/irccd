/*
 * nce_io.c -- coroutine watcher support for ev_io
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
#include <unistd.h>

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

static void
nce_io_coro_entry_cb(EV_P_ struct nce_coro *self)
{
	struct nce_io_coro *evco = NCE_CONTAINER_OF(self, struct nce_io_coro, coro);

	evco->entry(EV_A_ &evco->io);
}

static void
nce_io_coro_finalizer_cb(EV_P_ struct nce_coro *self)
{
	struct nce_io_coro *evco = NCE_CONTAINER_OF(self, struct nce_io_coro, coro);

	/* Stop the watcher for convenience. */
	nce_io_stop(EV_A_ &evco->io);

	if (evco->close)
		close(evco->io.io.fd);


	/* Call user as very last function. */
	if (evco->finalizer)
		evco->finalizer(EV_A_ &evco->io);
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
nce_io_wait(EV_P_ struct nce_io *ev)
{
	assert(ev);

	int rc = 0;

	while (!(rc = nce_io_ready(ev)))
		nce_coro_yield();

	return rc;
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

	ev_io_stop(EV_A_ &ev->io);
	ev_io_set(&ev->io, fd, events);
	ev_io_start(EV_A_ &ev->io);
}

int
nce_io_coro_spawn(EV_P_ struct nce_io_coro *evco, const struct nce_io_coro_args *args)
{
	assert(evco);
	assert(evco->entry);

	(void)args;

	int rc;

	evco->coro.entry = nce_io_coro_entry_cb;

	if (!evco->coro.finalizer)
		evco->coro.finalizer = nce_io_coro_finalizer_cb;

	/*
	 * Watchers should be executed before attached coroutines to allow
	 * resuming them if an event happened.
	 */
	ev_set_priority(&evco->io.io, NCE_PRI_MAX - 1);

	/*
	 * Avoid starting the watcher if events is zero because if the whole
	 * cio_coro_args is zero it could start on STDIN_FILENO which may be
	 * undesired.
	 */
	if (!args || args->events == 0)
		evco->coro.flags |= NCE_CORO_INACTIVE;
	else
		nce_io_set(&evco->io, args->fd, args->events);

	/* Automatically start the watcher unless disabled. */
	if (!(evco->coro.flags & NCE_CORO_INACTIVE))
		nce_io_start(EV_A_ &evco->io);
	else
		ev_init(&evco->io.io, nce_io_cb);

	if ((rc = nce_coro_create(EV_A_ &evco->coro)) < 0)
		nce_io_stop(EV_A_ &evco->io);
	else
		nce_coro_resume(&evco->coro);

	return rc;
}

void
nce_io_coro_destroy(struct nce_io_coro *evco)
{
	assert(evco);

	/* Will call nce_io_coro_finalizer_cb */
	nce_coro_destroy(&evco->coro);
}
