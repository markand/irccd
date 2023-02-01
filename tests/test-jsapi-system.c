/*
 * test-jsapi-system.c -- test Irccd.System API
 *
 * Copyright (c) 2013-2023 David Demelier <markand@malikania.fr>
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

#define GREATEST_USE_ABBREVS 0
#include <greatest.h>

#include <irccd/config.h>
#include <irccd/js-plugin.h>
#include <irccd/plugin.h>

static struct irc_plugin *plugin;
static duk_context *ctx;

static void
setup(void *udata)
{
	(void)udata;

	plugin = js_plugin_open("example", TOP "/tests/data/example-plugin.js");
	ctx = js_plugin_get_context(plugin);
}

static void
teardown(void *udata)
{
	(void)udata;

	irc_plugin_finish(plugin);

	plugin = NULL;
	ctx = NULL;
}

GREATEST_TEST
basics_popen(void)
{
	int ret = duk_peval_string(ctx,
		"f = Irccd.System.popen(\"" IRCCD_EXECUTABLE " version\", \"r\");"
		"r = f.readline();"
	);

	if (ret != 0)
		GREATEST_FAIL();

	GREATEST_ASSERT(duk_get_global_string(ctx, "r"));
	GREATEST_ASSERT_STR_EQ(IRCCD_VERSION, duk_get_string(ctx, -1));

	GREATEST_PASS();
}

GREATEST_TEST
basics_sleep(void)
{
	time_t start, now;

	start = time(NULL);

	if (duk_peval_string(ctx, "Irccd.System.sleep(2)") != 0)
		GREATEST_FAIL();

	now = time(NULL);

	GREATEST_ASSERT_IN_RANGE(2000LL, difftime(now, start) * 1000LL, 100LL);

	GREATEST_PASS();
}

GREATEST_TEST
basics_usleep(void)
{
	time_t start, now;

	start = time(NULL);

	if (duk_peval_string(ctx, "Irccd.System.usleep(2000000)") != 0)
		GREATEST_FAIL();

	now = time(NULL);

	GREATEST_ASSERT_IN_RANGE(2000LL, difftime(now, start) * 1000LL, 100LL);

	GREATEST_PASS();
}

GREATEST_SUITE(suite_basics)
{
	GREATEST_SET_SETUP_CB(setup, NULL);
	GREATEST_SET_TEARDOWN_CB(teardown, NULL);
	GREATEST_RUN_TEST(basics_popen);
	GREATEST_RUN_TEST(basics_sleep);
	GREATEST_RUN_TEST(basics_usleep);
}

GREATEST_MAIN_DEFS();

int
main(int argc, char **argv)
{
	GREATEST_MAIN_BEGIN();
	GREATEST_RUN_SUITE(suite_basics);
	GREATEST_MAIN_END();

	return 0;
}
