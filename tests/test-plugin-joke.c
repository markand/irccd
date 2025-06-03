/*
 * main.cpp -- test joke plugin
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

#define GREATEST_USE_ABBREVS 0
#include <greatest.h>

#include "mock/server.c"

#include <irccd/js-plugin.h>
#include <irccd/plugin.h>
#include <irccd/server.h>
#include <irccd/util.h>

#define CALL() do {                                                     \
        irc_plugin_handle(plugin, &(const struct irc_event) {           \
                .type = IRC_EVENT_COMMAND,                              \
                .server = server,                                       \
                        .message = {                                    \
                        .origin = "jean!jean@localhost",                \
                        .channel = "#joke",                             \
                        .message = ""                                   \
                }                                                       \
        });                                                             \
} while (0)

static struct irc_server *server;
static struct mock_server *mock;
static struct irc_plugin *plugin;

static void
setup(void *udata)
{
	(void)udata;

	server = irc_server_new("test");
	mock = IRC_UTIL_CONTAINER_OF(server, struct mock_server, parent);
	plugin = js_plugin_open("joke", TOP "/plugins/joke/joke.js");

	if (!plugin)
		irc_util_die("could not load plugin\n");

	irc_server_incref(server);
	irc_plugin_set_template(plugin, "error", "error=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}");
	irc_plugin_set_option(plugin, "file", TOP "/tests/data/joke/jokes.json");
	irc_plugin_load(plugin);
}

static void
teardown(void *udata)
{
	(void)udata;

	irc_plugin_finish(plugin);
	irc_server_decref(server);
}

GREATEST_TEST
basics_simple(void)
{
	/*
	 * jokes.json have two jokes.
	 *
	 * aaa
	 *
	 * And
	 *
	 * bbbb
	 * bbbb
	 */
	struct mock_server_msg *msg;
	int aaa = 0, bbbb = 0;

	for (int i = 0; i < 2; ++i)
		CALL();

	LL_FOREACH(mock->out, msg) {
		if (strcmp(msg->line, "message #joke aaa") == 0)
			aaa = 1;
		else if (strcmp(msg->line, "message #joke bbbb") == 0)
			bbbb = 1;
	}

	GREATEST_ASSERT(aaa);
	GREATEST_ASSERT(bbbb);

	GREATEST_PASS();
}

GREATEST_TEST
errors_toobig(void)
{
	/*
	 * The jokes "xxx" and "yyy" are both 3-lines which we disallow. only a
	 * must be said.
	 */
	irc_plugin_set_option(plugin, "file", TOP "/tests/data/joke/error-toobig.json");
	irc_plugin_set_option(plugin, "max-list-lines", "2");

	for (int i = 0; i < 64; ++i) {
		CALL();
		GREATEST_ASSERT_STR_EQ("message #joke a", mock->out->line);
	}

	GREATEST_PASS();
}

GREATEST_TEST
errors_invalid(void)
{
	/* Only a is the valid joke in this file. */
	irc_plugin_set_option(plugin, "file", TOP "/tests/data/joke/error-invalid.json");
	irc_plugin_set_option(plugin, "max-list-lines", "2");

	for (int i = 0; i < 64; ++i) {
		CALL();
		GREATEST_ASSERT_STR_EQ("message #joke a", mock->out->line);
	}

	GREATEST_PASS();
}

GREATEST_TEST
errors_not_found(void)
{
	irc_plugin_set_option(plugin, "file", "doesnotexist.json");

	CALL();
	GREATEST_ASSERT_STR_EQ("message #joke error=joke:!joke:test:#joke:jean!jean@localhost:jean", mock->out->line);

	GREATEST_PASS();
}

GREATEST_TEST
errors_not_array(void)
{
	irc_plugin_set_option(plugin, "file", TOP "/tests/data/joke/error-not-array.json");

	CALL();
	GREATEST_ASSERT_STR_EQ("message #joke error=joke:!joke:test:#joke:jean!jean@localhost:jean", mock->out->line);

	GREATEST_PASS();
}

GREATEST_TEST
errors_empty(void)
{
	irc_plugin_set_option(plugin, "file", TOP "/tests/data/joke/error-empty.json");

	CALL();
	GREATEST_ASSERT_STR_EQ("message #joke error=joke:!joke:test:#joke:jean!jean@localhost:jean", mock->out->line);

	GREATEST_PASS();
}

GREATEST_SUITE(suite_basics)
{
	GREATEST_SET_SETUP_CB(setup, NULL);
	GREATEST_SET_TEARDOWN_CB(teardown, NULL);
	GREATEST_RUN_TEST(basics_simple);
}

GREATEST_SUITE(suite_errors)
{
	GREATEST_SET_SETUP_CB(setup, NULL);
	GREATEST_SET_TEARDOWN_CB(teardown, NULL);
	GREATEST_RUN_TEST(errors_toobig);
	GREATEST_RUN_TEST(errors_invalid);
	GREATEST_RUN_TEST(errors_not_found);
	GREATEST_RUN_TEST(errors_not_array);
	GREATEST_RUN_TEST(errors_empty);
}

GREATEST_MAIN_DEFS();

int
main(int argc, char **argv)
{
	GREATEST_MAIN_BEGIN();
	GREATEST_RUN_SUITE(suite_basics);
	GREATEST_RUN_SUITE(suite_errors);
	GREATEST_MAIN_END();

	return 0;
}
