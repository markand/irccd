/*
 * plugin.h -- abstract plugin
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

#ifndef IRCCD_PLUGIN_H
#define IRCCD_PLUGIN_H

#include <stdbool.h>

struct irc_event;

struct irc_plugin {
	const char *name;
	const char *license;
	const char *version;
	const char *author;
	const char *description;
	void *data;

	void (*set_template)(struct irc_plugin *, const char *, const char *);
	const char *(*get_template)(struct irc_plugin *, const char *);
	const char **(*get_templates)(struct irc_plugin *);

	void (*set_path)(struct irc_plugin *, const char *, const char *);
	const char *(*get_path)(struct irc_plugin *, const char *);
	const char **(*get_paths)(struct irc_plugin *);

	void (*set_option)(struct irc_plugin *, const char *, const char *);
	const char *(*get_option)(struct irc_plugin *, const char *);
	const char **(*get_options)(struct irc_plugin *);

	void (*load)(struct irc_plugin *);
	void (*reload)(struct irc_plugin *);
	void (*unload)(struct irc_plugin *);
	void (*handle)(struct irc_plugin *, const struct irc_event *);

	void (*finish)(struct irc_plugin *);
};

void
irc_plugin_set_template(struct irc_plugin *, const char *, const char *);

const char *
irc_plugin_get_template(struct irc_plugin *, const char *);

const char **
irc_plugin_get_templates(struct irc_plugin *);

void
irc_plugin_set_path(struct irc_plugin *, const char *, const char *);

const char *
irc_plugin_get_path(struct irc_plugin *, const char *);

const char **
irc_plugin_get_paths(struct irc_plugin *);

void
irc_plugin_set_option(struct irc_plugin *, const char *, const char *);

const char *
irc_plugin_get_option(struct irc_plugin *, const char *);

const char **
irc_plugin_get_options(struct irc_plugin *);

void
irc_plugin_load(struct irc_plugin *);

void
irc_plugin_reload(struct irc_plugin *);

void
irc_plugin_unload(struct irc_plugin *);

void
irc_plugin_handle(struct irc_plugin *, const struct irc_event *);

void
irc_plugin_finish(struct irc_plugin *);

#endif /* !IRCCD_PLUGIN_H */
