/*
 * rule.h -- rule filtering
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

#ifndef IRCCD_RULE_H
#define IRCCD_RULE_H

#include <stdbool.h>
#include <stddef.h>

#define IRC_RULE_MAX 1024

enum irc_rule_action {
	IRC_RULE_ACCEPT,
	IRC_RULE_DROP
};

struct irc_rule {
	enum irc_rule_action action;
	char servers[IRC_RULE_MAX];
	char channels[IRC_RULE_MAX];
	char origins[IRC_RULE_MAX];
	char plugins[IRC_RULE_MAX];
	char events[IRC_RULE_MAX];
};

bool
irc_rule_add(char *, const char *);

void
irc_rule_remove(char *, const char *);

bool
irc_rule_match(const struct irc_rule *,
               const char *,
               const char *,
               const char *,
               const char *,
               const char *);

bool
irc_rule_matchlist(const struct irc_rule *,
                   size_t,
                   const char *,
                   const char *,
                   const char *,
                   const char *,
                   const char *);

void
irc_rule_finish(struct irc_rule *);

#endif /* !IRCCD_RULE_H */
