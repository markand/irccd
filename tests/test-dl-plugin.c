/*
 * test-dl-plugin.c -- test dl-plugin.h functions
 *
 * Copyright (c) 2013-2022 David Demelier <markand@malikania.fr>
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

#include <irccd/conn.h>
#include <irccd/dl-plugin.h>
#include <irccd/event.h>
#include <irccd/plugin.h>
#include <irccd/server.h>
#include <irccd/util.h>

static struct irc_plugin *plugin;

static void
setup(void *udata)
{
	(void)udata;

	/* TODO: No idea how to stop greatest from here. */
	if ((plugin = dl_plugin_open("example", EXAMPLE_DL_PLUGIN)) == NULL)
		irc_util_die("dlopen: %s\n", strerror(errno));
}

static void
teardown(void *udata)
{
	(void)udata;

	irc_plugin_finish(plugin);
}

GREATEST_TEST
options_set_get(void)
{
	irc_plugin_set_option(plugin, "option-1", "new-value-1");
	GREATEST_ASSERT_STR_EQ("new-value-1", irc_plugin_get_option(plugin, "option-1"));
	GREATEST_ASSERT(!irc_plugin_get_option(plugin, "not-found"));
	GREATEST_PASS();
}

GREATEST_TEST
options_list(void)
{
	const char * const *options = irc_plugin_get_options(plugin);

	GREATEST_ASSERT_STR_EQ("option-1", options[0]);
	GREATEST_ASSERT(!options[1]);
	GREATEST_PASS();
}

GREATEST_SUITE(suite_options)
{
	GREATEST_SET_SETUP_CB(setup, NULL);
	GREATEST_SET_TEARDOWN_CB(teardown, NULL);
	GREATEST_RUN_TEST(options_set_get);
	GREATEST_RUN_TEST(options_list);
}

GREATEST_TEST
paths_set_get(void)
{
	irc_plugin_set_path(plugin, "path-1", "new-value-1");
	GREATEST_ASSERT_STR_EQ("new-value-1", irc_plugin_get_path(plugin, "path-1"));
	GREATEST_ASSERT(!irc_plugin_get_path(plugin, "not-found"));
	GREATEST_PASS();
}

GREATEST_TEST
paths_list(void)
{
	const char * const *paths = irc_plugin_get_paths(plugin);

	GREATEST_ASSERT_STR_EQ("path-1", paths[0]);
	GREATEST_ASSERT(!paths[1]);
	GREATEST_PASS();
}

GREATEST_SUITE(suite_paths)
{
	GREATEST_SET_SETUP_CB(setup, NULL);
	GREATEST_SET_TEARDOWN_CB(teardown, NULL);
	GREATEST_RUN_TEST(paths_set_get);
	GREATEST_RUN_TEST(paths_list);
}

GREATEST_TEST
templates_set_get(void)
{
	irc_plugin_set_template(plugin, "template-1", "new-value-1");
	GREATEST_ASSERT_STR_EQ("new-value-1", irc_plugin_get_template(plugin, "template-1"));
	GREATEST_ASSERT(!irc_plugin_get_template(plugin, "not-found"));
	GREATEST_PASS();
}

GREATEST_TEST
templates_list(void)
{
	const char * const *templates = irc_plugin_get_templates(plugin);

	GREATEST_ASSERT_STR_EQ("template-1", templates[0]);
	GREATEST_ASSERT(!templates[1]);
	GREATEST_PASS();
}

GREATEST_SUITE(suite_templates)
{
	GREATEST_SET_SETUP_CB(setup, NULL);
	GREATEST_SET_TEARDOWN_CB(teardown, NULL);
	GREATEST_RUN_TEST(templates_set_get);
	GREATEST_RUN_TEST(templates_list);
}

GREATEST_TEST
calls_simple(void)
{
	struct irc_conn conn = {0};
	struct irc_server server = {
		.state = IRC_SERVER_STATE_CONNECTED,
		.conn = &conn
	};
	struct irc_event ev = {
		.server = &server
	};

	irc_plugin_load(plugin);
	irc_plugin_unload(plugin);
	irc_plugin_reload(plugin);
	irc_plugin_handle(plugin, &ev);
	GREATEST_ASSERT_STR_EQ("EVENT\r\n", server.conn->out);

	GREATEST_PASS();
}

GREATEST_SUITE(suite_calls)
{
	GREATEST_SET_SETUP_CB(setup, NULL);
	GREATEST_SET_TEARDOWN_CB(teardown, NULL);
	GREATEST_RUN_TEST(calls_simple);
}

GREATEST_MAIN_DEFS();

int
main(int argc, char **argv)
{
	GREATEST_MAIN_BEGIN();
	GREATEST_RUN_SUITE(suite_options);
	GREATEST_RUN_SUITE(suite_paths);
	GREATEST_RUN_SUITE(suite_templates);
	GREATEST_RUN_SUITE(suite_calls);
	GREATEST_MAIN_END();

	return 0;
}
