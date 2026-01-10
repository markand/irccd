/*
 * cprepare.h -- coroutine watcher support for ev_prepare
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

#ifndef LIBCORO_CPREPARE_H
#define LIBCORO_CPREPARE_H

/**
 * \file coro/cprepare.h
 * \brief Coroutine watcher support for ev_prepare.
 * \ingroup libcoro-ev-watchers
 */

#include <stddef.h>

#include <ev.h>

#include "coro.h"

#if defined(DOXYGEN)
#define EV_P_ struct ev_loop *,
#endif

/**
 * \brief Convenient ::CORO_CONTAINER_OF macro for ::cprepare.
 */
#define CPREPARE(Ptr, Field) \
	(CORO_CONTAINER_OF(Ptr, struct cprepare, Field))

struct cprepare;
struct cprepare_coro;
struct cprepare_coro_def;

/**
 * \brief Coroutine entrypoint for prepare.
 *
 * Similar to ::coro_entry_t but it receives its watcher as argument.
 */
typedef void (*cprepare_coro_entry_t)(EV_P_ struct cprepare *self);

/**
 * \brief Finalizer function.
 *
 * Similar to ::coro_finalizer_t but let the user perform extra step on a
 * coroutine watcher.
 */
typedef void (*cprepare_coro_finalizer_t)(EV_P_ struct cprepare *self);

/**
 * \struct cprepare
 * \brief Event watcher for ev_prepare.
 */
struct cprepare {
	/**
	 * Underlying ev_prepare.
	 */
	struct ev_prepare prepare;

	/**
	 * (read-only)
	 *
	 * Events received from the libev callback.
	 */
	int revents;
};

/**
 * \struct cprepare_coro
 * \brief Convenient coroutine coupled with a prepare.
 */
struct cprepare_coro {
	/**
	 * Underlying watcher to use.
	 */
	struct cprepare prepare;

	/**
	 * Underlying coroutine.
	 */
	struct coro coro;

	/**
	 * (private)
	 */
	cprepare_coro_entry_t entry;

	/**
	 * (private)
	 */
	cprepare_coro_finalizer_t finalizer;
};

/**
 * \struct cprepare_coro_def
 * \brief Watcher coroutine definition.
 *
 * This structure is used as a descriptor for ::cprepare_coro_spawn.
 */
struct cprepare_coro_def {
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
	cprepare_coro_entry_t entry;

	/**
	 * (optional)
	 *
	 * Coroutine finalizer.
	 *
	 * This user function is called after the coroutine watcher has been
	 * cleanup itself.
	 */
	cprepare_coro_finalizer_t finalizer;
};

/**
 * Initialize defaults.
 *
 * This function is not required if you directly use ::cprepare_use or
 * ::cprepare_create but is provided if you wish to call ::cprepare_finish
 * prematurly.
 */
void
cprepare_init(struct cprepare *ev);

/**
 * Start the event watcher.
 *
 * This function is the `ev_prepare_start` equivalent, it does nothing
 * regarding the internal coroutine.
 *
 * Caller must have a coroutine calling cprepare_wait indefinitely until the
 * watcher is stopped.
 *
 * No-op if the watcher is already active
 */
void
cprepare_start(EV_P_ struct cprepare *ev);

/**
 * Indicate if the watcher is active.
 *
 * \return non-zero if the watcher was started
 * \sa ::cprepare_start
 */
int
cprepare_active(const struct cprepare *ev);

/**
 * Feed an event to the watcher.
 *
 * It is equivalent to ev_feed_event.
 */
void
cprepare_feed(EV_P_ struct cprepare *ev, int events);

/**
 * Stop the event watcher.
 *
 * This function is the `ev_prepare_sttop` equivalent, it does nothing
 * regarding the internal coroutine.
 *
 * No-op if the watcher is already inactive.
 */
void
cprepare_stop(EV_P_ struct cprepare *ev);

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
cprepare_ready(struct cprepare *ev);

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
 * actually blocking on ::cprepare_wait. Resuming a coroutine that is waiting
 * on ::cprepare_wait while the watcher isn't ready nor started won't return
 * and will yield again until condition is met.
 *
 * It is perfectly safe to call this function even if the watcher is stopped.
 *
 * This function **yields**.
 *
 * \return the watcher revents
 */
int
cprepare_wait(EV_P_ struct cprepare *ev);

/**
 * Cleanup internal resources.
 *
 * \pre watcher must be stopped
 */
void
cprepare_finish(struct cprepare *ev);

/**
 * Initialize watcher and its coroutine.
 *
 * This is equivalent to calling ::cprepare_init followed by ::coro_init.
 */
void
cprepare_coro_init(struct cprepare_coro *evco);

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
cprepare_coro_spawn(EV_P_ struct cprepare_coro *evco, const struct cprepare_coro_def *def);

/**
 * Stop the internal watcher and destroy it along with its dedicated coroutine.
 *
 * Do not call this function within a ::cprepare_coro_def::finalizer callback.
 */
void
cprepare_coro_finish(struct cprepare_coro *evco);

#endif /* !LIBCORO_CPREPARE_H */
