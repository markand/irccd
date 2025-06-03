/*
 * dl-plugin.c -- native C plugins for irccd
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

#include <sys/stat.h>
#include <assert.h>
#include <ctype.h>
#include <dlfcn.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <irccd/config.h>
#include <irccd/log.h>
#include <irccd/plugin.h>
#include <irccd/util.h>

#include "dl-plugin.h"

#define LDR_PATHS IRCCD_LIBDIR "/irccd"

#if defined(_WIN32)
#       define LDR_EXTENSIONS "dll"
#elif defined(__APPLE__)
#       define LDR_EXTENSIONS "so:dylib"
#else
#       define LDR_EXTENSIONS "so"
#endif

#if defined(__GNUC__)
#       define SYM(sig, f) (__extension__ (sig)(f))
#else
#       define SYM(sig, f) ((sig)(f))
#endif

#define SELF(Plg)                                                       \
	IRC_UTIL_CONTAINER_OF(Plg, struct self, parent)

/* Invoke with arguments and return value. */
#define INVOKE(Pg, Name, Sig, ...)                                                      \
do {                                                                                    \
        struct self *self = SELF(Pg);                                                   \
        Sig fn;                                                                         \
                                                                                        \
        if (self->handle && (fn = SYM(Sig, dlsym(self->handle, symbol(self, Name)))))   \
                return fn(__VA_ARGS__);                                                 \
} while (0)

/* Invoke with argument and without return value. */
#define INVOKE_NR(Pg, Name, Sig, ...)                                                   \
do {                                                                                    \
        struct self *self = SELF(Pg);                                                   \
        Sig fn;                                                                         \
                                                                                        \
        if (self->handle && (fn = SYM(Sig, dlsym(self->handle, symbol(self, Name)))))   \
                fn(__VA_ARGS__);                                                        \
} while (0)

/* Invoke without arguments and with return value. */
#define INVOKE_NA(Pg, Name, Sig)                                                        \
do {                                                                                    \
        struct self *self = SELF(Pg);                                                   \
        Sig fn;                                                                         \
                                                                                        \
        if (self->handle && (fn = SYM(Sig, dlsym(self->handle, symbol(self, Name)))))   \
                return fn();                                                            \
} while (0)

/* Invoke without arguments and without return value. */
#define INVOKE_NA_NR(Pg, Name, Sig)                                                     \
do {                                                                                    \
        struct self *self = SELF(Pg);                                                   \
        Sig fn;                                                                         \
                                                                                        \
        if (self->handle && (fn = SYM(Sig, dlsym(self->handle, symbol(self, Name)))))   \
                fn();                                                                   \
} while (0)

struct self {
	struct irc_plugin parent;
	char prefix[32];
	void *handle;
};

typedef const char *            (*get_option_fn)(const char *);
typedef const char *            (*get_path_fn)(const char *);
typedef const char *            (*get_template_fn)(const char *);
typedef const char * const *    (*get_options_fn)(void);
typedef const char * const *    (*get_paths_fn)(void);
typedef const char * const *    (*get_templates_fn)(void);
typedef void                    (*event_fn)(const struct irc_event *);
typedef int                     (*load_fn)(void);
typedef void                    (*reload_fn)(void);
typedef void                    (*unload_fn)(void);
typedef void                    (*set_option_fn)(const char *, const char *);
typedef void                    (*set_path_fn)(const char *, const char *);
typedef void                    (*set_template_fn)(const char *, const char *);

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
	INVOKE_NR(plg, "set_template", set_template_fn, key, value);
}

static const char *
get_template(struct irc_plugin *plg, const char *key)
{
	INVOKE(plg, "get_template", get_template_fn, key);

	return NULL;
}

static const char * const *
get_templates(struct irc_plugin *plg)
{
	INVOKE_NA(plg, "get_templates", get_templates_fn);

	return NULL;
}

static void
set_path(struct irc_plugin *plg, const char *key, const char *value)
{
	INVOKE_NR(plg, "set_path", set_path_fn, key, value);
}

