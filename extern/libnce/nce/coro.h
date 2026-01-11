/*
 * coro.h -- coroutine library on top of libev
 *
 * Copyright (c) 2026 David Demelier <markand@malikania.fr>
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

#ifndef LIBNCE_NCE_CORO_H
#define LIBNCE_NCE_CORO_H

/**
 * \file nce/coro.h
 * \brief Coroutine library.
 * \ingroup libnce
 */

#include <stddef.h>

#include <ev.h>

#include "nce.h"

/**
 * Default coroutine name.
 */
#ifndef NCE_CORO_DEFAULT_NAME
#define NCE_CORO_DEFAULT_NAME "coroutine"
#endif

/**
 * Convenient ::NCE_CONTAINER_OF macro for ::nce_coro structure.
 */
#if defined(DOXYGEN)
#define NCE_CORO(Ptr, Field)
#else
#define CORO(Ptr, Field) \
	(NCE_CONTAINER_OF(Ptr, struct nce_coro, Field))
#endif

struct nce_coro;

/**
 * \brief Coroutine entrypoint.
 *
 * This function is executed when the coroutine is resumed for the first time.
 */
typedef void (* nce_coro_entry_t)(EV_P_ struct nce_coro *);

/**
 * \brief Finalizer function.
 *
 * This optional function can be set to perform an extra user finalization step
 * when an event loop destroy the coroutine.
 *
 * It is invoked with ::nce_coro_destroy.
 */
typedef void (* nce_coro_finalizer_t)(EV_P_ struct nce_coro *);

/**
 * If this flag is set, the coroutine is attached to the event loop and
 * is automatically resumed before and after the event loop iterated.
 *
 * This is usually the safest choice to use on "root" coroutines that
 * are resuming nested coroutines.
 *
 * If the coroutine terminates on its own it is disabled and destroyed but
 * program ev_run continues.
 *
 * When removed from the loop, ::nce_coro_destroy is called.
 */
#define NCE_CORO_ATTACHED (1 << 0)

/**
 * If this flag is set and if the coroutine that is resumed from the
 * event loop is no longer resumable the event loop is stopped.
 *
 * This flag implies ::NCE_CORO_ATTACHED.
 */
#define NCE_CORO_ESSENTIAL (1 << 1)

/**
 * This flag can be set as a hint so that the coroutine to be resumed
 * from the event loop is guaranteed to never terminate improving performances
 * as no check has to be made.
 *
 * When built unoptimized a check is still added and aborts if the case
 * still happen.
 *
 * This flag is mutually exclusive with ::NCE_CORO_ESSENTIAL.
 *
 * This flag implies ::NCE_CORO_ATTACHED.
 */
#define NCE_CORO_FOREVER (1 << 2)

/**
 * This flag is used when creating a coroutine associated with a watcher to
 * start the coroutine but with the watcher initially stopped.
 *
 * It is only meaningful with watcher coroutines spawner (e.g.
 * ::nce_timer_coro_spawn) and never used with ::nce_coro_spawn itself.
 */
#define NCE_CORO_INACTIVE (1 << 3)

#if defined(DOXYGEN)
/**
 * \struct nce_coro
 * \brief Coroutine object.
 */
struct nce_coro {
	/**
	 * (optional)
	 *
	 * Coroutine name.
	 *
	 * Mostly used for debugging purposes.
	 */
	const char *name;

	/**
	 * (optional)
	 *
	 * Change coroutine priority order.
	 *
	 * Only meaningful with attached coroutines.
	 *
	 * \sa ::NCE_CORO_ATTACHED
	 * \sa ::NCE_CORO_ESSENTIAL
	 * \sa ::NCE_CORO_FOREVER
	 */
	int priority;

	/**
	 * (optional)
	 *
	 * Coroutine optional behavior flags.
	 */
	unsigned int flags;

	/**
	 * (optional)
	 *
	 * Coroutine stack size to allocate.
	 *
	 * A value of 0 will use a library default.
	 */
	size_t stack_size;

	/**
	 * (init)
	 *
	 * Coroutine entrypoint.
	 */
	nce_coro_entry_t entry;

