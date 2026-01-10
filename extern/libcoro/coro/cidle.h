/*
 * cidle.h -- coroutine watcher support for ev_idle
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

#ifndef LIBCORO_CIDLE_H
#define LIBCORO_CIDLE_H

/**
 * \file coro/cidle.h
 * \brief Coroutine watcher support for ev_idle.
 * \ingroup libcoro-ev-watchers
 */

#include <stddef.h>

#include <ev.h>

#include "coro.h"

#if defined(DOXYGEN)
#define EV_P_ struct ev_loop *,
#endif

/**
 * \brief Convenient ::CORO_CONTAINER_OF macro for ::cidle.
 */
#define CIDLE(Ptr, Field) \
	(CORO_CONTAINER_OF(Ptr, struct cidle, Field))

struct cidle;
struct cidle_coro;
struct cidle_coro_ops;

/**
 * \brief Coroutine entrypoint for idle.
 *
 * Similar to ::coro_entry_t but it receives its watcher as argument.
 */
typedef void (*cidle_coro_entry_t)(EV_P_ struct cidle *self);

/**
 * \brief Finalizer function.
 *
 * Similar to ::coro_finalizer_t but let the user perform extra step on a
 * coroutine watcher.
 */
typedef void (*cidle_coro_finalizer_t)(EV_P_ struct cidle *self);

/**
 * \struct cidle
 * \brief Event watcher for ev_idle.
 */
struct cidle {
	/**
	 * Underlying ev_idle.
	 */
	struct ev_idle idle;

	/**
	 * (read-only)
	 *
	 * Events received from the libev callback.
	 */
	int revents;
};

/**
 * \struct cidle_coro
 * \brief Convenient coroutine coupled with a idle.
 */
struct cidle_coro {
	/**
	 * Underlying watcher to use.
	 */
	struct cidle idle;

	/**
	 * Coroutine attached to this watcher.
	 *
	 * Caller can set fields just like a normal coroutine however:
	 *
	 * The field ::coro::entry is replaced with an internal callback calling
	 * ::cidle_coro::entry instead.
	 *
	 * If ::coro::finalizer is NULL, an internal callback is set to stop and
	 * destroy the associated watcher. This internal callback will call
	 * ::cidle_coro::finalizer and user is encouraged to use it
	 * instead.
	 */
	struct coro coro;

	/**
	 * (init)
	 *
	 * Coroutine watcher entrypoint.
	 */
	cidle_coro_entry_t entry;

	/**
	 * (optional)
	 *
	 * Finalizer for the coroutine watcher.
	 */
	cidle_coro_finalizer_t finalizer;
};


/**
 * Initialize private fields.
 */
void
cidle_init(struct cidle *ev);

/**
 * Start the event watcher.
 *
 * This function is the `ev_idle_start` equivalent, it does nothing
 * regarding the internal coroutine.
 *
 * Caller must have a coroutine calling cidle_wait indefinitely until the
 * watcher is stopped.
 *
 * No-op if the watcher is already active
 */
void
cidle_start(EV_P_ struct cidle *ev);

/**
 * Indicate if the watcher is active.
 *
 * \return non-zero if the watcher was started
 * \sa ::cidle_start
 */
int
cidle_active(const struct cidle *ev);

/**
 * Feed an event to the watcher.
 *
 * It is equivalent to ev_feed_event.
 */
void
cidle_feed(EV_P_ struct cidle *ev, int events);

/**
 * Stop the event watcher.
 *
 * This function is the `ev_idle_sttop` equivalent, it does nothing
 * regarding the internal coroutine.
 *
 * No-op if the watcher is already inactive.
 */
void
cidle_stop(EV_P_ struct cidle *ev);

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
cidle_ready(struct cidle *ev);

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
 * actually blocking on ::cidle_wait. Resuming a coroutine that is waiting
 * on ::cidle_wait while the watcher isn't ready nor started won't return
 * and will yield again until condition is met.
 *
 * It is perfectly safe to call this function even if the watcher is stopped.
 *
 * This function **yields**.
 *
 * \return the watcher revents
 */
int
cidle_wait(EV_P_ struct cidle *ev);

/**
 * Cleanup internal resources.
 *
 * \pre watcher must be stopped
 */
void
cidle_finish(struct cidle *ev);

/**
 * Initialize watcher and its coroutine.
 *
 * This is equivalent to calling ::cidle_init followed by ::coro_init.
 */
void
cidle_coro_init(struct cidle_coro *evco);

/**
 * This all in one function initialize, set and optionnally start the watcher
 * and immediately creates its dedicated coroutine which is also started
 * automatically.
 *
 * \param ops additional watcher spawn options (maybe NULL)
 * \return refer to ::coro_spawn
 */
int
cidle_coro_spawn(EV_P_ struct cidle_coro *evco, const struct cidle_coro_ops *ops);

/**
 * Stop the internal watcher and destroy it along with its dedicated coroutine.
 *
 * Do not call this function within a ::cidle_coro::finalizer callback.
 */
void
cidle_coro_finish(struct cidle_coro *evco);

#endif /* !LIBCORO_CIDLE_H */
