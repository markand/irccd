/*
 * casync.h -- coroutine watcher support for ev_async
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

#ifndef LIBCORO_CASYNC_H
#define LIBCORO_CASYNC_H

/**
 * \file coro/casync.h
 * \brief Coroutine watcher support for ev_async.
 * \ingroup libcoro-ev-watchers
 */

#include <stddef.h>

#include <ev.h>

#include "coro.h"

#if defined(DOXYGEN)
#define EV_P_ struct ev_loop *,
#endif

/**
 * \brief Convenient ::CORO_CONTAINER_OF macro for ::casync.
 */
#define CASYNC(Ptr, Field) \
	(CORO_CONTAINER_OF(Ptr, struct casync, Field))

struct casync;
struct casync_coro;
struct casync_coro_def;

/**
 * \brief Coroutine entrypoint for async.
 *
 * Similar to ::coro_entry_t but it receives its watcher as argument.
 */
typedef void (*casync_coro_entry_t)(EV_P_ struct casync *self);

/**
 * \brief Finalizer function.
 *
 * Similar to ::coro_finalizer_t but let the user perform extra step on a
 * coroutine watcher.
 */
typedef void (*casync_coro_finalizer_t)(EV_P_ struct casync *self);

/**
 * \struct casync
 * \brief Event watcher for ev_async.
 */
struct casync {
	/**
	 * Underlying ev_async.
	 */
	struct ev_async async;

	/**
	 * (read-only)
	 *
	 * Events received from the libev callback.
	 */
	int revents;
};

/**
 * \struct casync_coro
 * \brief Convenient coroutine coupled with a async.
 */
struct casync_coro {
	/**
	 * Underlying watcher to use.
	 */
	struct casync async;

	/**
	 * Underlying coroutine.
	 */
	struct coro coro;

	/**
	 * (private)
	 */
	casync_coro_entry_t entry;

	/**
	 * (private)
	 */
	casync_coro_finalizer_t finalizer;
};

/**
 * \struct casync_coro_def
 * \brief Watcher coroutine definition.
 *
 * This structure is used as a descriptor for ::casync_coro_spawn.
 */
struct casync_coro_def {
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
	casync_coro_entry_t entry;

	/**
	 * (optional)
	 *
	 * Coroutine finalizer.
	 *
	 * This user function is called after the coroutine watcher has been
	 * cleanup itself.
	 */
	casync_coro_finalizer_t finalizer;
};

/**
 * Initialize defaults.
 *
 * This function is not required if you directly use ::casync_use or
 * ::casync_create but is provided if you wish to call ::casync_finish
 * prematurly.
 */
void
casync_init(struct casync *ev);

/**
 * Start the event watcher.
 *
 * This function is the `ev_async_start` equivalent, it does nothing
 * regarding the internal coroutine.
 *
 * Caller must have a coroutine calling casync_wait indefinitely until the
 * watcher is stopped.
 *
 * No-op if the watcher is already active
 */
void
casync_start(EV_P_ struct casync *ev);

/**
 * Indicate if the watcher is active.
 *
 * \return non-zero if the watcher was started
 * \sa ::casync_start
 */
int
casync_active(const struct casync *ev);

/**
 * Feed an event to the watcher.
 *
 * It is equivalent to ev_feed_event.
 */
void
casync_feed(EV_P_ struct casync *ev, int events);

/**
 * Stop the event watcher.
 *
 * This function is the `ev_async_sttop` equivalent, it does nothing
 * regarding the internal coroutine.
 *
 * No-op if the watcher is already inactive.
 */
void
casync_stop(EV_P_ struct casync *ev);

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
casync_ready(struct casync *ev);

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
 * actually blocking on ::casync_wait. Resuming a coroutine that is waiting
 * on ::casync_wait while the watcher isn't ready nor started won't return
 * and will yield again until condition is met.
 *
 * It is perfectly safe to call this function even if the watcher is stopped.
 *
 * This function **yields**.
 *
 * \return the watcher revents
 */
int
casync_wait(EV_P_ struct casync *ev);

/**
 * Cleanup internal resources.
 *
 * \pre watcher must be stopped
 */
void
casync_finish(struct casync *ev);

/**
 * Initialize watcher and its coroutine.
 *
 * This is equivalent to calling ::casync_init followed by ::coro_init.
 */
void
casync_coro_init(struct casync_coro *evco);

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
casync_coro_spawn(EV_P_ struct casync_coro *evco, const struct casync_coro_def *def);

/**
 * Stop the internal watcher and destroy it along with its dedicated coroutine.
 *
 * Do not call this function within a ::casync_coro_def::finalizer callback.
 */
void
casync_coro_finish(struct casync_coro *evco);

#endif /* !LIBCORO_CASYNC_H */
