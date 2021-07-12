/*
 * main.cpp -- test hangman plugin
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

#define GREATEST_USE_ABBREVS 0
#include <greatest.h>

#include <irccd/conn.h>
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
basics_asked(void)
{
	CALL(IRC_EVENT_COMMAND, "");
	GREATEST_ASSERT_STR_EQ("PRIVMSG #hangman :start=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:_ _ _\r\n", server->conn->out);

	CALL(IRC_EVENT_MESSAGE, "s");
	GREATEST_ASSERT_STR_EQ("PRIVMSG #hangman :found=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:s _ _\r\n", server->conn->out);

	CALL(IRC_EVENT_MESSAGE, "s");
	GREATEST_ASSERT_STR_EQ("PRIVMSG #hangman :asked=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:s\r\n", server->conn->out);
	GREATEST_PASS();
}

GREATEST_TEST
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
	GREATEST_ASSERT_STR_EQ("PRIVMSG #hangman :dead=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:sky\r\n", server->conn->out);
	GREATEST_PASS();
}

GREATEST_TEST
basics_found(void)
{
	CALL(IRC_EVENT_COMMAND, "");
	CALL(IRC_EVENT_MESSAGE, "s");
	GREATEST_ASSERT_STR_EQ("PRIVMSG #hangman :found=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:s _ _\r\n", server->conn->out);
	GREATEST_PASS();
}

GREATEST_TEST
basics_start(void)
{
	CALL(IRC_EVENT_COMMAND, "");
	GREATEST_ASSERT_STR_EQ("PRIVMSG #hangman :start=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:_ _ _\r\n", server->conn->out);
	GREATEST_PASS();
}

GREATEST_TEST
basics_win1(void)
{
	CALL(IRC_EVENT_COMMAND, "");
	CALL(IRC_EVENT_MESSAGE, "s");
	CALL(IRC_EVENT_MESSAGE, "k");
	CALL(IRC_EVENT_MESSAGE, "y");
	GREATEST_ASSERT_STR_EQ("PRIVMSG #hangman :win=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:sky\r\n", server->conn->out);
	GREATEST_PASS();
}

GREATEST_TEST
basics_win2(void)
{
	CALL(IRC_EVENT_COMMAND, "");
	CALL(IRC_EVENT_COMMAND, "sky");
	GREATEST_ASSERT_STR_EQ("PRIVMSG #hangman :win=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:sky\r\n", server->conn->out);
	GREATEST_PASS();
}

GREATEST_TEST
basics_wrong_letter(void)
{
	CALL(IRC_EVENT_COMMAND, "");
	CALL(IRC_EVENT_MESSAGE, "x");
	GREATEST_ASSERT_STR_EQ("PRIVMSG #hangman :wrong-letter=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:x\r\n", server->conn->out);
	GREATEST_PASS();
}

GREATEST_TEST
basics_wrong_word(void)
{
	CALL(IRC_EVENT_COMMAND, "");
	CALL(IRC_EVENT_COMMAND, "cheese");
	GREATEST_ASSERT_STR_EQ("PRIVMSG #hangman :wrong-word=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:cheese\r\n", server->conn->out);
	GREATEST_PASS();
}

GREATEST_TEST
basics_collaborative_enabled(void)
{
	irc_plugin_set_option(plugin, "collaborative", "true");

	CALL(IRC_EVENT_COMMAND, "");
	CALL(IRC_EVENT_MESSAGE, "s");

	/* Forbidden to play twice. */
	CALL(IRC_EVENT_MESSAGE, "k");
	GREATEST_ASSERT_STR_EQ("PRIVMSG #hangman :wrong-player=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:k\r\n", server->conn->out);

	/* Use a different nickname now. */
	CALL_EX(IRC_EVENT_MESSAGE, "francis!francis@localhost", "#hangman", "k");
	GREATEST_ASSERT_STR_EQ("PRIVMSG #hangman :found=hangman:!hangman:test:#hangman:francis!francis@localhost:francis:s k _\r\n", server->conn->out);

	GREATEST_PASS();
}

GREATEST_TEST
basics_case_insensitive(void)
{
	CALL_EX(IRC_EVENT_COMMAND, "jean!jean@localhost", "#hangman", "");

	CALL_EX(IRC_EVENT_MESSAGE, "jean!jean@localhost", "#HANGMAN", "s");
	GREATEST_ASSERT_STR_EQ("PRIVMSG #hangman :found=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:s _ _\r\n", server->conn->out);

	CALL_EX(IRC_EVENT_MESSAGE, "jean!jean@localhost", "#HaNGMaN", "k");
	GREATEST_ASSERT_STR_EQ("PRIVMSG #hangman :found=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:s k _\r\n", server->conn->out);

	GREATEST_PASS();
}

GREATEST_TEST
basics_query(void)
{
	/*
	 * Set collaborative mode but in query it must be ignored since there is
	 * only one player against the bot.
	 */
	irc_plugin_set_option(plugin, "collaborative", "true");

	CALL_EX(IRC_EVENT_COMMAND, "jean!jean@localhost", "t", "");
	GREATEST_ASSERT_STR_EQ("PRIVMSG jean!jean@localhost :start=hangman:!hangman:test:jean!jean@localhost:jean!jean@localhost:jean:_ _ _\r\n", server->conn->out);

	CALL_EX(IRC_EVENT_MESSAGE, "jean!jean@localhost", "t", "s");
	GREATEST_ASSERT_STR_EQ("PRIVMSG jean!jean@localhost :found=hangman:!hangman:test:jean!jean@localhost:jean!jean@localhost:jean:s _ _\r\n", server->conn->out);

	CALL_EX(IRC_EVENT_MESSAGE, "jean!jean@localhost", "t", "k");
	GREATEST_ASSERT_STR_EQ("PRIVMSG jean!jean@localhost :found=hangman:!hangman:test:jean!jean@localhost:jean!jean@localhost:jean:s k _\r\n", server->conn->out);

	CALL_EX(IRC_EVENT_COMMAND, "jean!jean@localhost", "t", "sky");
	GREATEST_ASSERT_STR_EQ("PRIVMSG jean!jean@localhost :win=hangman:!hangman:test:jean!jean@localhost:jean!jean@localhost:jean:sky\r\n", server->conn->out);

	GREATEST_PASS();
}

GREATEST_TEST
basics_running(void)
{
	CALL(IRC_EVENT_COMMAND, "");
	CALL(IRC_EVENT_MESSAGE, "y");
	CALL(IRC_EVENT_COMMAND, "");
	GREATEST_ASSERT_STR_EQ("PRIVMSG #hangman :running=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:_ _ y\r\n", server->conn->out);
	GREATEST_PASS();
}

GREATEST_SUITE(suite_basics)
{
	GREATEST_SET_SETUP_CB(setup, NULL);
	GREATEST_SET_TEARDOWN_CB(teardown, NULL);
	GREATEST_RUN_TEST(basics_asked);
	GREATEST_RUN_TEST(basics_dead);
	GREATEST_RUN_TEST(basics_found);
	GREATEST_RUN_TEST(basics_start);
	GREATEST_RUN_TEST(basics_win1);
	GREATEST_RUN_TEST(basics_win2);
	GREATEST_RUN_TEST(basics_wrong_letter);
	GREATEST_RUN_TEST(basics_wrong_word);
	GREATEST_RUN_TEST(basics_collaborative_enabled);
	GREATEST_RUN_TEST(basics_case_insensitive);
	GREATEST_RUN_TEST(basics_query);
	GREATEST_RUN_TEST(basics_running);
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
