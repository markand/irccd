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

#include <unity.h>

#include <irccd/js-plugin.h>
#include <irccd/plugin.h>

#include "mock/server.h"

/*
 * 0 -> nickserv without nickname
 * 1 -> nickserv with nickname
 * 2 -> quakenet
 */
static struct {
	struct irc_server *server;
	struct mock_server *mock;
} servers[3];

static struct irc_plugin *plugin;

void
setUp(void)
{
	servers[0].server = irc_server_new("nickserv1");
	servers[0].mock   = IRC_UTIL_CONTAINER_OF(servers[0].server, struct mock_server, parent);
	servers[1].server = irc_server_new("nickserv2");
	servers[1].mock   = IRC_UTIL_CONTAINER_OF(servers[1].server, struct mock_server, parent);
	servers[2].server = irc_server_new("quakenet");
	servers[2].mock   = IRC_UTIL_CONTAINER_OF(servers[2].server, struct mock_server, parent);

	plugin = js_plugin_open("test", TOP "/plugins/auth/auth.js");

	if (!plugin)
		irc_util_die("could not load plugin\n");

	irc_server_incref(servers[0].server);
	irc_server_incref(servers[1].server);
	irc_server_incref(servers[2].server);

	irc_plugin_set_option(plugin, "nickserv1.type", "nickserv");
	irc_plugin_set_option(plugin, "nickserv1.password", "plopation");
	irc_plugin_set_option(plugin, "nickserv2.type", "nickserv");
	irc_plugin_set_option(plugin, "nickserv2.password", "something");
	irc_plugin_set_option(plugin, "nickserv2.username", "jean");
	irc_plugin_set_option(plugin, "quakenet.type", "quakenet");
	irc_plugin_set_option(plugin, "quakenet.password", "hello");
	irc_plugin_set_option(plugin, "quakenet.username", "mario");
	irc_plugin_load(plugin);
}

void
tearDown(void)
{
	irc_plugin_finish(plugin);

	irc_server_decref(servers[0].server);
	irc_server_decref(servers[1].server);
	irc_server_decref(servers[2].server);
}

static void
basics_nickserv1(void)
{
	irc_plugin_handle(plugin, &(const struct irc_event) {
		.type = IRC_EVENT_CONNECT,
		.server = servers[0].server
	});

	TEST_ASSERT_EQUAL_STRING("message NickServ identify plopation", servers[0].mock->out->line);
}

static void
basics_nickserv2(void)
{
	irc_plugin_handle(plugin, &(const struct irc_event) {
		.type = IRC_EVENT_CONNECT,
		.server = servers[1].server
	});

	TEST_ASSERT_EQUAL_STRING("message NickServ identify jean something", servers[1].mock->out->line);
}

static void
basics_quakenet(void)
{
	irc_plugin_handle(plugin, &(const struct irc_event) {
		.type = IRC_EVENT_CONNECT,
		.server = servers[2].server
	});

	TEST_ASSERT_EQUAL_STRING("message Q@CServe.quakenet.org AUTH mario hello", servers[2].mock->out->line);
}

int
main(void)
{
	UNITY_BEGIN();

	RUN_TEST(basics_nickserv1);
	RUN_TEST(basics_nickserv2);
	RUN_TEST(basics_quakenet);

	return UNITY_END();
}
