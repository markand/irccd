/*
 * cchild.h -- coroutine watcher support for ev_child
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

#ifndef LIBCORO_CCHILD_H
#define LIBCORO_CCHILD_H

/**
 * \file coro/cchild.h
 * \brief Coroutine watcher support for ev_child.
 * \ingroup libcoro-ev-watchers
 */

#include <stddef.h>

#include <ev.h>

#include "coro.h"

#if defined(DOXYGEN)
#define EV_P_ struct ev_loop *,
#endif

/**
 * \brief Convenient ::CORO_CONTAINER_OF macro for ::cchild.
 */
#define CCHILD(Ptr, Field) \
	(CORO_CONTAINER_OF(Ptr, struct cchild, Field))

struct cchild;
struct cchild_coro;
struct cchild_coro_def;

/**
 * \brief Coroutine entrypoint for child.
 *
 * Similar to ::coro_entry_t but it receives its watcher as argument.
 */
typedef void (*cchild_coro_entry_t)(EV_P_ struct cchild *self);

/**
 * \brief Finalizer function.
 *
 * Similar to ::coro_finalizer_t but let the user perform extra step on a
 * coroutine watcher.
 */
typedef void (*cchild_coro_finalizer_t)(EV_P_ struct cchild *self);

/**
 * \struct cchild
 * \brief Event watcher for ev_child.
 */
struct cchild {
	/**
	 * Underlying ev_child.
	 */
	struct ev_child child;

	/**
	 * (read-only)
	 *
	 * Events received from the libev callback.
	 */
	int revents;
};

/**
 * \struct cchild_coro
 * \brief Convenient coroutine coupled with a child.
 */
struct cchild_coro {
	/**
	 * Underlying watcher to use.
	 */
	struct cchild child;

	/**
	 * Underlying coroutine.
	 */
	struct coro coro;

	/**
	 * (private)
	 */
	cchild_coro_entry_t entry;

	/**
	 * (private)
	 */
	cchild_coro_finalizer_t finalizer;
};

/**
 * \struct cchild_coro_def
 * \brief Watcher coroutine definition.
 *
 * This structure is used as a descriptor for ::cchild_coro_spawn.
 */
struct cchild_coro_def {
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
	cchild_coro_entry_t entry;

	/**
	 * (optional)
	 *
	 * Coroutine finalizer.
	 *
	 * This user function is called after the coroutine watcher has been
	 * cleanup itself.
	 */
	cchild_coro_finalizer_t finalizer;

	/**
	 * Process PID to monitor.
	 */
	pid_t pid;

	/**
	 * Set non-zero to monitor stopped/continued events too.
	 */
	int trace;
};

/**
 * Initialize defaults.
 *
 * This function is not required if you directly use ::cchild_use or
 * ::cchild_create but is provided if you wish to call ::cchild_finish
 * prematurly.
 */
void
cchild_init(struct cchild *ev);

/**
 * Start the event watcher.
 *
 * This function is the `ev_child_start` equivalent, it does nothing
 * regarding the internal coroutine.
 *
 * Caller must have a coroutine calling cchild_wait indefinitely until the
 * watcher is stopped.
 *
 * No-op if the watcher is already active
 */
void
cchild_start(EV_P_ struct cchild *ev);

/**
 * Indicate if the watcher is active.
 *
 * \return non-zero if the watcher was started
 * \sa ::cchild_start
 */
int
cchild_active(const struct cchild *ev);

/**
 * Feed an event to the watcher.
 *
 * It is equivalent to ev_feed_event.
 */
void
cchild_feed(EV_P_ struct cchild *ev, int events);

/**
 * Stop the event watcher.
 *
 * This function is the `ev_child_sttop` equivalent, it does nothing
 * regarding the internal coroutine.
 *
 * No-op if the watcher is already inactive.
 */
void
cchild_stop(EV_P_ struct cchild *ev);

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
cchild_ready(struct cchild *ev);

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
 * actually blocking on ::cchild_wait. Resuming a coroutine that is waiting
 * on ::cchild_wait while the watcher isn't ready nor started won't return
 * and will yield again until condition is met.
 *
 * It is perfectly safe to call this function even if the watcher is stopped.
 *
 * This function **yields**.
 *
 * \return the watcher revents
 */
int
cchild_wait(EV_P_ struct cchild *ev);

/**
 * Configure watcher.
 *
 * This function is equivalent to ev_child_set.
 */
void
cchild_set(struct cchild *ev, pid_t pid, int trace);

/**
 * Cleanup internal resources.
 *
 * \pre watcher must be stopped
 */
void
cchild_finish(struct cchild *ev);

/**
 * Initialize watcher and its coroutine.
 *
 * This is equivalent to calling ::cchild_init followed by ::coro_init.
 */
void
cchild_coro_init(struct cchild_coro *evco);

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
cchild_coro_spawn(EV_P_ struct cchild_coro *evco, const struct cchild_coro_def *def);

/**
 * Stop the internal watcher and destroy it along with its dedicated coroutine.
 *
 * Do not call this function within a ::cchild_coro_def::finalizer callback.
 */
void
cchild_coro_finish(struct cchild_coro *evco);

#endif /* !LIBCORO_CCHILD_H */