static const char *
get_path(struct irc_plugin *plg, const char *key)
{
	INVOKE(plg, "get_path", get_path_fn, key);

	return NULL;
}

static const char * const *
get_paths(struct irc_plugin *plg)
{
	INVOKE_NA(plg, "get_paths", get_paths_fn);

	return NULL;
}

static void
set_option(struct irc_plugin *plg, const char *key, const char *value)
{
	INVOKE_NR(plg, "set_option", set_option_fn, key, value);
}

static const char *
get_option(struct irc_plugin *plg, const char *key)
{
	INVOKE(plg, "get_option", get_option_fn, key);

	return NULL;
}

static const char * const *
get_options(struct irc_plugin *plg)
{
	INVOKE_NA(plg, "get_options", get_options_fn);

	return NULL;
}

static int
load(struct irc_plugin *plg)
{
	INVOKE_NA(plg, "load", load_fn);

	return 0;
}

static void
reload(struct irc_plugin *plg)
{
	INVOKE_NA_NR(plg, "reload", reload_fn);
}

static void
unload(struct irc_plugin *plg)
{
	INVOKE_NA_NR(plg, "unload", unload_fn);
}

static void
handle(struct irc_plugin *plg, const struct irc_event *ev)
{
	INVOKE_NR(plg, "event", event_fn, ev);
}

static void
finish(struct irc_plugin *plg)
{
	struct self *self = SELF(plg);

	if (self->handle)
		dlclose(self->handle);

	free(self);
}

static struct self *
init(const char *name, const char *path)
{
	struct self self;
	struct stat st;

	irc_plugin_init(&self.parent, name);

	/*
	 * It's not possible to get the exact error code when loading a plugin
	 * using dlopen, since we're trying a lot of files that potentially not
	 * exist we check presence before even though there's a possible
	 * condition but at least we can print an error message if there are
	 * other errors than missing file.
	 */
	if (path != NULL && stat(path, &st) < 0 && errno == ENOENT) {
		irc_log_warn("plugin: %s: %s", path, strerror(errno));
		return NULL;
	}

	if (!(self.handle = dlopen(path, RTLD_NOW | RTLD_GLOBAL))) {
		irc_log_warn("plugin: %s: %s", path, dlerror());
		return NULL;
	}

	/*
	 * Compute prefix name, usually from the file name which we will remove
	 * the extension, otherwise we use the name as alternative.
	 */
	if (path) {
		irc_util_strlcpy(self.prefix, irc_util_basename(path), sizeof (self.prefix));

		/* Remove plugin extension. */
		self.prefix[strcspn(self.prefix, ".")] = '\0';
	} else
		irc_util_strlcpy(self.prefix, name, sizeof (self.prefix));

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

static void
wrap_finish(struct irc_plugin_loader *ldr)
{
	free(ldr);
}

struct irc_plugin *
dl_plugin_open(const char *name, const char *path)
{
	struct self *self;

	if (!(self = init(name, path)))
		return NULL;

	/* Data and all callbacks. */
	self->parent.set_template = set_template;
	self->parent.get_template = get_template;
	self->parent.get_templates = get_templates;
	self->parent.set_path = set_path;
	self->parent.get_path = get_path;
	self->parent.get_paths = get_paths;
	self->parent.set_option = set_option;
	self->parent.get_option = get_option;
	self->parent.get_options = get_options;
	self->parent.load = load;
	self->parent.reload = reload;
	self->parent.unload = unload;
	self->parent.handle = handle;
	self->parent.finish = finish;

	/* Metadata variables. */
	irc_plugin_set_info(&self->parent,
	    metadata(self, "author"),
	    metadata(self, "description"),
	    metadata(self, "version"),
	    metadata(self, "license")
	);

	return &self->parent;
}

struct irc_plugin_loader *
dl_plugin_loader_new(void)
{
	struct irc_plugin_loader *ldr;

	ldr = irc_util_calloc(1, sizeof (*ldr));
	irc_plugin_loader_init(ldr, LDR_PATHS, LDR_EXTENSIONS);

	ldr->open   = wrap_open;
	ldr->finish = wrap_finish;

	return ldr;
}
