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

#ifndef LIBCORO_CORO_H
#define LIBCORO_CORO_H

/**
 * \file coro/coro.h
 * \brief Coroutine library.
 * \ingroup libcoro-ev
 */

#include <stddef.h>

#include <ev.h>

/**
 * Default coroutine name.
 */
#ifndef CORO_DEFAULT_NAME
#define CORO_DEFAULT_NAME "coroutine"
#endif

/**
 * Minimum priority.
 */
#define CORO_PRI_MIN EV_MINPRI

/**
 * Maximum priority.
 */
#define CORO_PRI_MAX EV_MAXPRI

/**
 * Convenient ::CORO_CONTAINER_OF macro for ::coro structure.
 */
#if defined(DOXYGEN)
#define CORO(Ptr, Field)
#else
#define CORO(Ptr, Field) \
	(CORO_CONTAINER_OF(Ptr, struct coro, Field))
#endif

/**
 * Macro to retrieve original structure pointer from a inner field.
 *
 * Example:
 *
 * ```c
 * struct driver {
 *     int foo;
 *     int bar;
 *     struct coro coro;
 * };
 *
 * static void
 * driver_entry(EV_P_ struct coro *self)
 * {
 *     struct driver *driver = CORO_CONTAINER_OF(self, struct driver, coro);
 * }
 * ```
 */
#if defined(DOXYGEN)
#define CORO_CONTAINER_OF(Ptr, Type, Member)
#else
#define CORO_CONTAINER_OF(Ptr, Type, Member) \
	((Type *)((char *)(1 ? (Ptr) : &((Type *)0)->Member) - offsetof(Type, Member)))
#endif

struct coro;
struct coro_def;

/**
 * \brief Coroutine entrypoint.
 *
 * This function is executed when the coroutine is resumed for the first time.
 */
typedef void (*coro_entry_t)(EV_P_ struct coro *);

/**
 * \brief Finalizer function.
 *
 * This optional function can be set to perform an extra user finalization step
 * when an event loop destroy the coroutine.
 *
 * It is invoked with ::coro_finish.
 */
typedef void (*coro_finalizer_t)(EV_P_ struct coro *);

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
 * When removed from the loop, ::coro_finish is called. Use ::coro_set_finalizer
 * to add an optional cleanup function.
 */
#define CORO_ATTACHED (1 << 0)

/**
 * If this flag is set and if the coroutine that is resumed from the
 * event loop is no longer resumable the event loop is stopped.
 *
 * This flag implies ::CORO_ATTACHED.
 */
#define CORO_ESSENTIAL (1 << 1)

/**
 * This flag can be set as a hint so that the coroutine to be resumed
 * from the event loop is guaranteed to never terminate improving performances
 * as no check has to be made.
 *
 * When built unoptimized a check is still added and aborts if the case
 * still happen.
 *
 * This flag is mutually exclusive with ::CORO_ESSENTIAL.
 *
 * This flag implies ::CORO_ATTACHED.
 */
#define CORO_FOREVER (1 << 2)

/**
 * This flag is used when creating a coroutine associated with a watcher to
 * start the coroutine but with the watcher initially stopped.
 *
 * It is only meaningful with watcher coroutines spawner (e.g.
 * ::ctimer_coro_spawn) and never used with ::coro_spawn itself.
 */
#define CORO_INACTIVE (1 << 3)

/**
 * \struct coro_def
 * \brief Coroutine description for ::coro_spawn.
 */
struct coro_def {
	/**
	 * Coroutine name.
	 *
	 * Mostly used for debugging purposes.
	 */
	const char *name;

	/**
	 * Change coroutine priority order.
	 *
	 * Only meaningful with attached coroutines.
	 *
	 * \sa ::CORO_ATTACHED
	 * \sa ::CORO_ESSENTIAL
	 * \sa ::CORO_FOREVER
	 */
	int priority;

	/**
	 * Optional coroutines flags.
	 */
	unsigned int flags;

	/**
	 * Optional stack size.
	 *
	 * A value of 0 will use a library default.
	 */
	size_t stack_size;

	/**
	 * Coroutine entrypoint.
	 */
	coro_entry_t entry;

	/**
	 * (optional)
	 *
	 * Finalizer to be invoked by ::coro_finish before destroying it.
	 */
	coro_finalizer_t finalizer;
};

#if defined(DOXYGEN)
/**
 * \struct coro
 * \brief Coroutine object.
 */
struct coro {
	/**
	 * Coroutine definition.
	 */
	struct coro_def def;
};
#endif

/**
 * Initialize defaults in coroutine.
 */
void
coro_init(struct coro *coro);

