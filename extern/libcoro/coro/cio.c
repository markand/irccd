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

static inline void
cio_coro_finalizer(struct cio_coro *coro)
{
#if EV_MULTIPLICITY
	if (coro->coro.loop)
		cio_stop(coro->coro.loop, &coro->io);
#else
	cio_stop(&coro->io);
#endif

	cio_finish(&coro->io);
}

static void
cio_coro_entry_cb(EV_P_ struct coro *self)
{
	struct cio_coro *coro = CORO_CONTAINER_OF(self, struct cio_coro, coro);

	coro->entry(EV_A_ &coro->io);
}

static void
cio_coro_finalizer_cb(EV_P_ struct coro *self)
{
	struct cio_coro *coro = CORO_CONTAINER_OF(self, struct cio_coro, coro);

	cio_coro_finalizer(coro);
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

int
cio_coro_spawn(EV_P_ struct cio_coro *coro, const struct cio_coro_def *def)
{
	assert(coro);
	assert(def);

	int rc;

	coro->entry = def->entry;
	cio_init(&coro->io);

	/*
	 * Watchers should be executed before attached coroutines to allow
	 * resuming them if an event happened.
	 */
	ev_set_priority(&coro->io.io, CORO_PRI_MAX - 1);
	cio_set(&coro->io, def->fd, def->events);

	/* Automatically start the watcher unless disabled. */
	if (!(def->flags & CORO_INACTIVE))
		cio_start(EV_A_ &coro->io);

	coro_init(&coro->coro);

	/* All other fields are available for customization. */
	coro_set_name(&coro->coro, def->name);
	coro_set_stack_size(&coro->coro, def->stack_size);
	coro_set_flags(&coro->coro, def->flags);
	coro_set_entry(&coro->coro, cio_coro_entry_cb);

	/*
	 * Use a default finalizer if NULL to conveniently stop and destroy the
	 * underlying watcher.
	 */
	if (!def->finalizer)
		coro_set_finalizer(&coro->coro, cio_coro_finalizer_cb);
	else
		coro_set_finalizer(&coro->coro, def->finalizer);

	if ((rc = coro_create(EV_A_ &coro->coro)) < 0)
		cio_stop(EV_A_ &coro->io);
	else
		coro_resume(&coro->coro);

	return rc;
}
