/*
 * nce.h -- nano coroutine events
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

#ifndef LIBNCE_H
#define LIBNCE_H

/**
 * \file nce/nce.h
 * \brief Nano Coroutine Events
 */

#include <stddef.h>

#ifndef NCE_NO_ERRNO
#       include <errno.h>
#endif

#include <ev.h>

/**
 * Major version.
 */
#define NCE_VERSION_MAJOR 0

/**
 * Minor version.
 */
#define NCE_VERSION_MINOR 5

/**
 * Patch version.
 */
#define NCE_VERSION_PATCH 0

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
 *     struct driver *driver = NCE_CONTAINER_OF(self, struct driver, coro);
 * }
 * ```
 */
#if defined(DOXYGEN)
#define NCE_CONTAINER_OF(Ptr, Type, Member)
#else
#define NCE_CONTAINER_OF(Ptr, Type, Member) \
	((Type *)((char *)(1 ? (Ptr) : &((Type *)0)->Member) - offsetof(Type, Member)))
#endif

/**
 * Default coroutine name.
 */
#ifndef NCE_CORO_DEFAULT_NAME
#define NCE_CORO_DEFAULT_NAME "coroutine"
#endif

#if defined(DOXYGEN)

/**
 * Convenient ::NCE_CONTAINER_OF macro for ::nce_coro structure.
 */
#define NCE_CORO(Ptr, Field)

/**
 * Convenient ::NCE_CONTAINER_OF macro for ::nce_sched structure.
 */
#define NCE_SCHED(Ptr, Field)

/**
 * Define this macro to turn on debugging messages on stderr.
 */
#define NCE_DEBUG

/**
 * Define this macro to avoid use of errno.h nor errno constants.
 *
 * If set, any function that would fail will return internal negative errors.
 */
#define NCE_NO_ERRNO

/**
 * Define this macro if you want to implement your own scheduler.
 *
 * You will need to declare your own ::nce_sched structure and all of the
 * associated functions (e.g. ::nce_sched_run)
 */
#define NCE_NO_SCHED

/**
 * Define this macro if you want to disable the storage API.
 *
 * \sa ::nce_coro_dequeue
 * \sa ::nce_coro_pull
 * \sa ::nce_coro_push
 * \sa ::nce_coro_queue
 * \sa ::nce_coro_return
 * \sa ::nce_coro_wait
 */
#define NCE_NO_STORAGE

#define EV_P_ struct ev_loop *,

#else

#define NCE_CORO(Ptr, Field) \
        (NCE_CONTAINER_OF(Ptr, struct nce_coro, Field))

#define NCE_SCHED(Ptr, Field) \
        (NCE_CONTAINER_OF(Ptr, struct nce_sched, Field))

#endif /* !DOXYGEN */

struct mco_coro;

struct nce_coro;
struct nce_sched;

/**
 * Function pointer for ::nce_coro.
 */
typedef void (*nce_coro_fn_t)(EV_P_ struct nce_coro *);

/* {{{ nce_flags */

/**
 * \enum nce_flags
 * \brief Coroutine flags
 */
enum nce_flags {
	/**
	 * When this flag is set and a coroutine entrypoint exits the main loop
	 * is stopped.
	 */
	NCE_ESSENTIAL = (1 << 0),

	/**
	 * This flag is used with a watcher coroutine so that spawning the
	 * coroutine does not automatically start the underlying watcher.
	 */
	NCE_INACTIVE = (1 << 1),

	/**
	 * When a coroutine is attached to a scheduler and terminates, the
	 * coroutine is detached as usual but ::nce_coro_destroy won't be called
	 * on it.
	 */
	NCE_IMMORTAL = (1 << 2)
};

/* }}} */

/* {{{ nce_sched */

#ifndef NCE_NO_SCHED

/**
 * \struct nce_sched
 * \brief Default naive scheduler.
 *
 * This scheduler implements a double linked-list of coroutines which resumes
 * them before and after libev made an iteration.
 */
struct nce_sched {
	/**
	 * (read-only)
	 *
	 * Doubly linked list of coroutines.
	 */
	struct nce_coro *coroutines;

	struct ev_prepare prepare;  /* pre iteration resumer */
	struct ev_check check;      /* post iteration resumer */

#ifndef NCE_NO_STORAGE
	struct ev_idle persist;     /* high-priority coroutines */
	size_t persisting;          /* number of high priority coroutines */
#endif
#if EV_MULTIPLICITY
	struct ev_loop *loop;       /* associated event loop */
#endif
};

#endif /* !NCE_NO_SCHED */

/* }}} */

/* {{{ nce_coro */

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
	 * Optional coroutine or watcher flags.
	 */
	enum nce_flags flags;

	/**
	 * (optional)
	 *
	 * Change coroutine priority order.
	 *
	 * This priority is used to order the coroutine in the scheduler and
	 * must not be confused with `libev` watchers priority.
	 */
	int priority;

	/**
	 * (optional)
	 *
	 * Scheduler associated with the coroutine.
	 *
	 * If NULL and global ::nce_sched_default is not NULL, it will be used
	 * when creating the coroutine.
	 */
	struct nce_sched *sched;

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
	 *
	 * This function will be entered the first time the coroutine is
	 * resumed (through the first ::nce_coro_resume or ::nce_coro_spawn).
	 */
	nce_coro_fn_t entry;

	/**
	 * (optional)
	 *
	 * Termination function called when the coroutine is about to be
	 * removed from its scheduler.
	 *
	 * Within this function the coroutine can still be resumed by the user
	 * as normally. Use it to perform additional steps when the coroutine
	 * needs to be terminated.
	 *
	 * \warning The coroutine **MUST** not be free'd and ::nce_coro_destroy
	 *          **MUST** not be called.
	 */
	nce_coro_fn_t terminate;

	/**
	 * (optional)
	 *
	 * This function can be used to release memory for the coroutine.
	 *
	 * It is invoked with ::nce_coro_destroy once the coroutine has been internally
	 * disposed so user can free the pointer itself if needed.
	 */
	nce_coro_fn_t finalizer;

	/**
	 * \cond DOXYGEN_PRIVATE
	 */
	struct mco_coro *mco_coro;

	/* non-zero if coroutine is in nce_coro_off() */
	unsigned int off : 1;

	/* non-zero if coroutine is in nce_coro_push() */
	unsigned int persisting : 1;

#ifndef NCE_NO_SCHED
	struct nce_coro *next;
	struct nce_coro *prev;
#endif

#if EV_MULTIPLICITY
	struct ev_loop *loop;
#endif
	/**
	 * \endcond DOXYGEN_PRIVATE
	 */
};

/* }}} */

#ifdef __cplusplus
extern "C" {
#endif

/* {{{ nce_sched */

/**
 * \name Scheduler
 *
 * Module for scheduling coroutines.
 *
 * \{
 */

/**
 * Default scheduler to use.
 *
 * To attach coroutines by default without having to carry the scheduler through
 * all functions call, set this variable to the scheduler and creating
 * coroutines with a NULL ::nce_coro::sched will use this one instead.
 *
 * Calling ::nce_sched_default_init will initialize it to a local one.
 */
extern struct nce_sched *nce_sched_default;

#if !defined(NCE_NO_SCHED) || defined(DOXYGEN)

/**
 * Initialize the default built-in naive scheduler.
 *
 * This function will replace ::nce_sched_default.
 */
void
nce_sched_default_init(EV_P);

#endif

/**
 * This function attaches or detaches the coroutine into the scheduler.
 *
 * \param mode non-zero to attach the coroutine, detach otherwise
 */
void
nce_sched_attach(struct nce_sched *sched, struct nce_coro *coro, int mode);

#ifndef NCE_NO_STORAGE

/**
 * This function makes a coroutine persistent, meaning that it will be
 * constantly resumed even if the event loop has no events.
 *
 * This is the case with ::nce_coro_push which needs to be resumed until
 * coroutines that consume the data actually did.
 *
 * \param mode non-zero to to make coroutine persistent
 */
void
nce_sched_persist(struct nce_sched *sched, struct nce_coro *coro, int mode);

#endif /* !NCE_NO_STORAGE */

/**
 * Run the scheduler until the event loop is stopped.
 *
 * This function should be used instead of `ev_run` as it does all the necessary
 * steps to attach and schedule coroutines.
 *
 * When this function returns, each coroutine attached to it is destroyed.
 *
 * \param flags is identical to argument in `ev_run`
 */
void
nce_sched_run(struct nce_sched *sched, int flags);

/**
 * Stop asynchronously the scheduler.
 *
 * \param how is identical to argument in `ev_break`
 */
void
nce_sched_break(struct nce_sched *sched, int how);

/**
 * \}
 */

/* }}} */

/* {{{ nce_coro */

/**
 * \name Creation and destruction
 *
 * Create and destroy coroutines.
 *
 * \{
 */

/**
 * Create the underlying coroutine.
 *
 * This function only creates the coroutine object, thus it is not started yet.
 *
 * \return 0 on success
 * \return -ENOMEM insufficent memory
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

/**
 * \}
 */

/**
 * \name Context switching
 *
 * Functions used for resuming and yielding, transferring control between
 * coroutines and/or host program.
 *
 * \{
 */

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
 * \}
 */

#if !defined(NCE_NO_STORAGE) || defined(DOXYGEN)

/**
 * \name Data synchronization
 *
 * Functions used for exchanging data between coroutines.
 *
 * \{
 */

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
 * This function pushes data into the internal coroutine storage `into`.
 *
 * In contrast to ::nce_coro_push, this function does not yield and can be used
 * even if the internal storage has already data. However, it only inserts the
 * provided data as an entire object.
 *
 * \pre data != NULL
 * \param data the data to append
 * \param size size of data to push
 * \return 0 if object was appended
 * \return -ENOBUFS if there is not enough internal storage space
 */
int
nce_coro_queue(struct nce_coro *into, const void *data, size_t size);

/**
 * Analogous function to ::nce_coro_queue.
 *
 * This function extract the `size` bytes from the internal coroutine storage
 * pointing to `from`.
 *
 * It does not yield nor resume the coroutine at all.
 *
 * \pre data != NULL
 * \param from the coroutine to pull data from
 * \param data location where to pull data
 * \param size size of data to pull
 * \return 0 if object was extracted
 * \return -ENOMSG if no data of that size was available
 */
int
nce_coro_dequeue(struct nce_coro *from, void *data, size_t size);

/**
 * Clear internal coroutine storage.
 *
 * This function does not resume the target coroutine, it does not yield either.
 *
 * \param target the coroutine to clear
 */
void
nce_coro_clear(struct nce_coro *target);

/**
 * \}
 */

#endif /* !NCE_NO_STORAGE */

/**
 * \name Inspection
 *
 * Read only function for coroutine status.
 *
 * \{
 */

/**
 * Indicate if the coroutine is resumable (aka not started or suspended).
 *
 * \return non-zero if coroutine is resumable
 */
int
nce_coro_resumable(const struct nce_coro *coro);

/**
 * Return the current coroutine.
 *
 * \return the current running coroutine
 */
struct nce_coro *
nce_coro_self(void);

/**
 * \}
 */

/* }}} */

#ifdef __cplusplus
}
#endif

#endif /* !LIBNCE_H */
