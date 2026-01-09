/*
 * test-plugin-history.c -- test history plugin
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

#include <irccd/js-plugin.h>
#include <irccd/log.h>
#include <irccd/plugin.h>

#include "mock/server.h"

#define CALL(t, m) do {                                                 \
        irc_plugin_handle(plugin, &(const struct irc_event) {           \
                .type = t,                                              \
                .server = server,                                       \
                .message = {                                            \
                        .origin = "jean!jean@localhost",                \
                        .channel = "#history",                          \
                        .message = m                                    \
                }                                                       \
        });                                                             \
} while (0)

#define CALL_EX(t, o, c, m) do {                                        \
        irc_plugin_handle(plugin, &(const struct irc_event) {           \
                .type = t,                                              \
                .server = server,                                       \
                .message = {                                            \
                        .origin = o,                                    \
                        .channel = c,                                   \
                        .message = m                                    \
                }                                                       \
        });                                                             \
} while (0)

static struct irc_server *server;
static struct mock_server *mock;
static struct irc_plugin *plugin;

void
setUp(void)
{
	remove(TOP "/tests/seen.json");

	server = irc_server_new("test");
	mock = IRC_UTIL_CONTAINER_OF(server, struct mock_server, parent);
	plugin = js_plugin_open("history", TOP "/plugins/history/history.js");

	if (!plugin)
		irc_util_die("could not load plugin\n");

	irc_log_to_console();
	irc_server_incref(server);
	irc_plugin_set_template(plugin, "error", "error=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}");
	irc_plugin_set_template(plugin, "seen", "seen=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}:#{target}:%H:%M");
	irc_plugin_set_template(plugin, "said", "said=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}:#{target}:#{message}:%H:%M");
	irc_plugin_set_template(plugin, "silent", "silent=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}:#{target}");
	irc_plugin_set_template(plugin, "unknown", "unknown=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}:#{target}");
	irc_plugin_set_option(plugin, "file", TOP "/tests/seen.json");
	irc_plugin_load(plugin);
}

void
tearDown(void)
{
	remove(TOP "/tests/seen.json");

	irc_plugin_finish(plugin);
	irc_server_decref(server);
}

static void
basics_error(void)
{
	irc_plugin_set_option(plugin, "file", TOP "/tests/data/error.json");
	CALL(IRC_EVENT_COMMAND, "seen francis");
	TEST_ASSERT_EQUAL_STRING("message #history error=history:!history:test:#history:jean!jean@localhost:jean", mock->out->line);
}

static void
basics_seen(void)
{
	int d1, d2;

	CALL_EX(IRC_EVENT_MESSAGE, "jean!jean@localhost", "#history", "hello");
	CALL_EX(IRC_EVENT_COMMAND, "francis!francis@localhost", "#history", "seen jean");

	TEST_ASSERT_EQUAL_INT(2, sscanf(mock->out->line, "message #history seen=history:!history:test:#history:francis!francis@localhost:francis:jean:%d:%d", &d1, &d2));
}

static void
basics_said(void)
{
	int d1, d2;

	CALL_EX(IRC_EVENT_MESSAGE, "jean!jean@localhost", "#history", "hello");
	CALL_EX(IRC_EVENT_COMMAND, "francis!francis@localhost", "#history", "said jean");

	TEST_ASSERT_EQUAL_INT(2, sscanf(mock->out->line, "message #history said=history:!history:test:#history:francis!francis@localhost:francis:jean:hello:%d:%d", &d1, &d2));
}

static void
basics_silent(void)
{
	/* Join but without any message. */
	irc_plugin_handle(plugin, &(const struct irc_event) {
		.type = IRC_EVENT_JOIN,
		.server = server,
		.join = {
			.origin = "jean!jean@localhost",
			.channel = "#history"
		}
	});
	CALL_EX(IRC_EVENT_COMMAND, "francis!francis@localhost", "#history", "said jean");

	TEST_ASSERT_EQUAL_STRING("message #history silent=history:!history:test:#history:francis!francis@localhost:francis:jean", mock->out->line);
}

static void
basics_unknown(void)
{
	CALL_EX(IRC_EVENT_MESSAGE, "jean!jean@localhost", "#history", "hello");
	CALL_EX(IRC_EVENT_COMMAND, "francis!francis@localhost", "#history", "said nobody");

	TEST_ASSERT_EQUAL_STRING("message #history unknown=history:!history:test:#history:francis!francis@localhost:francis:nobody", mock->out->line);
}

static void
basics_case_insensitive(void)
{
	int d1, d2;

	CALL_EX(IRC_EVENT_MESSAGE, "JeaN!JeaN@localhost", "#history", "hello");

	CALL_EX(IRC_EVENT_COMMAND, "destructor!dst@localhost", "#HISTORY", "said JEAN");
	TEST_ASSERT_EQUAL_INT(2, sscanf(mock->out->line, "message #history said=history:!history:test:#history:destructor!dst@localhost:destructor:jean:hello:%d:%d", &d1, &d2));

	CALL_EX(IRC_EVENT_COMMAND, "destructor!dst@localhost", "#HiSToRy", "said JeaN");
	TEST_ASSERT_EQUAL_INT(2, sscanf(mock->out->line, "message #history said=history:!history:test:#history:destructor!dst@localhost:destructor:jean:hello:%d:%d", &d1, &d2));
}

int
main(void)
{
	UNITY_BEGIN();

	RUN_TEST(basics_error);
	RUN_TEST(basics_seen);
	RUN_TEST(basics_said);
	RUN_TEST(basics_silent);
	RUN_TEST(basics_unknown);
	RUN_TEST(basics_case_insensitive);

	return UNITY_END();
}
