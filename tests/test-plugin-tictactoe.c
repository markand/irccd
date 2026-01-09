/*
 * test-plugin-tictactoe.c -- test tictactoe plugin
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

#include <irccd/irccd.h>
#include <irccd/js-plugin.h>
#include <irccd/log.h>
#include <irccd/plugin.h>

#include "mock/server.h"

#define CALL(t, m) do {                                                 \
        irc_plugin_handle(plugin, &(const struct irc_event) {           \
                .type = t,                                              \
                .server = server,                                       \
                        .message = {                                    \
                        .origin = "jean!jean@localhost",                \
                        .channel = "#hangman",                          \
                        .message = m                                    \
                }                                                       \
        });                                                             \
} while (0)

#define CALL_EX(t, o, c, m) do {                                        \
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
static struct mock_server *mock;
static struct irc_plugin *plugin;

void
setUp(void)
{
	server = irc_server_new("test");
	mock = IRC_UTIL_CONTAINER_OF(server, struct mock_server, parent);
	plugin = js_plugin_open("tictactoe", TOP "/plugins/tictactoe/tictactoe.js");

	if (!plugin)
		irc_util_die("could not load plugin\n");

	irc_bot_init(NULL);
	irc_server_incref(server);
	irc_plugin_set_template(plugin, "draw", "draw=#{channel}:#{command}:#{nickname}:#{plugin}:#{server}");
	irc_plugin_set_template(plugin, "invalid", "invalid=#{channel}:#{command}:#{nickname}:#{origin}:#{plugin}:#{server}");
	irc_plugin_set_template(plugin, "running", "running=#{channel}:#{command}:#{nickname}:#{origin}:#{plugin}:#{server}");
	irc_plugin_set_template(plugin, "turn", "turn=#{channel}:#{command}:#{nickname}:#{plugin}:#{server}");
	irc_plugin_set_template(plugin, "used", "used=#{channel}:#{command}:#{nickname}:#{origin}:#{plugin}:#{server}");
	irc_plugin_set_template(plugin, "win", "win=#{channel}:#{command}:#{nickname}:#{plugin}:#{server}");
	irc_plugin_load(plugin);

	/* We need two players on a channel to play the game. */
	irc_server_join(server, "#tictactoe", NULL);
	irc_channel_add(server->channels, "a", 0);
	irc_channel_add(server->channels, "b", 0);
}

void
tearDown(void)
{
	irc_plugin_finish(plugin);
	irc_server_decref(server);
}


static const char *
line_no(size_t index)
{
	struct mock_server_msg *msg = mock->out;

	for (; index; --index)
		msg = msg ? msg->next : msg;

	return msg ? msg->line : "";
}

static char
next(void)
{
	char player = 0;

	if (sscanf(line_no(0), "message #tictactoe turn=#tictactoe:!tictactoe:%c:tictactoe:test", &player) != 1)
		irc_util_die("could not determine player\n");

	return player;
}

static void
play(const char *value)
{
	char player[] = { next(), '\0' };

	CALL_EX(IRC_EVENT_MESSAGE, player, "#tictactoe", (char *)value);
}

static void
basics_win(void)
{
	char k1, k2;

	CALL_EX(IRC_EVENT_COMMAND, "a", "#tictactoe", "b");

	play("a 1");
	play("b1");
	play("a 2");
	play("b2");
	play("a3");

	TEST_ASSERT_EQUAL_INT(0, sscanf(line_no(4), "message #tictactoe   a b c"));
	TEST_ASSERT_EQUAL_INT(2, sscanf(line_no(3), "message #tictactoe 1 %c %c .", &k1, &k2));
	TEST_ASSERT_EQUAL_INT(2, sscanf(line_no(2), "message #tictactoe 2 %c %c .", &k1, &k2));
	TEST_ASSERT_EQUAL_INT(1, sscanf(line_no(1), "message #tictactoe 3 %c . .", &k1));
	TEST_ASSERT_EQUAL_INT(1, sscanf(line_no(0), "message #tictactoe win=#tictactoe:!tictactoe:%c:tictactoe:test", &k1));
}

static void
basics_draw(void)
{
	/*
	 *   a b c
	 * 1 o x o
	 * 2 o x x
	 * 3 x o x
	 */
	char k1, k2, k3;

	CALL_EX(IRC_EVENT_COMMAND, "a", "#tictactoe", "b");

	play("b 2");
	play("c 1");
	play("c 3");
	play("b 3");
	play("c 2");
	play("a 2");
	play("a 3");
	play("a 1");
	play("b 1");

	TEST_ASSERT_EQUAL_INT(0, sscanf(line_no(4), "message #tictactoe   a b c"));
	TEST_ASSERT_EQUAL_INT(3, sscanf(line_no(3), "message #tictactoe 1 %c %c %c", &k1, &k2, &k3));
	TEST_ASSERT_EQUAL_INT(3, sscanf(line_no(2), "message #tictactoe 2 %c %c %c", &k1, &k2, &k3));
	TEST_ASSERT_EQUAL_INT(3, sscanf(line_no(1), "message #tictactoe 3 %c %c %c", &k1, &k2, &k3));
	TEST_ASSERT_EQUAL_INT(1, sscanf(line_no(0), "message #tictactoe draw=#tictactoe:!tictactoe:%c:tictactoe:test", &k1));
}

