/*
 * example-dl-plugin.c -- simple plugin for unit tests
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

#include <string.h>

#include <irccd/dl-plugin.h>
#include <irccd/event.h>
#include <irccd/util.h>

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
			strlcpy(table[i].value, value, sizeof (table[i].value));
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

IRC_DL_EXPORT void
example_dl_plugin_set_option(const char *key, const char *value)
{
	set(options, IRC_UTIL_SIZE(options), key, value);
}

IRC_DL_EXPORT const char *
example_dl_plugin_get_option(const char *key)
{
	return get(options, IRC_UTIL_SIZE(options), key);
}

IRC_DL_EXPORT const char **
example_dl_plugin_get_options(void)
{
	return options_list;
}

IRC_DL_EXPORT void
example_dl_plugin_set_template(const char *key, const char *value)
{
	set(templates, IRC_UTIL_SIZE(templates), key, value);
}

IRC_DL_EXPORT const char *
example_dl_plugin_get_template(const char *key)
{
	return get(templates, IRC_UTIL_SIZE(templates), key);
}

IRC_DL_EXPORT const char **
example_dl_plugin_get_templates(void)
{
	return templates_list;
}

IRC_DL_EXPORT void
example_dl_plugin_set_path(const char *key, const char *value)
{
	set(paths, IRC_UTIL_SIZE(paths), key, value);
}

IRC_DL_EXPORT const char *
example_dl_plugin_get_path(const char *key)
{
	return get(paths, IRC_UTIL_SIZE(paths), key);
}

IRC_DL_EXPORT const char **
example_dl_plugin_get_paths(void)
{
	return paths_list;
}

IRC_DL_EXPORT void
example_dl_plugin_event(const struct irc_event *ev)
{
	(void)ev;
}

IRC_DL_EXPORT void
example_dl_plugin_load(void)
{
}

IRC_DL_EXPORT void
example_dl_plugin_reload(void)
{
}

IRC_DL_EXPORT void
example_dl_plugin_unload(void)
{
}