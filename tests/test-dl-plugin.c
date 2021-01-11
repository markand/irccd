/*
 * test-dl-plugin.c -- test dl-plugin.h functions
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

#include <irccd/dl-plugin.h>
#include <irccd/event.h>
#include <irccd/plugin.h>

static struct irc_plugin plugin;

static void
setup(void *udata)
{
	(void)udata;

	/* TODO: No idea how to stop greatest from here. */
	if (!irc_dl_plugin_open(&plugin, SOURCEDIR "/tests/example-dl-plugin" IRC_DL_EXT))
		exit(1);
}

static void
teardown(void *udata)
{
	(void)udata;

	irc_plugin_finish(&plugin);
}

GREATEST_TEST
options_set_get(void)
{
	irc_plugin_set_option(&plugin, "option-1", "new-value-1");
	GREATEST_ASSERT_STR_EQ("new-value-1", irc_plugin_get_option(&plugin, "option-1"));
	GREATEST_ASSERT(!irc_plugin_get_option(&plugin, "not-found"));
	GREATEST_PASS();
}

GREATEST_TEST
options_list(void)
{
	const char **options = irc_plugin_get_options(&plugin);

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
	irc_plugin_set_path(&plugin, "path-1", "new-value-1");
	GREATEST_ASSERT_STR_EQ("new-value-1", irc_plugin_get_path(&plugin, "path-1"));
	GREATEST_ASSERT(!irc_plugin_get_path(&plugin, "not-found"));
	GREATEST_PASS();
}

GREATEST_TEST
paths_list(void)
{
	const char **paths = irc_plugin_get_paths(&plugin);

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
	irc_plugin_set_template(&plugin, "template-1", "new-value-1");
	GREATEST_ASSERT_STR_EQ("new-value-1", irc_plugin_get_template(&plugin, "template-1"));
	GREATEST_ASSERT(!irc_plugin_get_template(&plugin, "not-found"));
	GREATEST_PASS();
}

GREATEST_TEST
templates_list(void)
{
	const char **templates = irc_plugin_get_templates(&plugin);

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
	struct irc_event ev = {0};

	irc_plugin_load(&plugin);
	irc_plugin_unload(&plugin);
	irc_plugin_reload(&plugin);
	irc_plugin_handle(&plugin, &ev);

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
