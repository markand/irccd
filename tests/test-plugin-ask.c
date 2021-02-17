/*
 * test-plugin-ask.c -- test ask plugin
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
#include <irccd/conn.h>
#include <irccd/js-plugin.h>
#include <irccd/plugin.h>
#include <irccd/server.h>

static struct irc_server *server;
static struct irc_plugin *plugin;

static void
setup(void *udata)
{
	(void)udata;

	server = irc_server_new("test", "t", "t", "t", "127.0.0.1", 6667);
	plugin = js_plugin_open("test", CMAKE_SOURCE_DIR "/plugins/ask/ask.js");

	if (!plugin)
		errx(1, "could not load plugin");

	irc_server_incref(server);
	irc_plugin_set_option(plugin, "file", SOURCE "/data/answers.conf");
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
	int no = 0, yes = 0;

	/*
	 * Invoke the plugin 1000 times, it will be very unlucky to not have
	 * both answers in that amount of tries.
	 */
	for (int i = 0; i < 1000; ++i) {
		irc_plugin_handle(plugin, &(const struct irc_event) {
			.type = IRC_EVENT_COMMAND,
			.server = server,
			.message = {
				.message = "",
				.origin = "jean",
				.channel = "#test"
			}
		});

		if (strcmp(server->conn->out, "PRIVMSG #test :jean, NO\r\n") == 0)
			yes = 1;
		else if (strcmp(server->conn->out, "PRIVMSG #test :jean, YES\r\n") == 0)
			no = 1;

		memset(server->conn->out, 0, sizeof (server->conn->out));
	}

	GREATEST_ASSERT(no);
	GREATEST_ASSERT(yes);

	GREATEST_PASS();
}

GREATEST_SUITE(suite_basics)
{
	GREATEST_SET_SETUP_CB(setup, NULL);
	GREATEST_SET_TEARDOWN_CB(teardown, NULL);
	GREATEST_RUN_TEST(basics_simple);
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
