/*
 * test-plugin-history.c -- test history plugin
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

#include <irccd/compat.h>
#include <irccd/js-plugin.h>
#include <irccd/log.h>
#include <irccd/plugin.h>
#include <irccd/server.h>

#define CALL(t, m) do {                                                 \
	memset(server->conn.out, 0, sizeof (server->conn.out));         \
	irc_plugin_handle(plugin, &(const struct irc_event) {           \
		.type = t,                                              \
		.server = server,                                       \
			.message = {                                    \
			.origin = "jean!jean@localhost",                \
			.channel = "#history",                          \
			.message = m                                    \
		}                                                       \
	});                                                             \
} while (0)

#define CALL_EX(t, o, c, m) do {                                        \
	memset(server->conn.out, 0, sizeof (server->conn.out));         \
	irc_plugin_handle(plugin, &(const struct irc_event) {           \
		.type = t,                                              \
		.server = server,                                       \
			.message = {                                    \
			.origin = o,                                    \
			.channel = c,                                   \
			.message = m                                    \
		}                                                       \
	});                                                             \
} while (0)

static struct irc_server *server;
static struct irc_plugin *plugin;

static void
setup(void *udata)
{
	(void)udata;

	remove(BINARY "/seen.json");

	server = irc_server_new("test", "t", "t", "t", "127.0.0.1", 6667);
	plugin = js_plugin_open("history", CMAKE_SOURCE_DIR "/plugins/history/history.js");

	if (!plugin)
		errx(1, "could not load plugin");

	irc_log_to_console();
	irc_server_incref(server);
	irc_plugin_set_template(plugin, "error", "error=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}");
	irc_plugin_set_template(plugin, "seen", "seen=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}:#{target}:%H:%M");
	irc_plugin_set_template(plugin, "said", "said=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}:#{target}:#{message}:%H:%M");
	irc_plugin_set_template(plugin, "unknown", "unknown=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}:#{target}");
	irc_plugin_set_option(plugin, "file", BINARY "/seen.json");
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
basics_error(void)
{
	irc_plugin_set_option(plugin, "file", SOURCE "/data/error.json");
	CALL(IRC_EVENT_COMMAND, "seen francis");
	GREATEST_ASSERT_STR_EQ("PRIVMSG #history :error=history:!history:test:#history:jean!jean@localhost:jean\r\n", server->conn.out);
	GREATEST_PASS();
}

GREATEST_TEST
basics_seen(void)
{
	int d1, d2;

	CALL_EX(IRC_EVENT_MESSAGE, "jean!jean@localhost", "#history", "hello");
	CALL_EX(IRC_EVENT_COMMAND, "francis!francis@localhost", "#history", "seen jean");

	GREATEST_ASSERT_EQ(2, sscanf(server->conn.out, "PRIVMSG #history :seen=history:!history:test:#history:francis!francis@localhost:francis:jean:%d:%d\r\n", &d1, &d2));

	GREATEST_PASS();
}

GREATEST_TEST
basics_said(void)
{
	int d1, d2;

	CALL_EX(IRC_EVENT_MESSAGE, "jean!jean@localhost", "#history", "hello");
	CALL_EX(IRC_EVENT_COMMAND, "francis!francis@localhost", "#history", "said jean");

	GREATEST_ASSERT_EQ(2, sscanf(server->conn.out, "PRIVMSG #history :said=history:!history:test:#history:francis!francis@localhost:francis:jean:hello:%d:%d", &d1, &d2));

	GREATEST_PASS();
}

GREATEST_TEST
basics_unknown(void)
{
	CALL_EX(IRC_EVENT_MESSAGE, "jean!jean@localhost", "#history", "hello");
	CALL_EX(IRC_EVENT_COMMAND, "francis!francis@localhost", "#history", "said nobody");

	GREATEST_ASSERT_STR_EQ("PRIVMSG #history :unknown=history:!history:test:#history:francis!francis@localhost:francis:nobody\r\n", server->conn.out);
	GREATEST_PASS();
}

GREATEST_TEST
basics_case_insensitive(void)
{
	int d1, d2;

	CALL_EX(IRC_EVENT_MESSAGE, "JeaN!JeaN@localhost", "#history", "hello");

	CALL_EX(IRC_EVENT_COMMAND, "destructor!dst@localhost", "#HISTORY", "said JEAN");
	GREATEST_ASSERT_EQ(2, sscanf(server->conn.out, "PRIVMSG #history :said=history:!history:test:#history:destructor!dst@localhost:destructor:jean:hello:%d:%d\r\n", &d1, &d2));

	CALL_EX(IRC_EVENT_COMMAND, "destructor!dst@localhost", "#HiSToRy", "said JeaN");
	GREATEST_ASSERT_EQ(2, sscanf(server->conn.out, "PRIVMSG #history :said=history:!history:test:#history:destructor!dst@localhost:destructor:jean:hello:%d:%d\r\n", &d1, &d2));

	GREATEST_PASS();
}

GREATEST_SUITE(suite_basics)
{
	GREATEST_SET_SETUP_CB(setup, NULL);
	GREATEST_SET_TEARDOWN_CB(teardown, NULL);
	GREATEST_RUN_TEST(basics_error);
	GREATEST_RUN_TEST(basics_seen);
	GREATEST_RUN_TEST(basics_said);
	GREATEST_RUN_TEST(basics_unknown);
	GREATEST_RUN_TEST(basics_case_insensitive);
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


