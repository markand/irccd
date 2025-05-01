/*
 * rule.c -- rule filtering
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

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include <utlist.h>

#include "rule.h"
#include "util.h"

static inline int
list_match(char **list, const char *value)
{
	if (!list)
		return 1;

	for (char **i = list; *i; ++i)
		if (strcasecmp(*i, value) == 0)
			return 1;

	return 0;
}

static inline int
list_contains(char **list, const char *value)
{
	if (!list)
		return 0;

	for (char **i = list; *i; ++i)
		if (strcasecmp(*i, value) == 0)
			return 1;

	return 0;
}

static inline size_t
list_count(char **list)
{
	size_t rc = 0;

	if (list) {
		for (char **i = list; *i; ++i)
			++rc;
	}

	return rc;
}

static inline char **
list_free(char **list)
{
	if (list) {
		for (char **i = list; *i; ++i)
			free(*i);

		free(list);
	}

	return NULL;
}

static char **
list_add(char **list, const char *value)
{
	size_t len;

	if (list_contains(list, value))
		return list;

	len = list_count(list);

	/*
	 * Reallocate the list plus 2 elements, one for the new value and one
	 * for the NULL sentinel.
	 */
	list          = irc_util_reallocarray(list, len + 2, sizeof (char *));
	list[len]     = irc_util_strdup(value);
	list[len + 1] = NULL;

	return list;
}

/*
 * We reallocate the whole list rather than doing memmove trickery to save space
 * even though it may happen that the system may not reallocate when shrinking
 * but assume it does.
 *
 * It will also cleanup duplicates in case user modified the list by itself...
 *
 * In any case, this function is usually not called that much so we don't need
 * performance.
 */
static char **
list_remove(char **list, const char *value)
{
	char **rc = NULL;

	if (!list)
		return NULL;

	for (char **i = list; *i; ++i) {
		if (strcasecmp(*i, value) == 0)
			continue;

		rc = list_add(rc, *i);
	}

	list_free(list);

	return rc;
}


struct irc_rule *
irc_rule_new(enum irc_rule_action action)
{
	struct irc_rule *r;

	r = irc_util_calloc(1, sizeof (*r));
	r->action = action;

	return r;
}

void
irc_rule_add_server(struct irc_rule *rule, const char *value)
{
	rule->servers = list_add(rule->servers, value);
}

void
irc_rule_remove_server(struct irc_rule *rule, const char *value)
{
	rule->servers = list_remove(rule->servers, value);
}

void
irc_rule_add_channel(struct irc_rule *rule, const char *value)
{
	rule->channels = list_add(rule->channels, value);
}

void
irc_rule_remove_channel(struct irc_rule *rule, const char *value)
{
	rule->channels = list_remove(rule->channels, value);
}

void
irc_rule_add_origin(struct irc_rule *rule, const char *value)
{
	rule->origins = list_add(rule->origins, value);
}

void
irc_rule_remove_origin(struct irc_rule *rule, const char *value)
{
	rule->origins = list_remove(rule->origins, value);
}

void
irc_rule_add_plugin(struct irc_rule *rule, const char *value)
{
	rule->plugins = list_add(rule->plugins, value);
}

void
irc_rule_remove_plugin(struct irc_rule *rule, const char *value)
{
	rule->plugins = list_remove(rule->plugins, value);
}

void
irc_rule_add_event(struct irc_rule *rule, const char *value)
{
	rule->events = list_add(rule->events, value);
}

void
irc_rule_remove_event(struct irc_rule *rule, const char *value)
{
	rule->events = list_remove(rule->events, value);
}

int
irc_rule_match(const struct irc_rule *rule,
               const char *server,
               const char *channel,
               const char *origin,
               const char *plugin,
               const char *event)
{
	return list_match(rule->servers, server)     &&
	       list_match(rule->channels, channel)   &&
	       list_match(rule->origins, origin)     &&
	       list_match(rule->plugins, plugin)     &&
	       list_match(rule->events, event);
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
irc_rule_free(struct irc_rule *rule)
{
	assert(rule);

	list_free(rule->servers);
	list_free(rule->channels);
	list_free(rule->origins);
	list_free(rule->plugins);
	list_free(rule->events);

	free(rule);
}
