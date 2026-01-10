/*
 * coro-minicoro.h -- coroutine library (minicoro implementation)
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

#include <minicoro.h>

struct coro {
	/**
	 * (optional)
	 *
	 * Coroutine name.
	 *
	 * Mostly used for debugging purposes.
	 */
	const char *name;

	/**
	 * (init, optional)
	 *
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
	 * (init, optional)
	 *
	 * Optional coroutines flags.
	 */
	unsigned int flags;

	/**
	 * (init, optional)
	 *
	 * Optional stack size.
	 *
	 * A value of 0 will use a library default.
	 */
	size_t stack_size;

	/**
	 * (init)
	 *
	 * Coroutine entrypoint.
	 */
	coro_entry_t entry;

	/**
	 * (optional)
	 *
	 * Finalizer to be invoked by ::coro_finish before destroying it.
	 */
	coro_finalizer_t finalizer;

	/* minicoro */
	struct mco_desc mco_desc;
	struct mco_coro *mco_coro;

	/* loop iteration hooks */
	struct ev_prepare prepare;
	struct ev_check check;

	/* non-zero if coroutine is in coro_off() */
	int off;

#if EV_MULTIPLICITY
	/* attached event loop */
	struct ev_loop *loop;
#endif
};
