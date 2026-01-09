/*
 * test-jsapi-timer.c -- test Irccd.System API
 *
 * Copyright (c) 2013-2026 David Demelier <markand@malikania.fr>
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

#include <ev.h>

#include <unity.h>

#include <irccd/irccd.h>
#include <irccd/js-plugin.h>
#include <irccd/plugin.h>

static struct irc_plugin *plugin;
static duk_context *ctx;

void
setUp(void)
{
	plugin = js_plugin_open("timer", TOP "/tests/data/timer.js");
	ctx = js_plugin_get_context(plugin);
}

void
tearDown(void)
{
	irc_plugin_finish(plugin);

	plugin = NULL;
	ctx = NULL;
}

static void
set_type(const char *name)
{
	duk_get_global_string(ctx, "Irccd");
	duk_get_prop_string(ctx, -1, "Timer");
	duk_get_prop_string(ctx, -1, name);
	duk_put_global_string(ctx, "type");
	duk_pop_n(ctx, 2);

	irc_plugin_load(plugin);
}

static void
basics_single(void)
{
	time_t start = time(NULL);

	set_type("Single");

	while (difftime(time(NULL), start) < 3)
		ev_run(EVRUN_ONCE);

	TEST_ASSERT(duk_get_global_string(ctx, "count"));
	TEST_ASSERT_EQUAL_INT(duk_get_int(ctx, -1), 1);
}

static void
basics_repeat(void)
{
	time_t start = time(NULL);

	set_type("Repeat");

	while (difftime(time(NULL), start) < 3)
		ev_run(EVRUN_ONCE);

	TEST_ASSERT(duk_get_global_string(ctx, "count"));
	TEST_ASSERT(duk_get_int(ctx, -1) >= 5);
}

int
main(void)
{
	ev_default_loop(0);

	UNITY_BEGIN();

	RUN_TEST(basics_single);
	RUN_TEST(basics_repeat);

	return UNITY_END();
}
