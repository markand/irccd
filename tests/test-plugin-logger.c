/*
 * test-plugin-logger.c -- test logger plugin
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

#include <errno.h>
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

static struct irc_server *server;
static struct irc_plugin *plugin;

static void
setup(void *udata)
{
	(void)udata;

	remove(TOP "/tests/log");

	server = irc_server_new("test");
	irc_server_set_ident(server, "test", "test", "test");
	irc_server_set_params(server, "127.0.0.1", 6667, 0);
	irc_server_connect(server);

	plugin = js_plugin_open("logger", TOP "/plugins/logger/logger.js");

	if (!plugin)
		irc_util_die("could not load plugin\n");

	irc_server_incref(server);
	irc_plugin_set_template(plugin, "join", "join=#{server}:#{channel}:#{origin}:#{nickname}");
	irc_plugin_set_template(plugin, "kick", "kick=#{server}:#{channel}:#{origin}:#{nickname}:#{target}:#{reason}");
	irc_plugin_set_template(plugin, "me", "me=#{server}:#{channel}:#{origin}:#{nickname}:#{message}");
	irc_plugin_set_template(plugin, "message", "message=#{server}:#{channel}:#{origin}:#{nickname}:#{message}");
	irc_plugin_set_template(plugin, "mode", "mode=#{server}:#{origin}:#{channel}:#{mode}:#{args}");
	irc_plugin_set_template(plugin, "notice", "notice=#{server}:#{origin}:#{channel}:#{message}");
	irc_plugin_set_template(plugin, "part", "part=#{server}:#{channel}:#{origin}:#{nickname}:#{reason}");
	irc_plugin_set_template(plugin, "query", "query=#{server}:#{origin}:#{nickname}:#{message}");
	irc_plugin_set_template(plugin, "topic", "topic=#{server}:#{channel}:#{origin}:#{nickname}:#{topic}");
	irc_plugin_set_option(plugin, "file", TOP "/tests/log");
	irc_plugin_load(plugin);
}

static void
teardown(void *udata)
{
	(void)udata;

	remove(TOP "/tests/log");

	irc_plugin_finish(plugin);
	irc_server_decref(server);
}

static const char *
last(void)
{
	static char buf[1024];
	FILE *fp;

	buf[0] = '\0';

	if (!(fp = fopen(TOP "/tests/log", "r")))
		irc_util_die("fopen: %s\n", strerror(errno));
	if (!(fgets(buf, sizeof (buf), fp)))
		irc_util_die("fgets: %s\n", strerror(errno));

	fclose(fp);

	buf[strcspn(buf, "\r\n")] = '\0';

	return buf;
}

GREATEST_TEST
basics_join(void)
{
	irc_plugin_handle(plugin, &(const struct irc_event) {
		.type = IRC_EVENT_JOIN,
		.server = server,
		.join = {
			.origin = "jean!jean@localhost",
			.channel = "#staff"
		}
	});

	GREATEST_ASSERT_STR_EQ("join=test:#staff:jean!jean@localhost:jean", last());
	GREATEST_PASS();
}

GREATEST_TEST
basics_kick(void)
{
	irc_plugin_handle(plugin, &(const struct irc_event) {
		.type = IRC_EVENT_KICK,
		.server = server,
		.kick = {
			.origin = "jean!jean@localhost",
			.channel = "#staff",
			.target = "badboy",
			.reason = "please do not flood"
		}
	});

	GREATEST_ASSERT_STR_EQ("kick=test:#staff:jean!jean@localhost:jean:badboy:please do not flood", last());
	GREATEST_PASS();
}

GREATEST_TEST
basics_me(void)
{
	irc_plugin_handle(plugin, &(const struct irc_event) {
		.type = IRC_EVENT_ME,
		.server = server,
		.message = {
			.origin = "jean!jean@localhost",
			.channel = "#staff",
			.message = "is drinking water"
		}
	});

	GREATEST_ASSERT_STR_EQ("me=test:#staff:jean!jean@localhost:jean:is drinking water", last());
	GREATEST_PASS();
}

GREATEST_TEST
basics_message(void)
{
	irc_plugin_handle(plugin, &(const struct irc_event) {
		.type = IRC_EVENT_MESSAGE,
		.server = server,
		.message = {
			.origin = "jean!jean@localhost",
			.channel = "#staff",
			.message = "hello guys"
		}
	});

	GREATEST_ASSERT_STR_EQ("message=test:#staff:jean!jean@localhost:jean:hello guys", last());
	GREATEST_PASS();
}

GREATEST_TEST
basics_mode(void)
{
	irc_plugin_handle(plugin, &(const struct irc_event) {
		.type = IRC_EVENT_MODE,
		.server = server,
		.mode = {
			.origin = "jean!jean@localhost",
			.channel = "#staff",
			.mode = "+ov",
			.args = (char *[]) { "francis", "benoit", NULL }
		}
	});

	GREATEST_ASSERT_STR_EQ("mode=test:jean!jean@localhost:#staff:+ov:francis benoit", last());
	GREATEST_PASS();
}

GREATEST_TEST
basics_notice(void)
{
	irc_plugin_handle(plugin, &(const struct irc_event) {
		.type = IRC_EVENT_NOTICE,
		.server = server,
		.notice = {
			.origin = "jean!jean@localhost",
			.channel = "chris",
			.notice = "tu veux voir mon chat ?"
		}
	});

	GREATEST_ASSERT_STR_EQ("notice=test:jean!jean@localhost:chris:tu veux voir mon chat ?", last());
	GREATEST_PASS();
}

GREATEST_TEST
basics_part(void)
{
	irc_plugin_handle(plugin, &(const struct irc_event) {
		.type = IRC_EVENT_PART,
		.server = server,
		.part = {
			.origin = "jean!jean@localhost",
			.channel = "#staff",
			.reason = "too noisy here"
		}
	});

	GREATEST_ASSERT_STR_EQ("part=test:#staff:jean!jean@localhost:jean:too noisy here", last());
	GREATEST_PASS();
}

GREATEST_TEST
basics_topic(void)
{
	irc_plugin_handle(plugin, &(const struct irc_event) {
		.type = IRC_EVENT_TOPIC,
		.server = server,
		.topic = {
			.origin = "jean!jean@localhost",
			.channel = "#staff",
			.topic = "oh yeah yeaaaaaaaah"
		}
	});

	GREATEST_ASSERT_STR_EQ("topic=test:#staff:jean!jean@localhost:jean:oh yeah yeaaaaaaaah", last());
	GREATEST_PASS();
}

GREATEST_TEST
basics_case_insensitive(void)
{
	irc_plugin_handle(plugin, &(const struct irc_event) {
		.type = IRC_EVENT_MESSAGE,
		.server = server,
		.message = {
			.origin = "jean!jean@localhost",
			.channel = "#STAFF",
			.message = "hello guys"
		}
	});

	GREATEST_ASSERT_STR_EQ("message=test:#staff:jean!jean@localhost:jean:hello guys", last());
	GREATEST_PASS();
}

GREATEST_SUITE(suite_basics)
{
	GREATEST_SET_SETUP_CB(setup, NULL);
	GREATEST_SET_TEARDOWN_CB(teardown, NULL);
	GREATEST_RUN_TEST(basics_join);
	GREATEST_RUN_TEST(basics_kick);
	GREATEST_RUN_TEST(basics_me);
	GREATEST_RUN_TEST(basics_message);
	GREATEST_RUN_TEST(basics_mode);
	GREATEST_RUN_TEST(basics_notice);
	GREATEST_RUN_TEST(basics_part);
	GREATEST_RUN_TEST(basics_topic);
	GREATEST_RUN_TEST(basics_case_insensitive);
}

GREATEST_MAIN_DEFS();

int
main(int argc, char **argv)
{
	irc_bot_init(NULL);
	irc_log_to_null();

	GREATEST_MAIN_BEGIN();
	GREATEST_RUN_SUITE(suite_basics);
	GREATEST_MAIN_END();

	return 0;
}
