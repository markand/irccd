/*
 * cio.c -- coroutine watcher support for ev_io
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

#include "cio.h"

static void
cio_cb(EV_P_ struct ev_io *self, int revents)
{
	struct cio *ev = CIO(self, io);

	if (revents & (EV_READ | EV_WRITE)) {
#if !defined(NDEBUG)
		/*
		 * Make sure user fetched events through the cio_wait
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
cio_coro_entry_cb(EV_P_ struct coro *self)
{
	struct cio_coro *evco = CORO_CONTAINER_OF(self, struct cio_coro, coro);

	evco->entry(EV_A_ &evco->io);
}

static void
cio_coro_finalizer_cb(EV_P_ struct coro *self)
{
	struct cio_coro *evco = CORO_CONTAINER_OF(self, struct cio_coro, coro);

	/* Stop the watcher for convenience. */
	cio_stop(EV_A_ &evco->io);

	if (evco->close)
		close(evco->io.io.fd);

	cio_finish(&evco->io);

	/* Call user as very last function. */
	if (evco->finalizer)
		evco->finalizer(EV_A_ &evco->io);
}

void
cio_init(struct cio *ev)
{
	assert(ev);

	ev->revents = 0;
	ev->io = (const struct ev_io) {};
	ev_init(&ev->io, cio_cb);
}

void
cio_start(EV_P_ struct cio *ev)
{
	assert(ev);

	ev_io_start(EV_A_ &ev->io);
}

int
cio_active(const struct cio *ev)
{
	assert(ev);

	return ev->io.active;
}

void
cio_feed(EV_P_ struct cio *ev, int events)
{
	assert(ev);

	ev_feed_event(EV_A_ &ev->io, events);
}

void
cio_stop(EV_P_ struct cio *ev)
{
	assert(ev);

	ev_io_stop(EV_A_ &ev->io);
}

int
cio_ready(struct cio *ev)
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
cio_wait(EV_P_ struct cio *ev)
{
	assert(ev);

	int rc = 0;

	while (!(rc = cio_ready(ev)))
		coro_yield();

	return rc;
}

void
cio_set(struct cio *ev, int fd, int events)
{
	assert(ev);
	assert(ev->io.active == 0);

	ev_io_set(&ev->io, fd, events);
}

void
cio_reset(EV_P_ struct cio *ev, int fd, int events)
{
	assert(ev);

	ev_io_stop(EV_A_ &ev->io);
	ev_io_set(&ev->io, fd, events);
	ev_io_start(EV_A_ &ev->io);
}

void
cio_finish(struct cio *ev)
{
	assert(ev);
	assert(ev->io.active == 0);

	ev->revents = 0;
}

void
cio_coro_init(struct cio_coro *evco)
{
	assert(evco);

	cio_init(&evco->io);

	coro_init(&evco->coro);
	coro_set_entry(&evco->coro, cio_coro_entry_cb);
	coro_set_finalizer(&evco->coro, cio_coro_finalizer_cb);

	evco->entry = NULL;
	evco->finalizer = NULL;
	evco->close = 0;
}

int
cio_coro_spawn(EV_P_ struct cio_coro *evco, const struct cio_coro_def *def)
{
	assert(evco);
	assert(def);
	assert(def->entry);

	int rc;

	cio_coro_init(evco);

	evco->entry = def->entry;
	evco->finalizer = def->finalizer;

	/*
	 * Watchers should be executed before attached coroutines to allow
	 * resuming them if an event happened.
	 */
	ev_set_priority(&evco->io.io, CORO_PRI_MAX - 1);
	cio_set(&evco->io, def->fd, def->events);
	evco->close = def->close;

	/* Automatically start the watcher unless disabled. */
	if (!(def->flags & CORO_INACTIVE))
		cio_start(EV_A_ &evco->io);

	/* All other fields are available for customization. */
	coro_set_name(&evco->coro, def->name);
	coro_set_stack_size(&evco->coro, def->stack_size);
	coro_set_flags(&evco->coro, def->flags);

	if ((rc = coro_create(EV_A_ &evco->coro)) < 0)
		cio_stop(EV_A_ &evco->io);
	else
		coro_resume(&evco->coro);

	return rc;
}

void
cio_coro_finish(struct cio_coro *evco)
{
	assert(evco);

	/* Will call cio_coro_finalizer_cb */
	coro_finish(&evco->coro);
}

