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

#if EV_PREPARE_ENABLE == 0
#error "minicoro implementation requires ev_prepare"
#endif

#if EV_CHECK_ENABLE == 0
#error "minicoro implementation requires ev_check"
#endif

struct coro {
	struct coro_def def;
	struct mco_desc mco_desc;
	struct mco_coro *mco_coro;
	struct ev_prepare prepare;
	struct ev_check check;
	int off;

#if EV_MULTIPLICITY
	/**
	 * (private)
	 *
	 * Event loop attached.
	 */
	struct ev_loop *loop;
#endif
};
