/*
 * test-dl-plugin.c -- test dl-plugin.h functions
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

/*
 * Test code below
 * ------------------------------------------------------------------
 */

#include <stdlib.h>
#include <string.h>

#define GREATEST_USE_ABBREVS 0
#include <greatest.h>

#include <irccd/dl-plugin.h>
#include <irccd/event.h>
#include <irccd/plugin.h>
#include <irccd/server.h>
#include <irccd/util.h>

static struct irc_plugin *plugin;

static void
test_setup(void *udata)
{
	(void)udata;

	/* TODO: No idea how to stop greatest from here. */
	if ((plugin = dl_plugin_open("plugin-dl-example", NULL)) == NULL)
		exit(1);
}

static void
test_teardown(void *udata)
{
	(void)udata;

	irc_plugin_finish(plugin);
}

GREATEST_TEST
test_options_set_get(void)
{
	irc_plugin_set_option(plugin, "option-1", "new-value-1");
	GREATEST_ASSERT_STR_EQ("new-value-1", irc_plugin_get_option(plugin, "option-1"));
	GREATEST_ASSERT(!irc_plugin_get_option(plugin, "not-found"));
	GREATEST_PASS();
}

GREATEST_TEST
test_options_list(void)
{
	const char * const *options = irc_plugin_get_options(plugin);

	GREATEST_ASSERT_STR_EQ("option-1", options[0]);
	GREATEST_ASSERT(!options[1]);
	GREATEST_PASS();
}

GREATEST_SUITE(suite_options)
{
	GREATEST_SET_SETUP_CB(test_setup, NULL);
	GREATEST_SET_TEARDOWN_CB(test_teardown, NULL);
	GREATEST_RUN_TEST(test_options_set_get);
	GREATEST_RUN_TEST(test_options_list);
}

GREATEST_TEST
test_paths_set_get(void)
{
	irc_plugin_set_path(plugin, "path-1", "new-value-1");
	GREATEST_ASSERT_STR_EQ("new-value-1", irc_plugin_get_path(plugin, "path-1"));
	GREATEST_ASSERT(!irc_plugin_get_path(plugin, "not-found"));
	GREATEST_PASS();
}

GREATEST_TEST
test_paths_list(void)
{
	const char * const *paths = irc_plugin_get_paths(plugin);

	GREATEST_ASSERT_STR_EQ("path-1", paths[0]);
	GREATEST_ASSERT(!paths[1]);
	GREATEST_PASS();
}

GREATEST_SUITE(suite_paths)
{
	GREATEST_SET_SETUP_CB(test_setup, NULL);
	GREATEST_SET_TEARDOWN_CB(test_teardown, NULL);
	GREATEST_RUN_TEST(test_paths_set_get);
	GREATEST_RUN_TEST(test_paths_list);
}

GREATEST_TEST
test_templates_set_get(void)
{
	irc_plugin_set_template(plugin, "template-1", "new-value-1");
	GREATEST_ASSERT_STR_EQ("new-value-1", irc_plugin_get_template(plugin, "template-1"));
	GREATEST_ASSERT(!irc_plugin_get_template(plugin, "not-found"));
	GREATEST_PASS();
}

GREATEST_TEST
test_templates_list(void)
{
	const char * const *templates = irc_plugin_get_templates(plugin);

	GREATEST_ASSERT_STR_EQ("template-1", templates[0]);
	GREATEST_ASSERT(!templates[1]);
	GREATEST_PASS();
}

GREATEST_SUITE(suite_templates)
{
	GREATEST_SET_SETUP_CB(test_setup, NULL);
	GREATEST_SET_TEARDOWN_CB(test_teardown, NULL);
	GREATEST_RUN_TEST(test_templates_set_get);
	GREATEST_RUN_TEST(test_templates_list);
}

GREATEST_TEST
test_calls_simple(void)
{
#if 0
	struct irc__conn conn = {0};
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

#endif
	GREATEST_PASS();
}

GREATEST_SUITE(suite_calls)
{
	GREATEST_SET_SETUP_CB(test_setup, NULL);
	GREATEST_SET_TEARDOWN_CB(test_teardown, NULL);
	GREATEST_RUN_TEST(test_calls_simple);
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

/*
 * Plugin embedded code below.
 * ------------------------------------------------------------------
 */

struct kw {
	const char *key;
	char value[256];
};

/*
 * Options.
 */
static struct kw options[] = {
	{ "option-1", "value-1" }
};

static const char *options_list[] = {
	"option-1",
	NULL
};

/*
 * Templates.
 */
static struct kw templates[] = {
	{ "template-1", "Welcome #{target}" }
};

static const char *templates_list[] = {
	"template-1",
	NULL
};

/*
 * Paths.
 */
static struct kw paths[] = {
	{ "path-1", "/usr/local/etc" }
};

static const char *paths_list[] = {
	"path-1",
	NULL
};

static void
set(struct kw *table, size_t tablesz, const char *key, const char *value)
{
	for (size_t i = 0; i < tablesz; ++i) {
		if (strcmp(table[i].key, key) == 0) {
			irc_util_strlcpy(table[i].value, value, sizeof (table[i].value));
			break;
		}
	}
}

static const char *
get(const struct kw *table, size_t tablesz, const char *key)
{
	for (size_t i = 0; i < tablesz; ++i)
		if (strcmp(table[i].key, key) == 0)
			return table[i].value;

	return NULL;
}

void
plugin_dl_example_set_option(const char *key, const char *value)
{
	set(options, IRC_UTIL_SIZE(options), key, value);
}

const char *
plugin_dl_example_get_option(const char *key)
{
	return get(options, IRC_UTIL_SIZE(options), key);
}

const char **
plugin_dl_example_get_options(void)
{
	return options_list;
}

void
plugin_dl_example_set_template(const char *key, const char *value)
{
	set(templates, IRC_UTIL_SIZE(templates), key, value);
}

const char *
plugin_dl_example_get_template(const char *key)
{
	return get(templates, IRC_UTIL_SIZE(templates), key);
}

const char **
plugin_dl_example_get_templates(void)
{
	return templates_list;
}

void
plugin_dl_example_set_path(const char *key, const char *value)
{
	set(paths, IRC_UTIL_SIZE(paths), key, value);
}

const char *
plugin_dl_example_get_path(const char *key)
{
	return get(paths, IRC_UTIL_SIZE(paths), key);
}

const char **
plugin_dl_example_get_paths(void)
{
	return paths_list;
}

void
plugin_dl_example_event(const struct irc_event *ev)
{
	irc_server_send(ev->server, "EVENT");
}

void
plugin_dl_example_load(void)
{
}

void
plugin_dl_example_reload(void)
{
}

void
plugin_dl_example_unload(void)
{
}
