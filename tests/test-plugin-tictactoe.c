/*
 * test-plugin-tictactoe.c -- test tictactoe plugin
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
#include <irccd/irccd.h>
#include <irccd/js-plugin.h>
#include <irccd/log.h>
#include <irccd/plugin.h>
#include <irccd/server.h>
#include <irccd/util.h>

#define CALL(t, m) do {                                                 \
        memset(server->conn->out, 0, sizeof (server->conn->out));       \
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
        memset(server->conn->out, 0, sizeof (server->conn->out));       \
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

	server = irc_server_new("test", "t", "t", "t", "127.0.0.1", 6667);
	plugin = js_plugin_open("tictactoe", TOP "/plugins/tictactoe/tictactoe.js");

	if (!plugin)
		errx(1, "could not load plugin");

	irc_bot_init();
	irc_server_incref(server);
	irc_plugin_set_template(plugin, "draw", "draw=#{channel}:#{command}:#{nickname}:#{plugin}:#{server}");
	irc_plugin_set_template(plugin, "invalid", "invalid=#{channel}:#{command}:#{nickname}:#{origin}:#{plugin}:#{server}");
	irc_plugin_set_template(plugin, "running", "running=#{channel}:#{command}:#{nickname}:#{origin}:#{plugin}:#{server}");
	irc_plugin_set_template(plugin, "turn", "turn=#{channel}:#{command}:#{nickname}:#{plugin}:#{server}");
	irc_plugin_set_template(plugin, "used", "used=#{channel}:#{command}:#{nickname}:#{origin}:#{plugin}:#{server}");
	irc_plugin_set_template(plugin, "win", "win=#{channel}:#{command}:#{nickname}:#{plugin}:#{server}");
	irc_plugin_load(plugin);

	/* We need tw players on a channel to play the game. */
	irc_server_join(server, "#tictactoe", NULL);
	irc_channel_add(LIST_FIRST(&server->channels), "a", 0);
	irc_channel_add(LIST_FIRST(&server->channels), "b", 0);

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

static char
next(void)
{
	const char *lines[5] = {0};
	char player = 0, *buf;

	/* We need to skip 4 lines.*/
	buf = irc_util_strdup(server->conn->out);
	irc_util_split(buf, lines, 5, '\n');

	if (!lines[4] || sscanf(lines[4], "PRIVMSG #tictactoe :turn=#tictactoe:!tictactoe:%c:tictactoe:test\r\n", &player) != 1)
		errx(1, "could not determine player");

	free(buf);

	return player;
}

static void
play(const char *value)
{
	char player[] = { next(), '\0' };

	CALL_EX(IRC_EVENT_MESSAGE, player, "#tictactoe", (char *)value);
}

GREATEST_TEST
basics_win(void)
{
	const char *lines[5] = {0};
	char k1, k2;

	CALL_EX(IRC_EVENT_COMMAND, "a", "#tictactoe", "b");

	play("a 1");
	play("b1");
	play("a 2");
	play("b2");
	play("a3");

	GREATEST_ASSERT_EQ(5U, irc_util_split(server->conn->out, lines, 5, '\n'));
	GREATEST_ASSERT_EQ(0, sscanf(lines[0], "PRIVMSG #tictactoe :  a b c\r"));
	GREATEST_ASSERT_EQ(2, sscanf(lines[1], "PRIVMSG #tictactoe :1 %c %c .\r", &k1, &k2));
	GREATEST_ASSERT_EQ(2, sscanf(lines[2], "PRIVMSG #tictactoe :2 %c %c .\r", &k1, &k2));
	GREATEST_ASSERT_EQ(1, sscanf(lines[3], "PRIVMSG #tictactoe :3 %c . .\r", &k1));
	GREATEST_ASSERT_EQ(1, sscanf(lines[4], "PRIVMSG #tictactoe :win=#tictactoe:!tictactoe:%c:tictactoe:test\r\n", &k1));

	GREATEST_PASS();
}

