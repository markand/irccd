/*
 * nce/check.h -- coroutine watcher support for ev_check
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

#ifndef LIBNCE_CHECK_H
#define LIBNCE_CHECK_H

/**
 * \file nce/check.h
 * \brief Coroutine watcher support for ev_check.
 * \ingroup libnce-watchers
 */

#include "nce.h"

#if defined(DOXYGEN)
#define EV_P_ struct ev_loop *,
#endif

#if defined(DOXYGEN)
/**
 * Convenient ::NCE_CONTAINER_OF macro for ::nce_check.
 */
#define NCE_CHECK(Ptr, Field)
#else
/**
 * Convenient ::NCE_CONTAINER_OF macro for ::nce_check.
 */
#define NCE_CHECK(Ptr, Field) \
	(NCE_CONTAINER_OF(Ptr, struct nce_check, Field))

/**
 * Convenient ::NCE_CONTAINER_OF macro for ::nce_check_coro.
 */
#define NCE_CHECK_CORO(Ptr, Field) \
	(NCE_CONTAINER_OF(Ptr, struct nce_check_coro, Field))
#endif

struct nce_check;
struct nce_check_coro;
struct nce_check_coro_args;


/**
 * \struct nce_check
 * \brief Event watcher for ev_check.
 */
struct nce_check {
	/**
	 * (read-only)
	 *
	 * Underlying ev_check.
	 */
	struct ev_check check;

	/**
	 * (read-only)
	 *
	 * Events received from the libev callback.
	 */
	int revents;
};

/**
 * \struct nce_check_coro
 * \brief Convenient coroutine coupled with a check.
 */
struct nce_check_coro {
	/**
	 * (read-write)
	 *
	 * Underlying watcher to use.
	 */
	struct nce_check check;

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
 * It is equivalent to ev_check_start.
 *
 * No-op if the watcher is already active
 */
void
nce_check_start(EV_P_ struct nce_check *ev);

/**
 * Indicate if the watcher is active.
 *
 * \return non-zero if the watcher was started
 * \sa ::nce_check_start
 */
int
nce_check_active(const struct nce_check *ev);

/**
 * Feed an event to the watcher.
 *
 * It is equivalent to ev_feed_event.
 */
void
nce_check_feed(EV_P_ struct nce_check *ev, int events);

/**
 * Stop the event watcher.
 *
 * It is equivalent to ev_check_stop.
 *
 * No-op if the watcher is already inactive.
 */
void
nce_check_stop(EV_P_ struct nce_check *ev);

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
nce_check_ready(struct nce_check *ev);

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
 * actually blocking on ::nce_check_wait. Resuming a coroutine that is
 * waiting on ::nce_check_wait while the watcher isn't ready nor started
 * won't return and will yield again until condition is true.
 *
 * It is perfectly safe to call this function even if the watcher is stopped.
 *
 * This function may **yield**.
 *
 * \return the watcher revents
 */
int
nce_check_wait(struct nce_check *ev);

/**
 * Spawn a coroutine with an embedded `ev_check`.
 */
int
nce_check_coro_spawn(EV_P_ struct nce_check_coro *evco);
/**
 * Usable callback function as ::nce_coro::terminate to stop the ::nce_check
 * when destroying the coroutine.
 */
void
nce_check_coro_terminate(EV_P_ struct nce_coro *self);

/**
 * Destroy the watcher and its coroutine.
 *
 * The watcher is stopped **before** destroying the coroutine.
 */
void
nce_check_coro_destroy(EV_P_ struct nce_check_coro *evco);

#ifdef __cplusplus
}
#endif

#endif /* !LIBNCE_NCE_CHECK_H */
