/*
 * rule.c -- rule filtering
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

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include <utlist.h>

#include "rule.h"
#include "util.h"

static inline void
lower(char *dst, const char *src)
{
	while (*src)
		*dst++ = tolower(*src++);
}

static char *
find(const char *str, const char *value)
{
	char strlower[IRC_RULE_LEN] = {0};
	char valuelower[IRC_RULE_LEN] = {0};
	char *p;

	lower(strlower, str);
	lower(valuelower, value);

	if ((p = strstr(strlower, valuelower)))
		return (char *)&str[p - strlower];

	return NULL;
}

static int
match(const char *str, const char *value)
{
	size_t len;
	const char *p;

	if (!str[0])
		return 1;
	if (!value || (len = strlen(value)) == 0 || !(p = find(str, value)))
		return 0;

	/*
	 * Consider the following scenario:
	 *
	 * value = no
	 * str   = mlk:freenode:
	 * p     =         ^
	 */
	while (p != str && *p != ':')
		--p;
	if (*p == ':')
		++p;

	return strncasecmp(p, value, len) == 0;
}

struct irc_rule *
irc_rule_new(enum irc_rule_action action)
{
	struct irc_rule *r;

	r = irc_util_calloc(1, sizeof (*r));
	r->action = action;

	return r;
}

int
irc_rule_add(char *str, const char *value)
{
	size_t slen, vlen;

	if (find(str, value))
		return 0;

	slen = strlen(str);
	vlen = strlen(value);

	if (vlen + 1 >= IRC_RULE_LEN - slen) {
		errno = ENOMEM;
		return -1;
	}

	sprintf(&str[slen], "%s:", value);

	return 0;
}

void
irc_rule_remove(char *str, const char *value)
{
	char *pos;
	size_t vlen;

	if (!(pos = find(str, value)))
		return;

	vlen = strlen(value) + 1;       /* includes ':' */

	assert(pos[vlen - 1] == ':');
	memmove(&pos[0], &pos[vlen], IRC_RULE_LEN - (&pos[vlen] - str));
}

int
irc_rule_match(const struct irc_rule *rule,
               const char *server,
               const char *channel,
               const char *origin,
               const char *plugin,
               const char *event)
{
	return match(rule->servers, server)     &&
	       match(rule->channels, channel)   &&
	       match(rule->origins, origin)     &&
	       match(rule->plugins, plugin)     &&
	       match(rule->events, event);
}

int
irc_rule_matchlist(const struct irc_rule *rules,
                   const char *server,
                   const char *channel,
                   const char *origin,
                   const char *plugin,
                   const char *event)
{
	int result = 1;
	const struct irc_rule *r;

	DL_FOREACH(rules, r)
		if (irc_rule_match(r, server, channel, origin, plugin, event))
			result = r->action == IRC_RULE_ACCEPT;

	return result;
}

void
irc_rule_finish(struct irc_rule *rule)
{
	assert(rule);

	free(rule);
}
