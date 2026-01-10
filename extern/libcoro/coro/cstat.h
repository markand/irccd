/*
 * cstat.h -- coroutine watcher support for ev_stat
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

#ifndef LIBCORO_CSTAT_H
#define LIBCORO_CSTAT_H

/**
 * \file coro/cstat.h
 * \brief Coroutine watcher support for ev_stat.
 * \ingroup libcoro-ev-watchers
 */

#include <stddef.h>

#include <ev.h>

#include "coro.h"

#if defined(DOXYGEN)
#define EV_P_ struct ev_loop *,
#endif

/**
 * \brief Convenient ::CORO_CONTAINER_OF macro for ::cstat.
 */
#define CSTAT(Ptr, Field) \
	(CORO_CONTAINER_OF(Ptr, struct cstat, Field))

struct cstat;
struct cstat_coro;
struct cstat_coro_def;

/**
 * \brief Coroutine entrypoint for stat.
 *
 * Similar to ::coro_entry_t but it receives its watcher as argument.
 */
typedef void (*cstat_coro_entry_t)(EV_P_ struct cstat *self);

/**
 * \brief Finalizer function.
 *
 * Similar to ::coro_finalizer_t but let the user perform extra step on a
 * coroutine watcher.
 */
typedef void (*cstat_coro_finalizer_t)(EV_P_ struct cstat *self);

/**
 * \struct cstat
 * \brief Event watcher for ev_stat.
 */
struct cstat {
	/**
	 * Underlying ev_stat.
	 */
	struct ev_stat stat;

	/**
	 * (read-only)
	 *
	 * Events received from the libev callback.
	 */
	int revents;
};

/**
 * \struct cstat_coro
 * \brief Convenient coroutine coupled with a stat.
 */
struct cstat_coro {
	/**
	 * Underlying watcher to use.
	 */
	struct cstat stat;

	/**
	 * Underlying coroutine.
	 */
	struct coro coro;

	/**
	 * (private)
	 */
	cstat_coro_entry_t entry;

	/**
	 * (private)
	 */
	cstat_coro_finalizer_t finalizer;
};

/**
 * \struct cstat_coro_def
 * \brief Watcher coroutine definition.
 *
 * This structure is used as a descriptor for ::cstat_coro_spawn.
 */
struct cstat_coro_def {
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
	cstat_coro_entry_t entry;

	/**
	 * (optional)
	 *
	 * Coroutine finalizer.
	 *
	 * This user function is called after the coroutine watcher has been
	 * cleanup itself.
	 */
	cstat_coro_finalizer_t finalizer;

	/**
	 * Path to monitor.
	 */
	const char *path;

	/**
	 * Interval for monitoring the path.
	 */
	ev_tstamp interval;
};

/**
 * Initialize defaults.
 *
 * This function is not required if you directly use ::cstat_use or
 * ::cstat_create but is provided if you wish to call ::cstat_finish
 * prematurly.
 */
void
cstat_init(struct cstat *ev);

/**
 * Start the event watcher.
 *
 * This function is the `ev_stat_start` equivalent, it does nothing
 * regarding the internal coroutine.
 *
 * Caller must have a coroutine calling cstat_wait indefinitely until the
 * watcher is stopped.
 *
 * No-op if the watcher is already active
 */
void
cstat_start(EV_P_ struct cstat *ev);

/**
 * Indicate if the watcher is active.
 *
 * \return non-zero if the watcher was started
 * \sa ::cstat_start
 */
int
cstat_active(const struct cstat *ev);

/**
 * Feed an event to the watcher.
 *
 * It is equivalent to ev_feed_event.
 */
void
cstat_feed(EV_P_ struct cstat *ev, int events);

/**
 * Stop the event watcher.
 *
 * This function is the `ev_stat_sttop` equivalent, it does nothing
 * regarding the internal coroutine.
 *
 * No-op if the watcher is already inactive.
 */
void
cstat_stop(EV_P_ struct cstat *ev);

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
cstat_ready(struct cstat *ev);

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
 * actually blocking on ::cstat_wait. Resuming a coroutine that is waiting
 * on ::cstat_wait while the watcher isn't ready nor started won't return
 * and will yield again until condition is met.
 *
 * It is perfectly safe to call this function even if the watcher is stopped.
 *
 * This function **yields**.
 *
 * \return the watcher revents
 */
int
cstat_wait(EV_P_ struct cstat *ev);

/**
 * Configure watcher.
 *
 * This function is equivalent to ev_stat_set.
 */
void
cstat_set(EV_P_ struct cstat *ev, const char *path, ev_tstamp interval);

/**
 * Update internal stat values immediately.
 *
 * This function is equivalent to ev_stat_set.
 */
void
cstat_stat(EV_P_ struct cstat *ev);

/**
 * Cleanup internal resources.
 *
 * \pre watcher must be stopped
 */
void
cstat_finish(struct cstat *ev);

/**
 * Initialize watcher and its coroutine.
 *
 * This is equivalent to calling ::cstat_init followed by ::coro_init.
 */
void
cstat_coro_init(struct cstat_coro *evco);

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
cstat_coro_spawn(EV_P_ struct cstat_coro *evco, const struct cstat_coro_def *def);

/**
 * Stop the internal watcher and destroy it along with its dedicated coroutine.
 *
 * Do not call this function within a ::cstat_coro_def::finalizer callback.
 */
void
cstat_coro_finish(struct cstat_coro *evco);

#endif /* !LIBCORO_CSTAT_H */
