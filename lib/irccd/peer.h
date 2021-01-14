/*
 * peer.h -- client connected to irccd
 *
 * Copyright (c) 2013-2021 David Demelier <markand@malikania.fr>
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

#include <stdbool.h>

#include "limits.h"

struct pollfd;

struct irc_peer {
	int fd;
	char in[IRC_BUF_MAX];
	char out[IRC_BUF_MAX];
};

bool
irc_peer_send(struct irc_peer *, const char *, ...);

void
irc_peer_prepare(struct irc_peer *, struct pollfd *);

bool
irc_peer_flush(struct irc_peer *, const struct pollfd *);

void
irc_peer_finish(struct irc_peer *);

#endif /* !IRCCD_PEER_H */
