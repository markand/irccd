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
#include <string.h>
#include <strings.h>

#include "rule.h"
#include "util.h"

static void
lower(char *dst, const char *src)
{
	while (*src)
		*dst++ = tolower(*src++);
}

static inline char *
find(const char *str, const char *value)
{
	char strlower[IRC_RULE_MAX] = {0};
	char valuelower[IRC_RULE_MAX] = {0};
	char *p;

	lower(strlower, str);
	lower(valuelower, value);

	if ((p = strstr(strlower, valuelower)))
		return (char *)&str[p - strlower];

	return NULL;
}

static inline bool
match(const char *str, const char *value)
{
	size_t len;
	const char *p;

	if (!str[0])
		return true;
	if (!value || (len = strlen(value)) == 0 || !(p = find(str, value)))
		return false;

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

bool
irc_rule_add(char *str, const char *value)
{
	size_t slen, vlen;

	if (find(str, value))
		return true;

	slen = strlen(str);
	vlen = strlen(value);

	if (vlen + 1 >= IRC_RULE_MAX - slen) {
		errno = ENOMEM;
		return false;
	}

	sprintf(&str[slen], "%s:", value);

	return true;
}

void
irc_rule_remove(char *str, const char *value)
{
	char *pos;
	size_t vlen, slen;

	if (!(pos = find(str, value)))
		return;

	slen = strlen(str);
	vlen = strlen(value) + 1;       /* includes ':' */

	assert(pos[vlen - 1] == ':');
	memmove(&pos[0], &pos[vlen], IRC_RULE_MAX - (&pos[vlen] - str));
}

bool
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

bool
irc_rule_matchlist(const struct irc_rule *rules,
                   size_t rulesz,
                   const char *server,
                   const char *channel,
                   const char *origin,
                   const char *plugin,
                   const char *event)
{
	bool result = true;

	for (size_t i = 0; i < rulesz; ++i)
		if (irc_rule_match(&rules[i], server, channel, origin, plugin, event))
			result = rules[i].action == IRC_RULE_ACCEPT;

	return result;
}

void
irc_rule_finish(struct irc_rule *rule)
{
	assert(rule);

	memset(rule, 0, sizeof (*rule));
}
