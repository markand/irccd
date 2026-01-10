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
struct cio_coro_ops;

/**
 * \brief Coroutine entrypoint for io.
 *
 * Similar to ::coro_entry_t but it receives its watcher as argument.
 */
typedef void (*cio_coro_entry_t)(EV_P_ struct cio *self);

/**
 * \brief Finalizer function.
 *
 * Similar to ::coro_finalizer_t but let the user perform extra step on a
 * coroutine watcher.
 */
typedef void (*cio_coro_finalizer_t)(EV_P_ struct cio *self);

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
	 * Coroutine attached to this watcher.
	 *
	 * Caller can set fields just like a normal coroutine however:
	 *
	 * The field ::coro::entry is replaced with an internal callback calling
	 * ::cio_coro::entry instead.
	 *
	 * If ::coro::finalizer is NULL, an internal callback is set to stop and
	 * destroy the associated watcher. This internal callback will call
	 * ::cio_coro::finalizer and user is encouraged to use it
	 * instead.
	 */
	struct coro coro;

	/**
	 * (init)
	 *
	 * Coroutine watcher entrypoint.
	 */
	cio_coro_entry_t entry;

	/**
	 * (optional)
	 *
	 * Finalizer for the coroutine watcher.
	 */
	cio_coro_finalizer_t finalizer;

	/**
	 * If the watcher is still active, the file descriptor is closed using
	 * POSIX `close()` after stopping the watcher.
	 */
	int close;
};

/**
 * \struct cio_coro_ops
 * \brief Options for ::cio_coro_spawn.
 */
struct cio_coro_ops {
	/**
	 * File descriptor to monitor.
	 */
	int fd;

	/**
	 * Events to monitor for the file descriptor.
	 *
	 * If 0, coroutine starts with the ::cio unset. User will have to call
	 * ::cio_set and ::cio_start manually.
	 */
	int events;
};

/**
 * Initialize private fields.
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
 * Initialize watcher and its coroutine.
 *
 * This is equivalent to calling ::cio_init followed by ::coro_init.
 */
void
cio_coro_init(struct cio_coro *evco);

/**
 * This all in one function initialize, set and optionnally start the watcher
 * and immediately creates its dedicated coroutine which is also started
 * automatically.
 *
 * \param ops additional watcher spawn options (maybe NULL)
 * \return refer to ::coro_spawn
 */
int
cio_coro_spawn(EV_P_ struct cio_coro *evco, const struct cio_coro_ops *ops);

/**
 * Stop the internal watcher and destroy it along with its dedicated coroutine.
 *
 * Do not call this function within a ::cio_coro::finalizer callback.
 */
void
cio_coro_finish(struct cio_coro *evco);

#endif /* !LIBCORO_CIO_H */
