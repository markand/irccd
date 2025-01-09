/*
 * plugin.h -- abstract plugin
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

#ifndef IRCCD_PLUGIN_H
#define IRCCD_PLUGIN_H

#include "limits.h"

#if defined(__cplusplus)
extern "C" {
#endif

struct irc_event;

struct irc_plugin {
	char name[IRC_ID_LEN];
	const char *license;
	const char *version;
	const char *author;
	const char *description;
	void *data;
	struct irc_plugin *next;

	void (*set_template)(struct irc_plugin *, const char *, const char *);
	const char *(*get_template)(struct irc_plugin *, const char *);
	const char * const *(*get_templates)(struct irc_plugin *);

	void (*set_path)(struct irc_plugin *, const char *, const char *);
	const char *(*get_path)(struct irc_plugin *, const char *);
	const char * const *(*get_paths)(struct irc_plugin *);

	void (*set_option)(struct irc_plugin *, const char *, const char *);
	const char *(*get_option)(struct irc_plugin *, const char *);
	const char * const *(*get_options)(struct irc_plugin *);

	int (*load)(struct irc_plugin *);
	void (*reload)(struct irc_plugin *);
	void (*unload)(struct irc_plugin *);
	void (*handle)(struct irc_plugin *, const struct irc_event *);

	void (*finish)(struct irc_plugin *);
};

struct irc_plugin_loader {
	char paths[IRC_PATHS_LEN];
	char extensions[IRC_EXTENSIONS_LEN];
	struct irc_plugin *(*open)(struct irc_plugin_loader *, const char *, const char *);
	void (*finish)(struct irc_plugin_loader *);
	void *data;
	struct irc_plugin_loader *next;
};

void
irc_plugin_set_template(struct irc_plugin *, const char *, const char *);

const char *
irc_plugin_get_template(struct irc_plugin *, const char *);

const char * const *
irc_plugin_get_templates(struct irc_plugin *);

void
irc_plugin_set_path(struct irc_plugin *, const char *, const char *);

const char *
irc_plugin_get_path(struct irc_plugin *, const char *);

const char * const *
irc_plugin_get_paths(struct irc_plugin *);

void
irc_plugin_set_option(struct irc_plugin *, const char *, const char *);

const char *
irc_plugin_get_option(struct irc_plugin *, const char *);

const char * const *
irc_plugin_get_options(struct irc_plugin *);

int
irc_plugin_load(struct irc_plugin *);

void
irc_plugin_reload(struct irc_plugin *);

void
irc_plugin_unload(struct irc_plugin *);

void
irc_plugin_handle(struct irc_plugin *, const struct irc_event *);

void
irc_plugin_finish(struct irc_plugin *);

struct irc_plugin *
irc_plugin_loader_open(struct irc_plugin_loader *, const char *, const char *);

void
irc_plugin_loader_finish(struct irc_plugin_loader *);

#if defined(__cplusplus)
}
#endif

#endif /* !IRCCD_PLUGIN_H */
