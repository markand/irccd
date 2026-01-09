/*
 * cperiodic.h -- coroutine watcher support for ev_periodic
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

#ifndef LIBCORO_CPERIODIC_H
#define LIBCORO_CPERIODIC_H

/**
 * \file coro/cperiodic.h
 * \brief Coroutine watcher support for ev_periodic.
 * \ingroup libcoro-ev-watchers
 */

#include <stddef.h>

#include <ev.h>

#include "coro.h"

#if defined(DOXYGEN)
#define EV_P_ struct ev_loop *,
#endif

/**
 * \brief Convenient ::CORO_CONTAINER_OF macro for ::cperiodic.
 */
#define CPERIODIC(Ptr, Field) \
	(CORO_CONTAINER_OF(Ptr, struct cperiodic, Field))

struct cperiodic;
struct cperiodic_coro;
struct cperiodic_coro_def;

/**
 * \brief Coroutine entrypoint for periodic.
 */
typedef void (*cperiodic_coro_entry_t)(EV_P_ struct cperiodic *self);

/**
 * Function wrapping ev_periodic's rescheduler callback.
 */
typedef ev_tstamp (*cperiodic_rescheduler_t)(struct cperiodic *self, ev_tstamp now);

/**
 * \struct cperiodic
 * \brief Event watcher for ev_periodic.
 */
struct cperiodic {
	/**
	 * Underlying ev_periodic.
	 */
	struct ev_periodic periodic;

	/**
	 * (read-only)
	 *
	 * Events received from the libev callback.
	 */
	int revents;

	/**
	 * (optional)
	 *
	 * When using a rescheduler, this function pointer wraps ev_periodic
	 * rescheduler_cb to provide cperiodic as argument.
	 */
	cperiodic_rescheduler_t rescheduler;
};

/**
 * \struct cperiodic_coro
 * \brief Convenient coroutine coupled with a periodic.
 */
struct cperiodic_coro {
	/**
	 * Underlying watcher to use.
	 */
	struct cperiodic periodic;

	/**
	 * Underlying coroutine.
	 */
	struct coro coro;

	/**
	 * Watcher coroutine entrypoint.
	 */
	cperiodic_coro_entry_t entry;
};

/**
 * \struct cperiodic_coro_def
 * \brief Watcher coroutine definition.
 *
 * This structure is used as a descriptor for ::cperiodic_coro_spawn.
 */
struct cperiodic_coro_def {
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
	cperiodic_coro_entry_t entry;

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
	 * See ::cwatcher_set.
	 */
	ev_tstamp offset;

	/**
	 * See ::cwatcher_set.
	 */
	ev_tstamp interval;

	/**
	 * See ::cwatcher_set.
	 */
	cperiodic_rescheduler_t rescheduler;
};

/**
 * Initialize defaults.
 *
 * This function is not required if you directly use ::cperiodic_use or
 * ::cperiodic_create but is provided if you wish to call ::cperiodic_finish
 * prematurly.
 */
void
cperiodic_init(struct cperiodic *ev);

/**
 * Start the event watcher.
 *
 * This function is the `ev_periodic_start` equivalent, it does nothing
 * regarding the internal coroutine.
 *
 * Caller must have a coroutine calling cperiodic_wait indefinitely until the
 * watcher is stopped.
 *
 * No-op if the watcher is already active
 */
void
cperiodic_start(EV_P_ struct cperiodic *ev);

/**
 * Indicate if the watcher is active.
 *
 * \return non-zero if the watcher was started
 * \sa ::cperiodic_start
 */
int
cperiodic_active(const struct cperiodic *ev);

/**
 * Feed an event to the watcher.
 *
 * It is equivalent to ev_feed_event.
 */
void
cperiodic_feed(EV_P_ struct cperiodic *ev, int events);

/**
 * Stop the event watcher.
 *
 * This function is the `ev_periodic_sttop` equivalent, it does nothing
 * regarding the internal coroutine.
 *
 * No-op if the watcher is already inactive.
 */
void
cperiodic_stop(EV_P_ struct cperiodic *ev);

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
cperiodic_ready(struct cperiodic *ev);

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
 * actually blocking on ::cperiodic_wait. Resuming a coroutine that is waiting
 * on ::cperiodic_wait while the watcher isn't ready nor started won't return
 * and will yield again until condition is met.
 *
 * It is perfectly safe to call this function even if the watcher is stopped.
 *
 * This function **yields**.
 *
 * \return the watcher revents
 */
int
cperiodic_wait(EV_P_ struct cperiodic *ev);

/**
 * Configure periodic interval, offset and optional rescheduler.
 *
 * It is equivalent to ev_periodic_set.
 */
void
cperiodic_set(struct cperiodic *ev,
              ev_tstamp offset,
              ev_tstamp interval,
              cperiodic_rescheduler_t rescheduler
);

/**
 * Cleanup internal resources.
 *
 * \pre watcher must be stopped
 */
void
cperiodic_finish(struct cperiodic *ev);

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
cperiodic_coro_spawn(EV_P_ struct cperiodic_coro *coro, const struct cperiodic_coro_def *def);

#endif /* !LIBCORO_CPERIODIC_H */
