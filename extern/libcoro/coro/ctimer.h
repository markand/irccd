/*
 * ctimer.h -- coroutine watcher support for ev_timer
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

#ifndef LIBCORO_CTIMER_H
#define LIBCORO_CTIMER_H

/**
 * \file coro/ctimer.h
 * \brief Coroutine watcher support for ev_timer.
 * \ingroup libcoro-ev-watchers
 */

#include <stddef.h>

#include <ev.h>

#include "coro.h"

#if defined(DOXYGEN)
#define EV_P_ struct ev_loop *,
#endif

/**
 * \brief Convenient ::CORO_CONTAINER_OF macro for ::ctimer.
 */
#define CTIMER(Ptr, Field) \
	(CORO_CONTAINER_OF(Ptr, struct ctimer, Field))

struct ctimer;
struct ctimer_coro;
struct ctimer_coro_def;

/**
 * \brief Coroutine entrypoint for timer.
 */
typedef void (*ctimer_coro_entry_t)(EV_P_ struct ctimer *self);

/**
 * \struct ctimer
 * \brief Event watcher for ev_timer.
 */
struct ctimer {
	/**
	 * Underlying ev_timer.
	 */
	struct ev_timer timer;

	/**
	 * (read-only)
	 *
	 * Events received from the libev callback.
	 */
	int revents;
};

/**
 * \struct ctimer_coro
 * \brief Convenient coroutine coupled with a timer.
 */
struct ctimer_coro {
	/**
	 * Underlying watcher to use.
	 */
	struct ctimer timer;

	/**
	 * Underlying coroutine.
	 */
	struct coro coro;

	/**
	 * Watcher coroutine entrypoint.
	 */
	ctimer_coro_entry_t entry;
};

/**
 * \struct ctimer_coro_def
 * \brief Watcher coroutine definition.
 *
 * This structure is used as a descriptor for ::ctimer_coro_spawn.
 */
struct ctimer_coro_def {
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
	ctimer_coro_entry_t entry;

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
	ev_tstamp after;
	ev_tstamp repeat;
};

/**
 * Initialize defaults.
 *
 * This function is not required if you directly use ::ctimer_use or
 * ::ctimer_create but is provided if you wish to call ::ctimer_finish
 * prematurly.
 */
void
ctimer_init(struct ctimer *ev);

/**
 * Start the event watcher.
 *
 * This function is the `ev_timer_start` equivalent, it does nothing
 * regarding the internal coroutine.
 *
 * Caller must have a coroutine calling ctimer_wait indefinitely until the
 * watcher is stopped.
 *
 * No-op if the watcher is already active
 */
void
ctimer_start(EV_P_ struct ctimer *ev);

/**
 * Indicate if the watcher is active.
 *
 * \return non-zero if the watcher was started
 * \sa ::ctimer_start
 */
int
ctimer_active(const struct ctimer *ev);

/**
 * Feed an event to the watcher.
 *
 * It is equivalent to ev_feed_event.
 */
void
ctimer_feed(EV_P_ struct ctimer *ev, int events);

/**
 * Stop the event watcher.
 *
 * This function is the `ev_timer_sttop` equivalent, it does nothing
 * regarding the internal coroutine.
 *
 * No-op if the watcher is already inactive.
 */
void
ctimer_stop(EV_P_ struct ctimer *ev);

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
ctimer_ready(struct ctimer *ev);

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
 * actually blocking on ::ctimer_wait. Resuming a coroutine that is waiting
 * on ::ctimer_wait while the watcher isn't ready nor started won't return
 * and will yield again until condition is met.
 *
 * It is perfectly safe to call this function even if the watcher is stopped.
 *
 * This function **yields**.
 *
 * \return the watcher revents
 */
int
ctimer_wait(EV_P_ struct ctimer *ev);

/**
 * Configure watcher.
 *
 * This function is equivalent to ev_timer_set.
 */
void
ctimer_set(struct ctimer *ev, ev_tstamp after, ev_tstamp repeat);

/**
 * This function stops the watcher, set its new values and start it again.
 *
 * There is no equivalent in libev.
 */
void
ctimer_restart(EV_P_ struct ctimer *ev, ev_tstamp after, ev_tstamp repeat);

/**
 * Rearm the timer.
 *
 * This function is equivalent to ev_timer_again.
 */
void
ctimer_again(EV_P_ struct ctimer *ev);

/**
 * Cleanup internal resources.
 *
 * \pre watcher must be stopped
 */
void
ctimer_finish(struct ctimer *ev);

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
ctimer_coro_spawn(EV_P_ struct ctimer_coro *coro, const struct ctimer_coro_def *def);

#endif /* !LIBCORO_CTIMER_H */
