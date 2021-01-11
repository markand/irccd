/*
 * dl-plugin.c -- native C plugins for irccd
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

#include <assert.h>
#include <ctype.h>
#include <dlfcn.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "dl-plugin.h"
#include "log.h"
#include "plugin.h"
#include "util.h"

#define INVOKE(pg, name, sig, ...)                                              \
do {                                                                            \
        struct self *self = pg->data;                                           \
        sig fn;                                                                 \
                                                                                \
        if (self->handle && (fn = dlsym(self->handle, symbol(self, name))))     \
                return fn(__VA_ARGS__);                                         \
} while (0)

struct self {
	char prefix[32];
	void *handle;
};

typedef const char *    (*get_option_fn)(const char *);
typedef const char *    (*get_path_fn)(const char *);
typedef const char *    (*get_template_fn)(const char *);
typedef const char **   (*get_options_fn)(void);
typedef const char **   (*get_paths_fn)(void);
typedef const char **   (*get_templates_fn)(void);
typedef void            (*event_fn)(const struct irc_event *);
typedef void            (*load_fn)(void);
typedef void            (*reload_fn)(void);
typedef void            (*unload_fn)(void);
typedef void            (*set_option_fn)(const char *, const char *);
typedef void            (*set_path_fn)(const char *, const char *);
typedef void            (*set_template_fn)(const char *, const char *);

static const char *
symbol(const struct self *self, const char *func)
{
	static char sym[128];

	snprintf(sym, sizeof (sym), "%s_%s",self->prefix, func);

	return sym;
}

static void
set_template(struct irc_plugin *plg, const char *key, const char *value)
{
	INVOKE(plg, "set_template", set_template_fn, key, value);
}

static const char *
get_template(struct irc_plugin *plg, const char *key)
{
	INVOKE(plg, "get_template", get_template_fn, key);

	return NULL;
}

static const char **
get_templates(struct irc_plugin *plg)
{
	INVOKE(plg, "get_templates", get_templates_fn);

	return NULL;
}

static void
set_path(struct irc_plugin *plg, const char *key, const char *value)
{
	INVOKE(plg, "set_path", set_path_fn, key, value);
}

static const char *
get_path(struct irc_plugin *plg, const char *key)
{
	INVOKE(plg, "get_path", get_path_fn, key);

	return NULL;
}

static const char **
get_paths(struct irc_plugin *plg)
{
	INVOKE(plg, "get_paths", get_paths_fn);

	return NULL;
}

static void
set_option(struct irc_plugin *plg, const char *key, const char *value)
{
	INVOKE(plg, "set_option", set_option_fn, key, value);
}

static const char *
get_option(struct irc_plugin *plg, const char *key)
{
	INVOKE(plg, "get_option", get_option_fn, key);

	return NULL;
}

static const char **
get_options(struct irc_plugin *plg)
{
	INVOKE(plg, "get_options", get_options_fn);

	return NULL;
}

static void
load(struct irc_plugin *plg)
{
	INVOKE(plg, "load", load_fn);
}

static void
reload(struct irc_plugin *plg)
{
	INVOKE(plg, "reload", reload_fn);
}

static void
unload(struct irc_plugin *plg)
{
	INVOKE(plg, "unload", unload_fn);
}

static void
handle(struct irc_plugin *plg, const struct irc_event *ev)
{
	INVOKE(plg, "event", event_fn, ev);
}

static void
finish(struct irc_plugin *plg)
{
	struct self *self = plg->data;

	if (self->handle)
		dlclose(self->handle);

	memset(self, 0, sizeof (*self));
}

static bool
init(struct self *self, const char *path)
{
	memset(self, 0, sizeof (*self));

	if (!(self->handle = dlopen(path, RTLD_NOW))) {
		irc_log_warn("plugin: %s: %s", strerror(errno));
		return false;
	}

	/* Compute prefix name */
	strlcpy(self->prefix, irc_util_basename(path), sizeof (self->prefix));

	/* Remove plugin extension. */
	self->prefix[strcspn(self->prefix, ".")] = '\0';

	/* Remove every invalid identifiers. */
	for (char *p = self->prefix; *p; ++p)
		if (!isalnum(*p))
			*p = '_';

	return true;
}

bool
irc_dl_plugin_open(struct irc_plugin *plg, const char *path)
{
	struct self self;

	if (!init(&self, path))
		return false;

	/* Data and all callbacks. */
	plg->data = irc_util_memdup(&self, sizeof (self));
	plg->set_template = set_template;
	plg->get_template = get_template;
	plg->get_templates = get_templates;
	plg->set_path = set_path;
	plg->get_path = get_path;
	plg->get_paths = get_paths;
	plg->set_option = set_option;
	plg->get_option = get_option;
	plg->get_options = get_options;
	plg->load = load;
	plg->reload = reload;
	plg->unload = unload;
	plg->handle = handle;
	plg->finish = finish;

	return true;
}