	/**
	 * (optional)
	 *
	 * Finalizer to be invoked by ::nce_coro_destroy before destroying it.
	 */
	nce_coro_finalizer_t finalizer;
};
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create the underlying coroutine.
 *
 * This function only creates the coroutine object, thus it is not started yet.
 *
 * \return 0 on success
 * \return -ENOMEM insufficent memory
 * \return -E<*> other errors happened
 */
int
nce_coro_create(EV_P_ struct nce_coro *coro);

/**
 * This function is similar to ::nce_coro_create but it also resumes the
 * coroutine immediately.
 *
 * \return refer to ::nce_coro_create.
 */
int
nce_coro_spawn(EV_P_ struct nce_coro *coro);

/**
 * Indicate if the coroutine is resumable (aka not started or suspended).
 *
 * \return non-zero if coroutine is resumable
 */
int
nce_coro_resumable(const struct nce_coro *coro);

/**
 * Start or resume the coroutine.
 *
 * If the coroutine is attached to the event loop (see ::nce_coro::attached) then
 * the coroutine is already resumed before and after event loop iteration.
 *
 * \pre coroutine **must** be resumable
 */
void
nce_coro_resume(struct nce_coro *coro);

/**
 * Yield the current coroutine.
 */
void
nce_coro_yield(void);

/**
 * Yield forever.
 */
void
nce_coro_idle(void);

/**
 * Continuously yield until ::nce_coro_on is called by another coroutine.
 *
 * This is useful for coroutines attached to the event loop but that needs to be
 * temporarily suspended and resumed by user explicit call to ::nce_coro_on.
 */
void
nce_coro_off(void);

/**
 * Resume a coroutine being suspend through use of ::nce_coro_off.
 *
 * This function will return immediately if the coroutine is already running.
 */
void
nce_coro_on(struct nce_coro *coro);

/**
 * This function is an alias to ::nce_coro_push using current coroutine.
 *
 * \sa ::nce_coro_push
 */
void
nce_coro_return(const void *data, size_t size);

/**
 * This function is similar to ::nce_coro_pull except that it yields the calling
 * coroutine until the data becomes available.
 *
 * \sa ::nce_coro_pull
 */
void
nce_coro_wait(void *data, size_t size);

/**
 * Push data into coroutine internal storage then yield until it consumes the
 * data through ::nce_coro_wait or ::nce_coro_pull.
 *
 * This function does not resume the coroutine passed as argument.
 *
 * \pre data != NULL
 * \param into the target coroutine
 * \param data data to push
 * \param size size of data to push
 */
void
nce_coro_push(struct nce_coro *into, const void *data, size_t size);

/**
 * This function is analogous to ::nce_coro_push.
 *
 * It will first check if data is available and return immediately.
 *
 * Otherwise, it will yield until the data becomes available.
 *
 * Because this function yields forever until the data is available, caller is
 * responsible of resuming both coroutine waiting and corouting producing in the
 * event loop.
 *
 * When coroutines are attached to the event loop and have proper priorities it
 * usually transparent.
 *
 * \pre data != NULL
 * \param from the coroutine to pull data from
 * \param data location where to pull data
 * \param size size of data to pull
 */
void
nce_coro_pull(struct nce_coro *from, void *data, size_t size);

/**
 * Return the current coroutine.
 *
 * \return the current running coroutine
 */
struct nce_coro *
nce_coro_self(void);

/**
 * Resume the coroutine until it ends by itself.
 *
 * This function should be used with care as the coroutine will be resumed until
 * it terminates on its own which means that it must be aware that the caller
 * is trying to destroy it.
 *
 * The coroutine is also destroyed once it has terminated, thus
 * ::nce_coro_destroy is not necessary.
 *
 * If the coroutine is already terminated, the function is no-op.
 *
 * It is mostly equivalent to:
 *
 * ```c
 * while (nce_coro_resumable(coro))
 *     nce_coro_resume(coro);
 *
 * nce_coro_destroy(coro);
 * ```
 */
void
nce_coro_join(struct nce_coro *coro);

/**
 * Destroy the coroutine.
 *
 * \pre the coroutine **must** not be running.
 */
void
nce_coro_destroy(struct nce_coro *coro);

#ifdef __cplusplus
}
#endif

/* Only available implementation now. */
#include "coro-minicoro.h"

#endif /* !LIBNCE_CORO_H */