GREATEST_TEST
basics_draw(void)
{
	/*
	 *   a b c
	 * 1 o x o
	 * 2 o x x
	 * 3 x o x
	 */
	const char *lines[5] = {0};
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

	GREATEST_ASSERT_EQ(5U, irc_util_split(server->conn->out, lines, 5, '\n'));
	GREATEST_ASSERT_EQ(0, sscanf(lines[0], "PRIVMSG #tictactoe :  a b c\r"));
	GREATEST_ASSERT_EQ(3, sscanf(lines[1], "PRIVMSG #tictactoe :1 %c %c %c\r", &k1, &k2, &k3));
	GREATEST_ASSERT_EQ(3, sscanf(lines[2], "PRIVMSG #tictactoe :2 %c %c %c\r", &k1, &k2, &k3));
	GREATEST_ASSERT_EQ(3, sscanf(lines[3], "PRIVMSG #tictactoe :3 %c %c %c\r", &k1, &k2, &k3));
	GREATEST_ASSERT_EQ(1, sscanf(lines[4], "PRIVMSG #tictactoe :draw=#tictactoe:!tictactoe:%c:tictactoe:test\r\n", &k1));

	GREATEST_PASS();
}

GREATEST_TEST
basics_used(void)
{
	char k1, k2;

	CALL_EX(IRC_EVENT_COMMAND, "a", "#tictactoe", "b");

	play("a 1");
	play("a 1");

	GREATEST_ASSERT_EQ(2, sscanf(server->conn->out, "PRIVMSG #tictactoe :used=#tictactoe:!tictactoe:%c:%c:tictactoe:test\r\n", &k1, &k2));

	GREATEST_PASS();
}

GREATEST_TEST
basics_invalid(void)
{
	char k1, k2;

	/* Player select itself. */
	CALL_EX(IRC_EVENT_COMMAND, "a", "#tictactoe", "a");
	GREATEST_ASSERT_EQ(2, sscanf(server->conn->out, "PRIVMSG #tictactoe :invalid=#tictactoe:!tictactoe:%c:%c:tictactoe:test\r\n", &k1, &k2));

	/* Player select the bot. */
	CALL_EX(IRC_EVENT_COMMAND, "a", "#tictactoe", "t");
	GREATEST_ASSERT_EQ(2, sscanf(server->conn->out, "PRIVMSG #tictactoe :invalid=#tictactoe:!tictactoe:%c:%c:tictactoe:test\r\n", &k1, &k2));

	/* Someone not on the channel. */
	CALL_EX(IRC_EVENT_COMMAND, "a", "#tictactoe", "jean");
	GREATEST_ASSERT_EQ(2, sscanf(server->conn->out, "PRIVMSG #tictactoe :invalid=#tictactoe:!tictactoe:%c:%c:tictactoe:test\r\n", &k1, &k2));

	GREATEST_PASS();
}

GREATEST_TEST
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

	GREATEST_ASSERT(a);
	GREATEST_ASSERT(b);

	GREATEST_PASS();
}

GREATEST_TEST
basics_disconnect(void)
{
	CALL_EX(IRC_EVENT_COMMAND, "a", "#tictactoe", "b");

	irc_plugin_handle(plugin, &(const struct irc_event) {
		.type = IRC_EVENT_DISCONNECT,
		.server = server
	});

	play("a 1");
	GREATEST_ASSERT_STR_EQ("", server->conn->out);

	GREATEST_PASS();
}

GREATEST_TEST
basics_kick(void)
{
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

	play("a 1");
	GREATEST_ASSERT_STR_EQ("", server->conn->out);

	GREATEST_PASS();
}

GREATEST_TEST
basics_part(void)
{
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

	play("a 1");
	GREATEST_ASSERT_STR_EQ("", server->conn->out);

	GREATEST_PASS();
}

GREATEST_SUITE(suite_basics)
{
	GREATEST_SET_SETUP_CB(setup, NULL);
	GREATEST_SET_TEARDOWN_CB(teardown, NULL);
	GREATEST_RUN_TEST(basics_win);
	GREATEST_RUN_TEST(basics_draw);
	GREATEST_RUN_TEST(basics_used);
	GREATEST_RUN_TEST(basics_invalid);
	GREATEST_RUN_TEST(basics_random);
	GREATEST_RUN_TEST(basics_disconnect);
	GREATEST_RUN_TEST(basics_kick);
	GREATEST_RUN_TEST(basics_part);
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
