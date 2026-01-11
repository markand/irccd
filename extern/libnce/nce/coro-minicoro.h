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
