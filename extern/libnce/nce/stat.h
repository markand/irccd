/*
 * nce/stat.h -- coroutine watcher support for ev_stat
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

#ifndef LIBNCE_STAT_H
#define LIBNCE_STAT_H

/**
 * \file nce/stat.h
 * \brief Coroutine watcher support for ev_stat.
 * \ingroup libnce-watchers
 */

#include "nce.h"

#if defined(DOXYGEN)
#define EV_P_ struct ev_loop *,
#endif

#if defined(DOXYGEN)
/**
 * Convenient ::NCE_CONTAINER_OF macro for ::nce_stat.
 */
#define NCE_STAT(Ptr, Field)
#else
/**
 * Convenient ::NCE_CONTAINER_OF macro for ::nce_stat.
 */
#define NCE_STAT(Ptr, Field) \
	(NCE_CONTAINER_OF(Ptr, struct nce_stat, Field))

/**
 * Convenient ::NCE_CONTAINER_OF macro for ::nce_stat_coro.
 */
#define NCE_STAT_CORO(Ptr, Field) \
	(NCE_CONTAINER_OF(Ptr, struct nce_stat_coro, Field))
#endif

struct nce_stat;
struct nce_stat_coro;
struct nce_stat_coro_args;


/**
 * \struct nce_stat
 * \brief Event watcher for ev_stat.
 */
struct nce_stat {
	/**
	 * (read-only)
	 *
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
 * \struct nce_stat_coro
 * \brief Convenient coroutine coupled with a stat.
 */
struct nce_stat_coro {
	/**
	 * (read-write)
	 *
	 * Underlying watcher to use.
	 */
	struct nce_stat stat;

	/**
	 * (read-write)
	 *
	 * Coroutine attached to this watcher.
	 */
	struct nce_coro coro;
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Start the event watcher.
 *
 * It is equivalent to ev_stat_start.
 *
 * No-op if the watcher is already active
 */
void
nce_stat_start(EV_P_ struct nce_stat *ev);

/**
 * Indicate if the watcher is active.
 *
 * \return non-zero if the watcher was started
 * \sa ::nce_stat_start
 */
int
nce_stat_active(const struct nce_stat *ev);

/**
 * Feed an event to the watcher.
 *
 * It is equivalent to ev_feed_event.
 */
void
nce_stat_feed(EV_P_ struct nce_stat *ev, int events);

/**
 * Stop the event watcher.
 *
 * It is equivalent to ev_stat_stop.
 *
 * No-op if the watcher is already inactive.
 */
void
nce_stat_stop(EV_P_ struct nce_stat *ev);

/**
 * Return internal watcher events.
 *
 * If events have been received they are removed from the watcher and returned
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
nce_stat_ready(struct nce_stat *ev);

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
 * actually blocking on ::nce_stat_wait. Resuming a coroutine that is
 * waiting on ::nce_stat_wait while the watcher isn't ready nor started
 * won't return and will yield again until condition is true.
 *
 * It is perfectly safe to call this function even if the watcher is stopped.
 *
 * This function may **yield**.
 *
 * \return the watcher revents
 */
int
nce_stat_wait(struct nce_stat *ev);

/**
 * Configure watcher.
 *
 * This function is equivalent to ev_stat_set.
 */
void
nce_stat_set(struct nce_stat *ev, const char *path, ev_tstamp interval);

/**
 * Update internal stat values immediately.
 *
 * This function is equivalent to ev_stat_set.
 */
void
nce_stat_stat(EV_P_ struct nce_stat *ev);

/**
 * Spawn a coroutine with an embedded `ev_stat`.
 *
 * Arguments are similar to ::nce_stat_set.
 */
int
nce_stat_coro_spawn(EV_P_ struct nce_stat_coro *evco,
                          const char *path,
                          ev_tstamp interval);
/**
 * Usable callback function as ::nce_coro::terminate to stop the ::nce_stat
 * when destroying the coroutine.
 */
void
nce_stat_coro_terminate(EV_P_ struct nce_coro *self);

/**
 * Destroy the watcher and its coroutine.
 *
 * The watcher is stopped **before** destroying the coroutine.
 */
void
nce_stat_coro_destroy(EV_P_ struct nce_stat_coro *evco);

#ifdef __cplusplus
}
#endif

#endif /* !LIBNCE_NCE_STAT_H */
