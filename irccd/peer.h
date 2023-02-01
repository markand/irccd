/*
 * peer.h -- client connected to irccd
 *
 * Copyright (c) 2013-2023 David Demelier <markand@malikania.fr>
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

#include <irccd/limits.h>

struct pollfd;

struct peer {
	int fd;
	int is_watching;
	char in[IRC_BUF_LEN];
	char out[IRC_BUF_LEN];
	struct peer *next;
};

struct peer *
peer_new(int);

int
peer_send(struct peer *, const char *, ...);

void
peer_prepare(struct peer *, struct pollfd *);

int
peer_flush(struct peer *, const struct pollfd *);

void
peer_finish(struct peer *);

#endif /* !IRCCD_PEER_H */
