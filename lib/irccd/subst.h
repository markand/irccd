/*
 * subst.h -- pattern substitution
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

#ifndef IRCCD_SUBST_H
#define IRCCD_SUBST_H

#include <sys/types.h>
#include <stddef.h>
#include <time.h>

enum irc_subst_flags {
	IRC_SUBST_DATE          = (1 << 0),
	IRC_SUBST_KEYWORDS      = (1 << 1),
	IRC_SUBST_ENV           = (1 << 2),
	IRC_SUBST_SHELL         = (1 << 3),
	IRC_SUBST_IRC_ATTRS     = (1 << 4),
	IRC_SUBST_SHELL_ATTRS   = (1 << 5)
};

struct irc_subst_keyword {
	const char *key;
	const char *value;
};

struct irc_subst {
	time_t time;
	enum irc_subst_flags flags;
	const struct irc_subst_keyword *keywords;
	size_t keywordsz;
};

ssize_t
irc_subst(char *, size_t, const char *, const struct irc_subst *);

#endif /* !IRCCD_SUBST_H */
