/*
 * peer.h -- client connected to irccd
 *
 * Copyright (c) 2013-2025 David Demelier <markand@malikania.fr>
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

#ifndef IRCCD_PEER_H
#define IRCCD_PEER_H

#include <stddef.h>

#include <ev.h>

#include <irccd/attrs.h>
#include <irccd/limits.h>

struct peer;

typedef void (*peer_close_cb)(struct peer *);

struct peer {
	int fd;
	struct ev_io io_fd;

	int is_watching;

	peer_close_cb on_close;

	char in[IRC_BUF_LEN];
	size_t insz;
	char out[IRC_BUF_LEN];
	size_t outsz;

	struct peer *next;
};

struct peer *
peer_new(int);

IRCCD_ATTR_PRINTF(2, 3)
int
peer_push(struct peer *, const char *, ...);

void
peer_free(struct peer *);

#endif /* !IRCCD_PEER_H */
