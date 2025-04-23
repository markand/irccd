/*
 * test-plugin-auth.c -- test auth plugin
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

#include <irccd/conn.h>
#include <irccd/js-plugin.h>
#include <irccd/plugin.h>
#include <irccd/server.h>
#include <irccd/util.h>

/*
 * 0 -> nickserv without nickname
 * 1 -> nickserv with nickname
 * 2 -> quakenet
 */
static struct irc_server *servers[3];
static struct irc_plugin *plugin;

static void
setup(void *udata)
{
	(void)udata;

	servers[0] = irc_server_new("nickserv1", "t", "t", "t", "127.0.0.1", 6667);
	servers[1] = irc_server_new("nickserv2", "t", "t", "t", "127.0.0.1", 6667);
	servers[2] = irc_server_new("quakenet", "t", "t", "t", "127.0.0.1", 6667);
	plugin = js_plugin_open("test", TOP "/plugins/auth/auth.js");

	if (!plugin)
		irc_util_die("could not load plugin\n");

	irc_server_incref(servers[0]);
	irc_server_incref(servers[1]);
	irc_server_incref(servers[2]);
	irc_plugin_set_option(plugin, "nickserv1.type", "nickserv");
	irc_plugin_set_option(plugin, "nickserv1.password", "plopation");
	irc_plugin_set_option(plugin, "nickserv2.type", "nickserv");
	irc_plugin_set_option(plugin, "nickserv2.password", "something");
	irc_plugin_set_option(plugin, "nickserv2.username", "jean");
	irc_plugin_set_option(plugin, "quakenet.type", "quakenet");
	irc_plugin_set_option(plugin, "quakenet.password", "hello");
	irc_plugin_set_option(plugin, "quakenet.username", "mario");
	irc_plugin_load(plugin);

	/* Fake server connected to send data. */
	servers[0]->state = servers[1]->state = servers[2]->state = IRC_SERVER_STATE_CONNECTED;
}

static void
teardown(void *udata)
{
	(void)udata;

	irc_plugin_finish(plugin);
	irc_server_decref(servers[0]);
	irc_server_decref(servers[1]);
	irc_server_decref(servers[2]);
}

GREATEST_TEST
basics_nickserv1(void)
{
	irc_plugin_handle(plugin, &(const struct irc_event) {
		.type = IRC_EVENT_CONNECT,
		.server = servers[0]
	});

	GREATEST_ASSERT_STR_EQ("PRIVMSG NickServ :identify plopation\r\n", servers[0]->conn->out);
	GREATEST_PASS();
}

GREATEST_TEST
basics_nickserv2(void)
{
	irc_plugin_handle(plugin, &(const struct irc_event) {
		.type = IRC_EVENT_CONNECT,
		.server = servers[1]
	});

	GREATEST_ASSERT_STR_EQ("PRIVMSG NickServ :identify jean something\r\n", servers[1]->conn->out);
	GREATEST_PASS();
}

GREATEST_TEST
basics_quakenet(void)
{
	irc_plugin_handle(plugin, &(const struct irc_event) {
		.type = IRC_EVENT_CONNECT,
		.server = servers[2]
	});

	GREATEST_ASSERT_STR_EQ("PRIVMSG Q@CServe.quakenet.org :AUTH mario hello\r\n", servers[2]->conn->out);
	GREATEST_PASS();
}

GREATEST_SUITE(suite_basics)
{
	GREATEST_SET_SETUP_CB(setup, NULL);
	GREATEST_SET_TEARDOWN_CB(teardown, NULL);
	GREATEST_RUN_TEST(basics_nickserv1);
	GREATEST_RUN_TEST(basics_nickserv2);
	GREATEST_RUN_TEST(basics_quakenet);
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
