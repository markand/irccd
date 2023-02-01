/*
 * test-jsapi-chrono.c -- test Irccd.Chrono API
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

#include <unistd.h>

#define GREATEST_USE_ABBREVS 0
#include <greatest.h>

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
basics_simple(void)
{
	if (duk_peval_string(ctx, "timer = new Irccd.Chrono();") != 0)
		GREATEST_FAIL();

	sleep(1);

	if (duk_peval_string(ctx, "result = timer.elapsed;") != 0)
		GREATEST_FAIL();

	duk_get_global_string(ctx, "result");

	GREATEST_ASSERT_IN_RANGE(1000U, duk_get_uint(ctx, -1), 100);
	GREATEST_PASS();
}

GREATEST_TEST
basics_reset(void)
{
	/*
	 * Create a timer and wait for it to accumulate some time. Then use
	 * start to reset its value and wait for 1s. The elapsed time must not
	 * be greater than 1s.
	 */
	if (duk_peval_string(ctx, "timer = new Irccd.Chrono()") != 0)
		GREATEST_FAIL();

	sleep(1);

	if (duk_peval_string(ctx, "timer.reset();") != 0)
		GREATEST_FAIL();

	sleep(1);

	if (duk_peval_string(ctx, "result = timer.elapsed") != 0)
		GREATEST_FAIL();

	duk_get_global_string(ctx, "result");

	GREATEST_ASSERT_IN_RANGE(1000U, duk_get_uint(ctx, -1), 100);
	GREATEST_PASS();
}

GREATEST_SUITE(suite_basics)
{
	GREATEST_SET_SETUP_CB(setup, NULL);
	GREATEST_SET_TEARDOWN_CB(teardown, NULL);
	GREATEST_RUN_TEST(basics_simple);
	GREATEST_RUN_TEST(basics_reset);
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
