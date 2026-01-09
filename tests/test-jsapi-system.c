/*
 * test-jsapi-system.c -- test Irccd.System API
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

#include <unity.h>

#include <irccd/config.h>
#include <irccd/js-plugin.h>
#include <irccd/plugin.h>

static struct irc_plugin *plugin;
static duk_context *ctx;

void
setUp(void)
{
	plugin = js_plugin_open("example", TOP "/tests/data/example-plugin.js");
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
basics_popen(void)
{
	int ret = duk_peval_string(ctx,
		"f = Irccd.System.popen(\"" IRCCD_EXECUTABLE " version\", \"r\");"
		"r = f.readline();"
	);

	if (ret != 0)
		TEST_FAIL();

	TEST_ASSERT(duk_get_global_string(ctx, "r"));
	TEST_ASSERT_EQUAL_STRING(IRCCD_VERSION, duk_get_string(ctx, -1));
}

static void
basics_sleep(void)
{
	time_t start, now;

	start = time(NULL);

	if (duk_peval_string(ctx, "Irccd.System.sleep(2)") != 0)
		TEST_FAIL();

	now = time(NULL);

	TEST_ASSERT(difftime(now, start) >= 2);
}

static void
basics_usleep(void)
{
	time_t start, now;

	start = time(NULL);

	if (duk_peval_string(ctx, "Irccd.System.usleep(2000000)") != 0)
		TEST_FAIL();

	now = time(NULL);

	TEST_ASSERT(difftime(now, start) >= 2);
}

int
main(void)
{
	UNITY_BEGIN();

	RUN_TEST(basics_popen);
	RUN_TEST(basics_sleep);
	RUN_TEST(basics_usleep);

	return UNITY_END();
}