/**
 * Set coroutine name.
 *
 * If name is NULL, ::CORO_DEFAULT_NAME will be used
 *
 * \param name the new coroutine name
 * \sa ::coro_def::name
 */
void
coro_set_name(struct coro *coro, const char *name);

/**
 * Set coroutine priority.
 *
 * \param priority the new priority
 * \sa ::coro_def::priority
 */
void
coro_set_priority(struct coro *coro, int priority);

/**
 * Set coroutine flags.
 *
 * \param flags the new flags
 * \sa ::coro_def::flags
 */
void
coro_set_flags(struct coro *coro, unsigned int flags);

/**
 * Set coroutine stack size.
 *
 * This function has no effect if already started.
 *
 * \param stack_size the coroutine stack size to allocate
 * \sa ::coro_def::stack_size
 */
void
coro_set_stack_size(struct coro *coro, size_t stack_size);

/**
 * Set coroutine entrypoint.
 *
 * This function can be called on an active coroutine.
 *
 * \pre entry != NULL
 * \param entry the coroutine entrypoint
 * \sa ::coro_def::entry
 */
void
coro_set_entry(struct coro *coro, coro_entry_t entry);

/**
 * Set coroutine finalizer.
 *
 * Passing NULL will remove the finalizer.
 *
 * \param finalizer the coroutine finalizer (maybe NULL)
 * \sa ::coro_def::finalizer
 */
void
coro_set_finalizer(struct coro *coro, coro_finalizer_t finalizer);

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
coro_create(EV_P_ struct coro *coro);

/**
 * This is a convenient all-in-one function that initialize the coroutine and
 * start it immediately.
 *
 * The `def` argument is copied in the coroutine and can be discarded (note that
 * the ::coro_def::name must remain valid though).
 *
 * \pre def != NULL
 * \param def the coroutine definition
 */
int
coro_spawn(EV_P_ struct coro *coro, const struct coro_def *def);

/**
 * Indicate if the coroutine is resumable (aka not started or suspended).
 *
 * \return non-zero if coroutine is resumable
 */
int
coro_resumable(const struct coro *coro);

/**
 * Start or resume the coroutine.
 *
 * If the coroutine is attached to the event loop (see ::coro::attached) then
 * the coroutine is already resumed before and after event loop iteration.
 *
 * \pre coroutine **must** be resumable
 */
void
coro_resume(struct coro *coro);

/**
 * Yield the current coroutine.
 */
void
coro_yield(void);

/**
 * Yield forever.
 */
void
coro_idle(void);

/**
 * Continuously yield until ::coro_on is called again.
 *
 * This is useful for coroutines attached to the event loop but that needs to
 * be temporarily suspended and resumed by user explicit call to ::coro_on.
 */
void
coro_off(void);

/**
 * Resume a coroutine being suspend through use of ::coro_off.
 *
 * This function will return immediately if the coroutine is already running.
 */
void
coro_on(struct coro *coro);

/**
 * This function is an alias to ::coro_push using current coroutine.
 *
 * \sa ::coro_push
 */
void
coro_return(const void *data, size_t size);

/**
 * This function is similar to ::coro_pull except that it yields the calling
 * coroutine until the data becomes available.
 *
 * \sa ::coro_pull
 */
void
coro_wait(void *data, size_t size);

/**
 * Push data into coroutine internal storage then yield until it consumes the
 * data through ::coro_wait or ::coro_pull.
 *
 * This function does not resume the coroutine passed as argument.
 *
 * \pre data != NULL
 * \param into the target coroutine
 * \param data data to push
 * \param size size of data to push
 */
void
coro_push(struct coro *into, const void *data, size_t size);

/**
 * This function is analogous to ::coro_push.
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
coro_pull(struct coro *from, void *data, size_t size);

/**
 * Return the current coroutine.
 *
 * \return the current running coroutine
 */
struct coro *
coro_self(void);

/**
 * Resume the coroutine until it ends by itself.
 *
 * This function should be used with care as the coroutine will be resumed until
 * it terminates on its own which means that it must be aware that the caller
 * is trying to destroy it.
 *
 * The coroutine is also destroyed once it has terminated, thus ::coro_finish is
 * not necessary.
 *
 * If the coroutine is already terminated, the function is no-op.
 *
 * It is mostly equivalent to:
 *
 * ```c
 * while (coro_resumable(coro))
 *     coro_resume(coro);
 *
 * coro_finish(coro);
 * ```
 */
void
coro_join(struct coro *coro);

/**
 * Destroy the coroutine.
 *
 * \pre the coroutine **must** not be running.
 */
void
coro_finish(struct coro *coro);

/* Only available implementation now. */
#include "coro-minicoro.h"

#endif /* !LIBCORO_CORO_H */
