/*
 * nce/prepare.h -- coroutine watcher support for ev_prepare
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

#ifndef LIBNCE_PREPARE_H
#define LIBNCE_PREPARE_H

/**
 * \file nce/prepare.h
 * \brief Coroutine watcher support for ev_prepare.
 * \ingroup libnce-watchers
 */
#include <stddef.h>

#include <ev.h>

#include "coro.h"

#if defined(DOXYGEN)
#define EV_P_ struct ev_loop *,
#endif

#if defined(DOXYGEN)
/**
 * Convenient ::NCE_CONTAINER_OF macro for ::nce_prepare.
 */
#define NCE_PREPARE(Ptr, Field)
#else
/**
 * Convenient ::NCE_CONTAINER_OF macro for ::nce_prepare.
 */
#define NCE_PREPARE(Ptr, Field) \
	(NCE_CONTAINER_OF(Ptr, struct nce_prepare, Field))
#endif

struct nce_prepare;
struct nce_prepare_coro;
struct nce_prepare_coro_args;

/**
 * \brief Coroutine entrypoint for prepare.
 *
 * Similar to ::nce_coro_entry_t but it receives its watcher as argument.
 */
typedef void (* nce_prepare_coro_entry_t)(EV_P_ struct nce_prepare *self);

/**
 * \brief Finalizer function.
 *
 * Similar to ::nce_coro_finalizer_t but let the user perform extra step on a
 * coroutine watcher.
 */
typedef void (* nce_prepare_coro_finalizer_t)(EV_P_ struct nce_prepare *self);

/**
 * \struct nce_prepare
 * \brief Event watcher for ev_prepare.
 */
struct nce_prepare {
	/**
	 * (read-only)
	 *
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
 * \struct nce_prepare_coro
 * \brief Convenient coroutine coupled with a prepare.
 */
struct nce_prepare_coro {
	/**
	 * (read-write)
	 *
	 * Underlying watcher to use.
	 */
	struct nce_prepare prepare;

	/**
	 * (read-write)
	 *
	 * Coroutine attached to this watcher.
	 *
	 * Caller can set fields just like a normal coroutine however:
	 *
	 * The field ::coro::entry is replaced with an internal callback calling
	 * ::nce_prepare_coro::entry instead.
	 */
	struct nce_coro coro;

	/**
	 * (init)
	 *
	 * Coroutine watcher entrypoint.
	 */
	nce_prepare_coro_entry_t entry;

	/**
	 * (optional)
	 *
	 * Finalizer for the coroutine watcher.
	 */
	nce_prepare_coro_finalizer_t finalizer;
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \struct nce_prepare_coro_args
 * \brief Options for ::nce_prepare_coro_spawn.
 */
struct nce_prepare_coro_args {
#if defined(DOXYGEN)
	/**
	 * (private)
	 *
	 * Dummy field because no options.
	 */
	int dummy;
#endif
};

/**
 * Start the event watcher.
 *
 * It is equivalent to ev_prepare_start.
 *
 * No-op if the watcher is already active
 */
void
nce_prepare_start(EV_P_ struct nce_prepare *ev);

/**
 * Indicate if the watcher is active.
 *
 * \return non-zero if the watcher was started
 * \sa ::nce_prepare_start
 */
int
nce_prepare_active(const struct nce_prepare *ev);

/**
 * Feed an event to the watcher.
 *
 * It is equivalent to ev_feed_event.
 */
void
nce_prepare_feed(EV_P_ struct nce_prepare *ev, int events);

/**
 * Stop the event watcher.
 *
 * It is equivalent to ev_prepare_stop.
 *
 * No-op if the watcher is already inactive.
 */
void
nce_prepare_stop(EV_P_ struct nce_prepare *ev);

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
nce_prepare_ready(struct nce_prepare *ev);

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
 * actually blocking on ::nce_prepare_wait. Resuming a coroutine that is
 * waiting on ::nce_prepare_wait while the watcher isn't ready nor started
 * won't return and will yield again until condition is true.
 *
 * It is perfectly safe to call this function even if the watcher is stopped.
 *
 * This function may **yield**.
 *
 * \return the watcher revents
 */
int
nce_prepare_wait(EV_P_ struct nce_prepare *ev);


/**
 * This all in one function initialize, set and optionnally start the watcher
 * and immediately creates its dedicated coroutine which is also started
 * automatically.
 *
 * \param args additional watcher spawn arguments (maybe NULL)
 * \return refer to ::nce_coro_spawn
 */
int
nce_prepare_coro_spawn(EV_P_ struct nce_prepare_coro *evco, const struct nce_prepare_coro_args *args);

/**
 * Stop the internal watcher and destroy it along with its dedicated coroutine.
 *
 * Do not call this function within a ::nce_prepare_coro::finalizer callback.
 */
void
nce_prepare_coro_destroy(struct nce_prepare_coro *evco);

#ifdef __cplusplus
}
#endif

#endif /* !LIBNCE_NCE_PREPARE_H */
