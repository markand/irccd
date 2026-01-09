/*
 * main.cpp -- test hangman plugin
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

	plugin = js_plugin_open("hangman", TOP "/plugins/hangman/hangman.js");

	if (!plugin)
		irc_util_die("could not load plugin\n");

	irc_log_to_console();
	irc_server_incref(server);
	irc_plugin_set_template(plugin, "asked", "asked=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}:#{letter}");
	irc_plugin_set_template(plugin, "dead", "dead=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}:#{word}");
	irc_plugin_set_template(plugin, "found", "found=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}:#{word}");
	irc_plugin_set_template(plugin, "start", "start=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}:#{word}");
	irc_plugin_set_template(plugin, "running", "running=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}:#{word}");
	irc_plugin_set_template(plugin, "win", "win=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}:#{word}");
	irc_plugin_set_template(plugin, "wrong-letter", "wrong-letter=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}:#{letter}");
	irc_plugin_set_template(plugin, "wrong-player", "wrong-player=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}:#{letter}");
	irc_plugin_set_template(plugin, "wrong-word", "wrong-word=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}:#{word}");
	irc_plugin_set_option(plugin, "file", TOP "/tests/data/words.conf");
	irc_plugin_set_option(plugin, "collaborative", "false");
	irc_plugin_load(plugin);
}

void
tearDown(void)
{
	irc_plugin_finish(plugin);
	irc_server_decref(server);
}

static void
basics_asked(void)
{
	CALL(IRC_EVENT_COMMAND, "");
	TEST_ASSERT_EQUAL_STRING("message #hangman start=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:_ _ _", mock->out->line);

	CALL(IRC_EVENT_MESSAGE, "s");
	TEST_ASSERT_EQUAL_STRING("message #hangman found=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:s _ _", mock->out->line);

	CALL(IRC_EVENT_MESSAGE, "s");
	TEST_ASSERT_EQUAL_STRING("message #hangman asked=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:s", mock->out->line);
}

static void
basics_dead(void)
{
	CALL(IRC_EVENT_COMMAND, "");
	CALL(IRC_EVENT_MESSAGE, "a");
	CALL(IRC_EVENT_MESSAGE, "b");
	CALL(IRC_EVENT_MESSAGE, "c");
	CALL(IRC_EVENT_MESSAGE, "d");
	CALL(IRC_EVENT_MESSAGE, "e");
	CALL(IRC_EVENT_MESSAGE, "f");
	CALL(IRC_EVENT_MESSAGE, "g");
	CALL(IRC_EVENT_MESSAGE, "h");
	CALL(IRC_EVENT_MESSAGE, "i");
	CALL(IRC_EVENT_MESSAGE, "j");
	TEST_ASSERT_EQUAL_STRING("message #hangman dead=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:sky", mock->out->line);
}

static void
basics_found(void)
{
	CALL(IRC_EVENT_COMMAND, "");
	CALL(IRC_EVENT_MESSAGE, "s");
	TEST_ASSERT_EQUAL_STRING("message #hangman found=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:s _ _", mock->out->line);
}

static void
basics_start(void)
{
	CALL(IRC_EVENT_COMMAND, "");
	TEST_ASSERT_EQUAL_STRING("message #hangman start=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:_ _ _", mock->out->line);
}

static void
basics_win1(void)
{
	CALL(IRC_EVENT_COMMAND, "");
	CALL(IRC_EVENT_MESSAGE, "s");
	CALL(IRC_EVENT_MESSAGE, "k");
	CALL(IRC_EVENT_MESSAGE, "y");
	TEST_ASSERT_EQUAL_STRING("message #hangman win=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:sky", mock->out->line);
}

static void
basics_win2(void)
{
	CALL(IRC_EVENT_COMMAND, "");
	CALL(IRC_EVENT_COMMAND, "sky");
	TEST_ASSERT_EQUAL_STRING("message #hangman win=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:sky", mock->out->line);
}

static void
basics_wrong_letter(void)
{
	CALL(IRC_EVENT_COMMAND, "");
	CALL(IRC_EVENT_MESSAGE, "x");
	TEST_ASSERT_EQUAL_STRING("message #hangman wrong-letter=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:x", mock->out->line);
}

static void
basics_wrong_word(void)
{
	CALL(IRC_EVENT_COMMAND, "");
	CALL(IRC_EVENT_COMMAND, "cheese");
	TEST_ASSERT_EQUAL_STRING("message #hangman wrong-word=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:cheese", mock->out->line);
}

static void
basics_collaborative_enabled(void)
{
	irc_plugin_set_option(plugin, "collaborative", "true");

	CALL(IRC_EVENT_COMMAND, "");
	CALL(IRC_EVENT_MESSAGE, "s");

	/* Forbidden to play twice. */
	CALL(IRC_EVENT_MESSAGE, "k");
	TEST_ASSERT_EQUAL_STRING("message #hangman wrong-player=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:k", mock->out->line);

	/* Use a different nickname now. */
	CALL_EX(IRC_EVENT_MESSAGE, "francis!francis@localhost", "#hangman", "k");
	TEST_ASSERT_EQUAL_STRING("message #hangman found=hangman:!hangman:test:#hangman:francis!francis@localhost:francis:s k _", mock->out->line);
}

