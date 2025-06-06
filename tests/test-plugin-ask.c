/*
 * test-plugin-ask.c -- test ask plugin
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

static struct irc_server *server;
static struct irc_plugin *plugin;

static struct mock_server *mock;

static void
setup(void *udata)
{
	(void)udata;

	server = irc_server_new("test");
	mock = IRC_UTIL_CONTAINER_OF(server, struct mock_server, parent);

	plugin = js_plugin_open("test", TOP "/plugins/ask/ask.js");

	if (!plugin)
		irc_util_die("could not load plugin\n");

	irc_server_incref(server);
	irc_plugin_set_option(plugin, "file", TOP "/tests/data/answers.conf");
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

		if (strcmp(mock->out->line, "message #test jean, NO") == 0)
			yes = 1;
		else if (strcmp(mock->out->line, "message #test jean, YES") == 0)
			no = 1;
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
