/*
 * hook.h -- irccd hooks
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

#ifndef IRCCD_HOOK_H
#define IRCCD_HOOK_H

#include <sys/queue.h>
#include <limits.h>

#include "limits.h"

struct irc_event;

struct irc_hook {
	char name[IRC_ID_LEN];
	char path[PATH_MAX];
	LIST_ENTRY(irc_hook) link;
};

LIST_HEAD(irc_hook_list, irc_hook);

struct irc_hook *
irc_hook_new(const char *, const char *);

void
irc_hook_invoke(struct irc_hook *, const struct irc_event *);

void
irc_hook_finish(struct irc_hook *);

#endif /* !IRCCD_HOOK_H */
