/*
 * jsapi-rule.c -- Irccd.Rule API
 *
 * Copyright (c) 2013-2022 David Demelier <markand@malikania.fr>
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
#include <string.h>

#include <utlist.h>

#include <irccd/irccd.h>
#include <irccd/rule.h>
#include <irccd/util.h>

#include "jsapi-rule.h"

static void
push_list(duk_context *ctx, const char *value, const char *prop)
{
	char tmp[IRC_RULE_LEN], *token, *p;
	size_t i = 0;

	irc_util_strlcpy(tmp, value, sizeof (tmp));
	duk_push_array(ctx);

	for (p = tmp; (token = strtok_r(p, ":", &p)); ) {
		duk_push_string(ctx, token);
		duk_put_prop_index(ctx, -2, i++);
	}

	duk_put_prop_string(ctx, -2, prop);
}

static void
get_list(duk_context *ctx, char *dst, const char *prop)
{
	duk_get_prop_string(ctx, 1, prop);

	if (!duk_is_object(ctx, -1)) {
		duk_pop(ctx);
		return;
	}

	duk_enum(ctx, -1, DUK_ENUM_ARRAY_INDICES_ONLY);

	while (duk_next(ctx, -1, 1)) {
		if (duk_is_string(ctx, -1))
			irc_rule_add(dst, duk_to_string(ctx, -1));

		duk_pop_n(ctx, 2);
	}

	duk_pop(ctx);
}

static int
Rule_add(duk_context *ctx)
{
	struct irc_rule *rule;
	size_t index = duk_opt_uint(ctx, 0, -1);
	int action;

	duk_require_object(ctx, 1);
	duk_get_prop_string(ctx, 1, "action");

	if (!duk_is_number(ctx, -1) || (action = duk_to_int(ctx, -1)) < 0 || action > IRC_RULE_DROP)
		return duk_error(ctx, DUK_ERR_TYPE_ERROR, "invalid rule action");

	rule = irc_rule_new(action);

	get_list(ctx, rule->servers, "servers");
	get_list(ctx, rule->channels, "channels");
	get_list(ctx, rule->origins, "origins");
	get_list(ctx, rule->plugins, "plugins");
	get_list(ctx, rule->events, "events");

	irc_bot_rule_insert(rule, index);

	return 0;
}

static int
Rule_clear(duk_context *ctx)
{
	(void)ctx;

	irc_bot_rule_clear();

	return 0;
}

static int
Rule_list(duk_context *ctx)
{
	struct irc_rule *rule;
	size_t i = 0;

	duk_push_array(ctx);

	DL_FOREACH(irc.rules, rule) {
		duk_push_object(ctx);
		duk_push_int(ctx, rule->action);
		duk_put_prop_string(ctx, -2, "action");
		push_list(ctx, rule->servers, "servers");
		push_list(ctx, rule->channels, "channels");
		push_list(ctx, rule->origins, "origins");
		push_list(ctx, rule->plugins, "plugins");
		push_list(ctx, rule->events, "events");
		duk_put_prop_index(ctx, -2, i++);
	}

	return 1;
}

static int
Rule_remove(duk_context *ctx)
{
	size_t index = duk_require_uint(ctx, 0);

	if (index >= irc_bot_rule_size())
		return duk_error(ctx, DUK_ERR_RANGE_ERROR, "rule index is invalid");

	irc_bot_rule_remove(index);

	return 0;
}

static const duk_number_list_entry actions[] = {
	{ "Accept",     IRC_RULE_ACCEPT         },
	{ "Drop",       IRC_RULE_DROP           },
	{ NULL,         -1                      }
};

static const duk_function_list_entry functions[] = {
	{ "add",        Rule_add,       2       },
	{ "clear",      Rule_clear,     0       },
	{ "list",       Rule_list,      0       },
	{ "remove",     Rule_remove,    1       },
	{ NULL,         NULL,           0       }
};

void
jsapi_rule_load(duk_context *ctx)
{
	assert(ctx);

	duk_get_global_string(ctx, "Irccd");
	duk_push_object(ctx);
	duk_put_number_list(ctx, -1, actions);
	duk_put_function_list(ctx, -1, functions);
	duk_put_prop_string(ctx, -2, "Rule");
	duk_pop(ctx);
}