static void
basics_case_insensitive(void)
{
	CALL_EX(IRC_EVENT_COMMAND, "jean!jean@localhost", "#hangman", "");

	CALL_EX(IRC_EVENT_MESSAGE, "jean!jean@localhost", "#HANGMAN", "s");
	TEST_ASSERT_EQUAL_STRING("message #hangman found=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:s _ _", mock->out->line);

	CALL_EX(IRC_EVENT_MESSAGE, "jean!jean@localhost", "#HaNGMaN", "k");
	TEST_ASSERT_EQUAL_STRING("message #hangman found=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:s k _", mock->out->line);
}

static void
basics_query(void)
{
	/*
	 * Set collaborative mode but in query it must be ignored since there is
	 * only one player against the bot.
	 */
	irc_plugin_set_option(plugin, "collaborative", "true");

	CALL_EX(IRC_EVENT_COMMAND, "jean!jean@localhost", "t", "");
	TEST_ASSERT_EQUAL_STRING("message jean!jean@localhost start=hangman:!hangman:test:jean!jean@localhost:jean!jean@localhost:jean:_ _ _", mock->out->line);

	CALL_EX(IRC_EVENT_MESSAGE, "jean!jean@localhost", "t", "s");
	TEST_ASSERT_EQUAL_STRING("message jean!jean@localhost found=hangman:!hangman:test:jean!jean@localhost:jean!jean@localhost:jean:s _ _", mock->out->line);

	CALL_EX(IRC_EVENT_MESSAGE, "jean!jean@localhost", "t", "k");
	TEST_ASSERT_EQUAL_STRING("message jean!jean@localhost found=hangman:!hangman:test:jean!jean@localhost:jean!jean@localhost:jean:s k _", mock->out->line);

	CALL_EX(IRC_EVENT_COMMAND, "jean!jean@localhost", "t", "sky");
	TEST_ASSERT_EQUAL_STRING("message jean!jean@localhost win=hangman:!hangman:test:jean!jean@localhost:jean!jean@localhost:jean:sky", mock->out->line);
}

static void
basics_running(void)
{
	CALL(IRC_EVENT_COMMAND, "");
	CALL(IRC_EVENT_MESSAGE, "y");
	CALL(IRC_EVENT_COMMAND, "");
	TEST_ASSERT_EQUAL_STRING("message #hangman running=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:_ _ y", mock->out->line);
}

int
main(void)
{
	UNITY_BEGIN();

	RUN_TEST(basics_asked);
	RUN_TEST(basics_dead);
	RUN_TEST(basics_found);
	RUN_TEST(basics_start);
	RUN_TEST(basics_win1);
	RUN_TEST(basics_win2);
	RUN_TEST(basics_wrong_letter);
	RUN_TEST(basics_wrong_word);
	RUN_TEST(basics_collaborative_enabled);
	RUN_TEST(basics_case_insensitive);
	RUN_TEST(basics_query);
	RUN_TEST(basics_running);

	return UNITY_END();
}
