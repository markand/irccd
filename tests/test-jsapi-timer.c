/*
 * test-jsapi-timer.c -- test Irccd.System API
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

#include <poll.h>
#include <signal.h>

#define GREATEST_USE_ABBREVS 0
#include <greatest.h>

#include <irccd/irccd.h>
#include <irccd/js-plugin.h>
#include <irccd/plugin.h>

static struct irc_plugin *plugin;
static duk_context *ctx;

static void
setup(void *udata)
{
	(void)udata;

	plugin = js_plugin_open("timer", TOP "/tests/data/timer.js");
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

GREATEST_TEST
basics_single(void)
{
	time_t start = time(NULL);
	struct pollfd fd;

	set_type("Single");

	while (difftime(time(NULL), start) < 3) {
		irc_bot_prepare(&fd);
		poll(&fd, 1, 1);
		irc_bot_flush(&fd);
	}

	GREATEST_ASSERT(duk_get_global_string(ctx, "count"));
	GREATEST_ASSERT_EQ(duk_get_int(ctx, -1), 1);
	GREATEST_PASS();
}

GREATEST_TEST
basics_repeat(void)
{
	time_t start = time(NULL);
	struct pollfd fd;

	set_type("Repeat");

	while (difftime(time(NULL), start) < 3) {
		irc_bot_prepare(&fd);
		poll(&fd, 1, 1);
		irc_bot_flush(&fd);
	}

	GREATEST_ASSERT(duk_get_global_string(ctx, "count"));
	GREATEST_ASSERT(duk_get_int(ctx, -1) >= 5);
	GREATEST_PASS();
}

GREATEST_SUITE(suite_basics)
{
	GREATEST_SET_SETUP_CB(setup, NULL);
	GREATEST_SET_TEARDOWN_CB(teardown, NULL);
	GREATEST_RUN_TEST(basics_single);
	GREATEST_RUN_TEST(basics_repeat);
}

GREATEST_MAIN_DEFS();

int
main(int argc, char **argv)
{
	struct sigaction sa;

	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = SIG_IGN;
	sigaction(SIGUSR1, &sa, NULL);

	irc_bot_init();

	GREATEST_MAIN_BEGIN();
	GREATEST_RUN_SUITE(suite_basics);
	GREATEST_MAIN_END();

	return 0;
}
