/*
 * cio.h -- coroutine watcher support for ev_io
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

#ifndef LIBCORO_CIO_H
#define LIBCORO_CIO_H

/**
 * \file coro/cio.h
 * \brief Coroutine watcher support for ev_io.
 * \ingroup libcoro-ev-watchers
 */

#include <stddef.h>

#include <ev.h>

#include "coro.h"

#if defined(DOXYGEN)
#define EV_P_ struct ev_loop *,
#endif

/**
 * \brief Convenient ::CORO_CONTAINER_OF macro for ::cio.
 */
#define CIO(Ptr, Field) \
	(CORO_CONTAINER_OF(Ptr, struct cio, Field))

struct cio;
struct cio_coro;
struct cio_coro_def;

/**
 * \brief Coroutine entrypoint for io.
 */
typedef void (*cio_coro_entry_t)(EV_P_ struct cio *self);

/**
 * \struct cio
 * \brief Event watcher for ev_io.
 */
struct cio {
	/**
	 * Underlying ev_io.
	 */
	struct ev_io io;

	/**
	 * (read-only)
	 *
	 * Events received from the libev callback.
	 */
	int revents;
};

/**
 * \struct cio_coro
 * \brief Convenient coroutine coupled with a io.
 */
struct cio_coro {
	/**
	 * Underlying watcher to use.
	 */
	struct cio io;

	/**
	 * Underlying coroutine.
	 */
	struct coro coro;

	/**
	 * Watcher coroutine entrypoint.
	 */
	cio_coro_entry_t entry;
};

/**
 * \struct cio_coro_def
 * \brief Watcher coroutine definition.
 *
 * This structure is used as a descriptor for ::cio_coro_spawn.
 */
struct cio_coro_def {
	/**
	 * \copydoc ::coro_def::name.
	 */
	const char *name;

	/**
	 * \copydoc ::coro_def::stack_size.
	 */
	size_t stack_size;

	/**
	 * \copydoc ::coro_def::flags.
	 */
	unsigned int flags;

	/**
	 * Watcher coroutine entrypoint.
	 */
	cio_coro_entry_t entry;

	/**
	 * (optional)
	 *
	 * Coroutine finalizer.
	 *
	 * If this function is NULL, a default will be provided so that calling
	 * calling ::coro_finish actually resolves to an internal handler
	 * stopping and destroying the watcher making possible to create
	 * coroutines entirely driven by the event loop
	 */
	coro_finalizer_t finalizer;

	/**
	 * File descriptor to monitor.
	 */
	int fd;

	/**
	 * Events to monitor (usually EV_READ or EV_WRITE)
	 */
	int events;

};

/**
 * Initialize defaults.
 *
 * This function is not required if you directly use ::cio_use or
 * ::cio_create but is provided if you wish to call ::cio_finish
 * prematurly.
 */
void
cio_init(struct cio *ev);

/**
 * Start the event watcher.
 *
 * This function is the `ev_io_start` equivalent, it does nothing
 * regarding the internal coroutine.
 *
 * Caller must have a coroutine calling cio_wait indefinitely until the
 * watcher is stopped.
 *
 * No-op if the watcher is already active
 */
void
cio_start(EV_P_ struct cio *ev);

/**
 * Indicate if the watcher is active.
 *
 * \return non-zero if the watcher was started
 * \sa ::cio_start
 */
int
cio_active(const struct cio *ev);

/**
 * Feed an event to the watcher.
 *
 * It is equivalent to ev_feed_event.
 */
void
cio_feed(EV_P_ struct cio *ev, int events);

/**
 * Stop the event watcher.
 *
 * This function is the `ev_io_sttop` equivalent, it does nothing
 * regarding the internal coroutine.
 *
 * No-op if the watcher is already inactive.
 */
void
cio_stop(EV_P_ struct cio *ev);

/**
 * Return internal watcher events.
 *
 * If events have been received, they are removed from the watcher and returned,
 * otherwise 0 is returned.
 *
 * It is useful if you plan to use multiple watchers in the same coroutine and
 * check them individually as long as you never forget to check them all.
 *
 * This function returns immediately.
 *
 * \return the current events or 0 if none
 */
int
cio_ready(struct cio *ev);

/**
 * Yield calling coroutine until watcher becomes ready.
 *
 * The returned value is identical to the `revents` argument that you would
 * receive in the normal libev usage.
 *
 * If the watcher is already ready the function returns immediately and the
 * internal state is cleared.
 *
 * Because libev's event loop only applies an event flag inside the watcher user
 * is responsible of resuming its own coroutines including the one that is
 * actually blocking on ::cio_wait. Resuming a coroutine that is waiting
 * on ::cio_wait while the watcher isn't ready nor started won't return
 * and will yield again until condition is met.
 *
 * It is perfectly safe to call this function even if the watcher is stopped.
 *
 * This function **yields**.
 *
 * \return the watcher revents
 */
int
cio_wait(EV_P_ struct cio *ev);

/**
 * Configure watcher.
 *
 * This function is equivalent to ev_io_set.
 */
void
cio_set(struct cio *ev, int fd, int events);

/**
 * This function stops the watcher, set its new values and start it again.
 *
 * There is no equivalent in libev.
 */
void
cio_reset(EV_P_ struct cio *ev, int fd, int events);

/**
 * Cleanup internal resources.
 *
 * \pre watcher must be stopped
 */
void
cio_finish(struct cio *ev);

/**
 * This all in one function initialize, set and optionnally start the watcher
 * and immediately creates its dedicated coroutine which is also started
 * automatically.
 *
 * \pre def != NULL
 * \param def the coroutine and watcher description
 * \return same as ::coro_create
 */
int
cio_coro_spawn(EV_P_ struct cio_coro *coro, const struct cio_coro_def *def);

#endif /* !LIBCORO_CIO_H */
