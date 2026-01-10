/*
 * csignal.h -- coroutine watcher support for ev_signal
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

#ifndef LIBCORO_CSIGNAL_H
#define LIBCORO_CSIGNAL_H

/**
 * \file coro/csignal.h
 * \brief Coroutine watcher support for ev_signal.
 * \ingroup libcoro-ev-watchers
 */

#include <stddef.h>

#include <ev.h>

#include "coro.h"

#if defined(DOXYGEN)
#define EV_P_ struct ev_loop *,
#endif

/**
 * \brief Convenient ::CORO_CONTAINER_OF macro for ::csignal.
 */
#define CSIGNAL(Ptr, Field) \
	(CORO_CONTAINER_OF(Ptr, struct csignal, Field))

struct csignal;
struct csignal_coro;
struct csignal_coro_ops;

/**
 * \brief Coroutine entrypoint for signal.
 *
 * Similar to ::coro_entry_t but it receives its watcher as argument.
 */
typedef void (*csignal_coro_entry_t)(EV_P_ struct csignal *self);

/**
 * \brief Finalizer function.
 *
 * Similar to ::coro_finalizer_t but let the user perform extra step on a
 * coroutine watcher.
 */
typedef void (*csignal_coro_finalizer_t)(EV_P_ struct csignal *self);

/**
 * \struct csignal
 * \brief Event watcher for ev_signal.
 */
struct csignal {
	/**
	 * Underlying ev_signal.
	 */
	struct ev_signal signal;

	/**
	 * (read-only)
	 *
	 * Events received from the libev callback.
	 */
	int revents;
};

/**
 * \struct csignal_coro
 * \brief Convenient coroutine coupled with a signal.
 */
struct csignal_coro {
	/**
	 * Underlying watcher to use.
	 */
	struct csignal signal;

	/**
	 * Coroutine attached to this watcher.
	 *
	 * Caller can set fields just like a normal coroutine however:
	 *
	 * The field ::coro::entry is replaced with an internal callback calling
	 * ::csignal_coro::entry instead.
	 *
	 * If ::coro::finalizer is NULL, an internal callback is set to stop and
	 * destroy the associated watcher. This internal callback will call
	 * ::csignal_coro::finalizer and user is encouraged to use it
	 * instead.
	 */
	struct coro coro;

	/**
	 * (init)
	 *
	 * Coroutine watcher entrypoint.
	 */
	csignal_coro_entry_t entry;

	/**
	 * (optional)
	 *
	 * Finalizer for the coroutine watcher.
	 */
	csignal_coro_finalizer_t finalizer;
};

/**
 * \struct csignal_coro_ops
 * \brief Options for ::csignal_coro_spawn.
 */
struct csignal_coro_ops {
	/**
	 * Signal to wait on.
	 */
	int signo;
};

/**
 * Initialize private fields.
 */
void
csignal_init(struct csignal *ev);

/**
 * Start the event watcher.
 *
 * This function is the `ev_signal_start` equivalent, it does nothing
 * regarding the internal coroutine.
 *
 * Caller must have a coroutine calling csignal_wait indefinitely until the
 * watcher is stopped.
 *
 * No-op if the watcher is already active
 */
void
csignal_start(EV_P_ struct csignal *ev);

/**
 * Indicate if the watcher is active.
 *
 * \return non-zero if the watcher was started
 * \sa ::csignal_start
 */
int
csignal_active(const struct csignal *ev);

/**
 * Feed an event to the watcher.
 *
 * It is equivalent to ev_feed_event.
 */
void
csignal_feed(EV_P_ struct csignal *ev, int events);

/**
 * Stop the event watcher.
 *
 * This function is the `ev_signal_sttop` equivalent, it does nothing
 * regarding the internal coroutine.
 *
 * No-op if the watcher is already inactive.
 */
void
csignal_stop(EV_P_ struct csignal *ev);

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
csignal_ready(struct csignal *ev);

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
 * actually blocking on ::csignal_wait. Resuming a coroutine that is waiting
 * on ::csignal_wait while the watcher isn't ready nor started won't return
 * and will yield again until condition is met.
 *
 * It is perfectly safe to call this function even if the watcher is stopped.
 *
 * This function **yields**.
 *
 * \return the watcher revents
 */
int
csignal_wait(EV_P_ struct csignal *ev);

/**
 * Configure watcher.
 *
 * This function is equivalent to ev_signal_set.
 */
void
csignal_set(struct csignal *ev, int signo);

/**
 * Cleanup internal resources.
 *
 * \pre watcher must be stopped
 */
void
csignal_finish(struct csignal *ev);

/**
 * Initialize watcher and its coroutine.
 *
 * This is equivalent to calling ::csignal_init followed by ::coro_init.
 */
void
csignal_coro_init(struct csignal_coro *evco);

/**
 * This all in one function initialize, set and optionnally start the watcher
 * and immediately creates its dedicated coroutine which is also started
 * automatically.
 *
 * \param ops additional watcher spawn options (maybe NULL)
 * \return refer to ::coro_spawn
 */
int
csignal_coro_spawn(EV_P_ struct csignal_coro *evco, const struct csignal_coro_ops *ops);

/**
 * Stop the internal watcher and destroy it along with its dedicated coroutine.
 *
 * Do not call this function within a ::csignal_coro::finalizer callback.
 */
void
csignal_coro_finish(struct csignal_coro *evco);

#endif /* !LIBCORO_CSIGNAL_H */
