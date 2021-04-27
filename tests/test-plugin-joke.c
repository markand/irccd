/*
 * main.cpp -- test joke plugin
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

#include <err.h>

#define GREATEST_USE_ABBREVS 0
#include <greatest.h>

#include <irccd/conn.h>
#include <irccd/js-plugin.h>
#include <irccd/plugin.h>
#include <irccd/server.h>

#define CALL() do {                                                     \
        memset(server->conn->out, 0, sizeof (server->conn->out));       \
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
static struct irc_plugin *plugin;

static void
setup(void *udata)
{
	(void)udata;

	server = irc_server_new("test", "t", "t", "t", "127.0.0.1", 6667);
	plugin = js_plugin_open("joke", TOP "/plugins/joke/joke.js");

	if (!plugin)
		errx(1, "could not load plugin");

	irc_server_incref(server);
	irc_plugin_set_template(plugin, "error", "error=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}");

	irc_plugin_set_option(plugin, "file", TOP "/tests/data/joke/jokes.json");
	irc_plugin_load(plugin);

	/* Fake server connected to send data. */
	server->state = IRC_SERVER_STATE_CONNECTED;
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
	int aaa = 0, bbbb = 0;

	for (int i = 0; i < 2; ++i) {
		CALL();

		if (strcmp(server->conn->out, "PRIVMSG #joke :aaa\r\n") == 0)
			aaa = 1;
		else if (strcmp(server->conn->out, "PRIVMSG #joke :bbbb\r\nPRIVMSG #joke :bbbb\r\n") == 0)
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
		GREATEST_ASSERT_STR_EQ("PRIVMSG #joke :a\r\n", server->conn->out);
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
		GREATEST_ASSERT_STR_EQ("PRIVMSG #joke :a\r\n", server->conn->out);
	}

	GREATEST_PASS();
}

GREATEST_TEST
errors_not_found(void)
{
	irc_plugin_set_option(plugin, "file", "doesnotexist.json");

	CALL();
	GREATEST_ASSERT_STR_EQ("PRIVMSG #joke :error=joke:!joke:test:#joke:jean!jean@localhost:jean\r\n", server->conn->out);

	GREATEST_PASS();
}

GREATEST_TEST
errors_not_array(void)
{
	irc_plugin_set_option(plugin, "file", TOP "/tests/data/joke/error-not-array.json");

	CALL();
	GREATEST_ASSERT_STR_EQ("PRIVMSG #joke :error=joke:!joke:test:#joke:jean!jean@localhost:jean\r\n", server->conn->out);

	GREATEST_PASS();
}

GREATEST_TEST
errors_empty(void)
{
	irc_plugin_set_option(plugin, "file", TOP "/tests/data/joke/error-empty.json");

	CALL();
	GREATEST_ASSERT_STR_EQ("PRIVMSG #joke :error=joke:!joke:test:#joke:jean!jean@localhost:jean\r\n", server->conn->out);

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