static void
basics_used(void)
{
	char k1, k2;

	CALL_EX(IRC_EVENT_COMMAND, "a", "#tictactoe", "b");

	play("a 1");
	play("a 1");

	TEST_ASSERT_EQUAL_INT(2, sscanf(mock->out->line, "message #tictactoe used=#tictactoe:!tictactoe:%c:%c:tictactoe:testn", &k1, &k2));
}

static void
basics_invalid(void)
{
	char k1, k2;

	/* Player select itself. */
	CALL_EX(IRC_EVENT_COMMAND, "a", "#tictactoe", "a");
	TEST_ASSERT_EQUAL_INT(2, sscanf(mock->out->line, "message #tictactoe invalid=#tictactoe:!tictactoe:%c:%c:tictactoe:test", &k1, &k2));

	/* Player select the bot. */
	CALL_EX(IRC_EVENT_COMMAND, "a", "#tictactoe", "t");
	TEST_ASSERT_EQUAL_INT(2, sscanf(mock->out->line, "message #tictactoe invalid=#tictactoe:!tictactoe:%c:%c:tictactoe:test", &k1, &k2));

	/* Someone not on the channel. */
	CALL_EX(IRC_EVENT_COMMAND, "a", "#tictactoe", "jean");
	TEST_ASSERT_EQUAL_INT(2, sscanf(mock->out->line, "message #tictactoe invalid=#tictactoe:!tictactoe:%c:%c:tictactoe:test", &k1, &k2));
}

static void
basics_random(void)
{
	/*
	 * Ensure that the first player is not always the originator, start the
	 * game for at most 100 times to avoid forever loop.
	 */
	int count = 0, a = 0, b = 0;

	/* Last player turn is the winner. */
	while (count++ < 100) {
		CALL_EX(IRC_EVENT_COMMAND, "a", "#tictactoe", "b");

		play("a 1");
		play("b 1");
		play("a 2");
		play("b 2");

		/* This is the player that will win. */
		if (next() == 'a')
			a = 1;
		else
			b = 1;

		play("a 3");
	}

	TEST_ASSERT(a);
	TEST_ASSERT(b);
}

static void
basics_disconnect(void)
{
	CALL_EX(IRC_EVENT_COMMAND, "a", "#tictactoe", "b");

	irc_plugin_handle(plugin, &(const struct irc_event) {
		.type = IRC_EVENT_DISCONNECT,
		.server = server
	});

	/*
	 * Clear the output of the server, the plugin should drop the game for
	 * this server/channel couple and thus the next player would not
	 * generate any kind of message from the plugin.
	 */
	mock_server_clear(server);
	CALL_EX(IRC_EVENT_COMMAND, "a", "#tictactoe", "a 1");

	/*
	 * Server is still connected, so we expect the plugin to tell that the
	 * game is invalid.
	 */
	TEST_ASSERT_EQUAL_STRING(line_no(0), "message #tictactoe invalid=#tictactoe:!tictactoe:a:a:tictactoe:test");
}

static void
basics_kick(void)
{
	const void *addr;

	CALL_EX(IRC_EVENT_COMMAND, "a", "#tictactoe", "b");

	irc_plugin_handle(plugin, &(const struct irc_event) {
		.type = IRC_EVENT_KICK,
		.server = server,
		.kick = {
			.origin = "god",
			.channel = "#TiCTaCToE",
			.target = "a",
			.reason = "No reason, I do what I want."
		}
	});

	/*
	 * We must have the exact same response before and after the user
	 * attempt of playing.
	 */
	addr = mock->out;
	play("a 1");
	TEST_ASSERT(addr == mock->out);
}

static void
basics_part(void)
{
	const void *addr;

	CALL_EX(IRC_EVENT_COMMAND, "a", "#tictactoe", "b");

	irc_plugin_handle(plugin, &(const struct irc_event) {
		.type = IRC_EVENT_PART,
		.server = server,
		.part = {
			.origin = "a",
			.channel = "#TiCTaCToE",
			.reason = "I'm too bad at this game."
		}
	});

	/* Exactly the same case as basics_kick. */
	addr = mock->out;
	play("a 1");
	TEST_ASSERT(addr == mock->out);
}

int
main(void)
{
	UNITY_BEGIN();

	RUN_TEST(basics_win);
	RUN_TEST(basics_draw);
	RUN_TEST(basics_used);
	RUN_TEST(basics_invalid);
	RUN_TEST(basics_random);
	RUN_TEST(basics_disconnect);
	RUN_TEST(basics_kick);
	RUN_TEST(basics_part);

	return UNITY_END();
}
