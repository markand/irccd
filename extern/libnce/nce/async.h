/*
 * nce/async.h -- coroutine watcher support for ev_async
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

#ifndef LIBNCE_ASYNC_H
#define LIBNCE_ASYNC_H

/**
 * \file nce/async.h
 * \brief Coroutine watcher support for ev_async.
 * \ingroup libnce-watchers
 */

#include "nce.h"

#if defined(DOXYGEN)
#define EV_P_ struct ev_loop *,
#endif

#if defined(DOXYGEN)
/**
 * Convenient ::NCE_CONTAINER_OF macro for ::nce_async.
 */
#define NCE_ASYNC(Ptr, Field)
#else
/**
 * Convenient ::NCE_CONTAINER_OF macro for ::nce_async.
 */
#define NCE_ASYNC(Ptr, Field) \
	(NCE_CONTAINER_OF(Ptr, struct nce_async, Field))

/**
 * Convenient ::NCE_CONTAINER_OF macro for ::nce_async_coro.
 */
#define NCE_ASYNC_CORO(Ptr, Field) \
	(NCE_CONTAINER_OF(Ptr, struct nce_async_coro, Field))
#endif

struct nce_async;
struct nce_async_coro;
struct nce_async_coro_args;


/**
 * \struct nce_async
 * \brief Event watcher for ev_async.
 */
struct nce_async {
	/**
	 * (read-only)
	 *
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
 * \struct nce_async_coro
 * \brief Convenient coroutine coupled with a async.
 */
struct nce_async_coro {
	/**
	 * (read-write)
	 *
	 * Underlying watcher to use.
	 */
	struct nce_async async;

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
 * It is equivalent to ev_async_start.
 *
 * No-op if the watcher is already active
 */
void
nce_async_start(EV_P_ struct nce_async *ev);

/**
 * Indicate if the watcher is active.
 *
 * \return non-zero if the watcher was started
 * \sa ::nce_async_start
 */
int
nce_async_active(const struct nce_async *ev);

/**
 * Feed an event to the watcher.
 *
 * It is equivalent to ev_feed_event.
 */
void
nce_async_feed(EV_P_ struct nce_async *ev, int events);

/**
 * Stop the event watcher.
 *
 * It is equivalent to ev_async_stop.
 *
 * No-op if the watcher is already inactive.
 */
void
nce_async_stop(EV_P_ struct nce_async *ev);

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
nce_async_ready(struct nce_async *ev);

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
 * actually blocking on ::nce_async_wait. Resuming a coroutine that is
 * waiting on ::nce_async_wait while the watcher isn't ready nor started
 * won't return and will yield again until condition is true.
 *
 * It is perfectly safe to call this function even if the watcher is stopped.
 *
 * This function may **yield**.
 *
 * \return the watcher revents
 */
int
nce_async_wait(struct nce_async *ev);

/**
 * Spawn a coroutine with an embedded `ev_async`.
 */
int
nce_async_coro_spawn(EV_P_ struct nce_async_coro *evco);
/**
 * Usable callback function as ::nce_coro::terminate to stop the ::nce_async
 * when destroying the coroutine.
 */
void
nce_async_coro_terminate(EV_P_ struct nce_coro *self);

/**
 * Destroy the watcher and its coroutine.
 *
 * The watcher is stopped **before** destroying the coroutine.
 */
void
nce_async_coro_destroy(EV_P_ struct nce_async_coro *evco);

#ifdef __cplusplus
}
#endif

#endif /* !LIBNCE_NCE_ASYNC_H */
