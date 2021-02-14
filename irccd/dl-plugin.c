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

#include <sys/stat.h>
#include <assert.h>
#include <ctype.h>
#include <dlfcn.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <irccd/compat.h>
#include <irccd/config.h>
#include <irccd/log.h>
#include <irccd/plugin.h>
#include <irccd/util.h>

#include "dl-plugin.h"

#define INVOKE(pg, name, sig, ...)                                              \
do {                                                                            \
        struct self *self = pg->data;                                           \
        sig fn;                                                                 \
                                                                                \
        if (self->handle && (fn = dlsym(self->handle, symbol(self, name))))     \
                return fn(__VA_ARGS__);                                         \
} while (0)

struct self {
	struct irc_plugin plugin;
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

static inline const char *
symbol(const struct self *self, const char *func)
{
	static char sym[128];

	snprintf(sym, sizeof (sym), "%s_%s", self->prefix, func);

	return sym;
}

static inline const char *
metadata(struct self *self, const char *name)
{
	char **addr;

	if (!(addr = dlsym(self->handle, symbol(self, name))))
		return NULL;

	return *addr;
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

	free(self);
}

static struct self *
init(const char *name, const char *path)
{
	struct self self;
	struct stat st;

	memset(&self, 0, sizeof (self));
	strlcpy(self.plugin.name, name, sizeof (self.plugin.name));

	/*
	 * It's not possible to get the exact error code when loading a plugin
	 * using dlopen, since we're trying a lot of files that potentially not
	 * exist we check presence before even though there's a possible
	 * condition but at least we can print an error message if there are
	 * other errors than missing file.
	 */
	if (stat(path, &st) < 0 && errno == ENOENT)
		return NULL;

	if (!(self.handle = dlopen(path, RTLD_NOW))) {
		irc_log_warn("plugin: %s: %s", path, dlerror());
		return NULL;
	}

	/* Compute prefix name */
	strlcpy(self.prefix, irc_util_basename(path), sizeof (self.prefix));

	/* Remove plugin extension. */
	self.prefix[strcspn(self.prefix, ".")] = '\0';

	/* Remove every invalid identifiers. */
	for (char *p = self.prefix; *p; ++p)
		if (!isalnum(*p))
			*p = '_';

	return irc_util_memdup(&self, sizeof (self));
}

static struct irc_plugin *
wrap_open(struct irc_plugin_loader *ldr, const char *name, const char *path)
{
	(void)ldr;

	return dl_plugin_open(name, path);
}

struct irc_plugin *
dl_plugin_open(const char *name, const char *path)
{
	struct self *self;

	if (!(self = init(name, path)))
		return NULL;

	/* Data and all callbacks. */
	self->plugin.data = self;
	self->plugin.set_template = set_template;
	self->plugin.get_template = get_template;
	self->plugin.get_templates = get_templates;
	self->plugin.set_path = set_path;
	self->plugin.get_path = get_path;
	self->plugin.get_paths = get_paths;
	self->plugin.set_option = set_option;
	self->plugin.get_option = get_option;
	self->plugin.get_options = get_options;
	self->plugin.load = load;
	self->plugin.reload = reload;
	self->plugin.unload = unload;
	self->plugin.handle = handle;
	self->plugin.finish = finish;

	/* Metadata variables. */
	self->plugin.author = metadata(self, "author");
	self->plugin.description = metadata(self, "description");
	self->plugin.version = metadata(self, "version");
	self->plugin.license = metadata(self, "license");

	return &self->plugin;
}

struct irc_plugin_loader *
dl_plugin_loader_new(void)
{
	struct irc_plugin_loader *ldr;

	ldr = irc_util_calloc(1, sizeof (*ldr));
	ldr->open = wrap_open;

#if defined(_WIN32)
	strlcpy(ldr->extensions, "dll", sizeof (ldr->extensions));
#elif defined(__APPLE__)
	strlcpy(ldr->extensions, "so:dylib", sizeof (ldr->extensions));
#else
	strlcpy(ldr->extensions, "so", sizeof (ldr->extensions));
#endif

	strlcpy(ldr->paths, IRCCD_LIBDIR "/irccd", sizeof (ldr->paths));

	return ldr;
}
