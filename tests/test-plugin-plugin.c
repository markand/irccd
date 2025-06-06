/*
 * test-plugin-plugin.c -- test plugin plugin
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

#include <string.h>

#define GREATEST_USE_ABBREVS 0
#include <greatest.h>

#include "mock/server.c"

#include <irccd/irccd.h>
#include <irccd/js-plugin.h>
#include <irccd/log.h>
#include <irccd/plugin.h>
#include <irccd/server.h>
#include <irccd/util.h>

#define CALL(t, m) do {                                                 \
        irc_plugin_handle(plugin, &(const struct irc_event) {           \
                .type = t,                                              \
                .server = server,                                       \
                        .message = {                                    \
                        .origin = "jean!jean@localhost",                \
                        .channel = "#plugin",                           \
                        .message = m                                    \
                }                                                       \
        });                                                             \
} while (0)

static struct irc_server *server;
static struct mock_server *mock;
static struct irc_plugin *plugin, *fake;

static struct irc_plugin *
fake_new(int n)
{
	struct irc_plugin *p;
	char name[32] = {};

	p = irc_util_calloc(1, sizeof (*p));
	irc_plugin_init(p, irc_util_printf(name, sizeof (name), "plugin-n-%d", n));

	return p;
}

static void
setup(void *udata)
{
	(void)udata;

	server = irc_server_new("test");
	mock = IRC_UTIL_CONTAINER_OF(server, struct mock_server, parent);
	plugin = js_plugin_open("plugin", TOP "/plugins/plugin/plugin.js");

	if (!plugin)
		irc_util_die("could not load plugin\n");

	/* Prepare a fake plugin. */
	fake = irc_util_calloc(1, sizeof (*fake));
	irc_plugin_init(fake, "fake");
	irc_plugin_set_info(fake, "BEER", "0.0.0.0.0.0.1", "David", "Fake White Beer 2000");

	irc_bot_init(NULL);
	irc_bot_plugin_add(fake);
	irc_plugin_set_template(plugin, "usage", "usage=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}");
	irc_plugin_set_template(plugin, "info", "info=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}:#{author}:#{license}:#{name}:#{summary}:#{version}");
	irc_plugin_set_template(plugin, "not-found", "not-found=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}:#{name}");
	irc_plugin_set_template(plugin, "too-long", "too-long=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}");
	irc_server_incref(server);
	irc_plugin_load(plugin);
}

static void
teardown(void *udata)
{
	(void)udata;

	irc_bot_finish();
	irc_plugin_finish(plugin);
	irc_server_decref(server);
}

GREATEST_TEST
basics_usage(void)
{
	CALL(IRC_EVENT_COMMAND, "");
	GREATEST_ASSERT_STR_EQ("message #plugin usage=plugin:!plugin:test:#plugin:jean!jean@localhost:jean", mock->out->line);

	CALL(IRC_EVENT_COMMAND, "fail");
	GREATEST_ASSERT_STR_EQ("message #plugin usage=plugin:!plugin:test:#plugin:jean!jean@localhost:jean", mock->out->line);

	CALL(IRC_EVENT_COMMAND, "info");
	GREATEST_ASSERT_STR_EQ("message #plugin usage=plugin:!plugin:test:#plugin:jean!jean@localhost:jean", mock->out->line);

	GREATEST_PASS();
}

GREATEST_TEST
basics_info(void)
{
	CALL(IRC_EVENT_COMMAND, "info fake");
	GREATEST_ASSERT_STR_EQ("message #plugin info=plugin:!plugin:test:#plugin:jean!jean@localhost:jean:David:BEER:fake:Fake White Beer 2000:0.0.0.0.0.0.1", mock->out->line);

	GREATEST_PASS();
}

GREATEST_TEST
basics_not_found(void)
{
	CALL(IRC_EVENT_COMMAND, "info doesnotexist");
	GREATEST_ASSERT_STR_EQ("message #plugin not-found=plugin:!plugin:test:#plugin:jean!jean@localhost:jean:doesnotexist", mock->out->line);

	GREATEST_PASS();
}

GREATEST_TEST
basics_too_long(void)
{
	for (int i = 0; i < 100; ++i)
		irc_bot_plugin_add(fake_new(i));

	CALL(IRC_EVENT_COMMAND, "list");
	GREATEST_ASSERT_STR_EQ("message #plugin too-long=plugin:!plugin:test:#plugin:jean!jean@localhost:jean", mock->out->line);

	GREATEST_PASS();
}

GREATEST_SUITE(suite_basics)
{
	GREATEST_SET_SETUP_CB(setup, NULL);
	GREATEST_SET_TEARDOWN_CB(teardown, NULL);
	GREATEST_RUN_TEST(basics_usage);
	GREATEST_RUN_TEST(basics_info);
	GREATEST_RUN_TEST(basics_not_found);
	GREATEST_RUN_TEST(basics_too_long);
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
