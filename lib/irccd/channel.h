/*
 * channel.h -- an IRC server channel
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

#ifndef IRCCD_CHANNEL_H
#define IRCCD_CHANNEL_H

#include <sys/queue.h>
#include <stddef.h>

#include "limits.h"

struct irc_channel_user {
	char nickname[IRC_NICKNAME_LEN];
	int modes;
	LIST_ENTRY(irc_channel_user) link;
};

struct irc_channel {
	char name[IRC_CHANNEL_LEN];
	char password[IRC_PASSWORD_LEN];
	int joined;
	LIST_HEAD(, irc_channel_user) users;
	LIST_ENTRY(irc_channel) link;
};

LIST_HEAD(irc_channel_list, irc_channel);

struct irc_channel *
irc_channel_new(const char *, const char *, int);

void
irc_channel_add(struct irc_channel *, const char *, int);

struct irc_channel_user *
irc_channel_find(struct irc_channel *, const char *);

void
irc_channel_clear(struct irc_channel *);

void
irc_channel_remove(struct irc_channel *, const char *);

void
irc_channel_finish(struct irc_channel *);

#endif /* !IRCCD_CHANNEL_H */
