/*
 * jsapi-plugin.c -- Javascript plugins
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
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#include <irccd/channel.h>
#include <irccd/config.h>
#include <irccd/event.h>
#include <irccd/log.h>
#include <irccd/plugin.h>
#include <irccd/server.h>
#include <irccd/util.h>

#include "js-plugin.h"
#include "jsapi-chrono.h"
#include "jsapi-directory.h"
#include "jsapi-file.h"
#include "jsapi-hook.h"
#include "jsapi-irccd.h"
#include "jsapi-logger.h"
#include "jsapi-plugin.h"
#include "jsapi-rule.h"
#include "jsapi-server.h"
#include "jsapi-system.h"
#include "jsapi-timer.h"
#include "jsapi-unicode.h"
#include "jsapi-util.h"

struct self {
	struct irc_plugin plugin;
	duk_context *ctx;
	char location[PATH_MAX];
	char **options;
	char **templates;
	char **paths;
	char *license;
	char *version;
	char *author;
	char *description;
};

static void
freelist(char **table)
{
	if (!table)
		return;

	for (char **p = table; *p; ++p)
		free(*p);

	free(table);
}

static char *
metadata(duk_context *ctx, const char *name)
{
	char *ret = NULL;

	duk_get_global_string(ctx, "info");

	if (duk_get_type(ctx, -1) == DUK_TYPE_OBJECT) {
		duk_get_prop_string(ctx, -1, name);

		if (duk_get_type(ctx, -1) == DUK_TYPE_STRING)
			ret = irc_util_strdup(duk_get_string(ctx, -1));

		duk_pop(ctx);
	}

	duk_pop(ctx);

	return ret ? ret : irc_util_strdup("unknown");
}

static void
push_modes(duk_context *ctx, char **modes)
{
	size_t i = 0;

	duk_push_array(ctx);

	for (char **mode = modes; mode && *mode; ++mode) {
		duk_push_string(ctx, *mode);
		duk_put_prop_index(ctx, -2, i++);
	}
}

static void
push_names(duk_context *ctx, const struct irc_event *ev)
{
	const char *token;
	char *p = ev->names.names;

	duk_push_array(ctx);

	for (size_t i = 0; (token = strtok_r(p, " ", &p)); ++i) {
		irc_server_strip(ev->server, &token);
		duk_push_string(ctx, token);
		duk_put_prop_index(ctx, -2, i);
	}
}

static void
push_whois(duk_context *ctx, const struct irc_event *ev)
{
	duk_push_object(ctx);
	duk_push_string(ctx, ev->whois.nickname);
	duk_put_prop_string(ctx, -2, "nickname");
	duk_push_string(ctx, ev->whois.username);
	duk_put_prop_string(ctx, -2, "username");
	duk_push_string(ctx, ev->whois.realname);
	duk_put_prop_string(ctx, -2, "realname");
	duk_push_string(ctx, ev->whois.hostname);
	duk_put_prop_string(ctx, -2, "hostname");
	duk_push_array(ctx);

	for (size_t i = 0; i < ev->whois.channelsz; ++i) {
		duk_push_object(ctx);
		duk_push_string(ctx, ev->whois.channels[i].name);
		duk_put_prop_string(ctx, -2, "channel");
		duk_push_int(ctx, ev->whois.channels[i].modes);
		duk_put_prop_string(ctx, -2, "modes");
		duk_put_prop_index(ctx, -2, i);
	}

	duk_put_prop_string(ctx, -2, "channels");
}

static void
log_trace(struct self *self)
{
	char *stack, *token, *p;
	int linenumber;

	duk_get_prop_string(self->ctx, -1, "stack");
	stack = strdup(duk_opt_string(self->ctx, -1, ""));
	duk_pop(self->ctx);
	duk_get_prop_string(self->ctx, -1, "lineNumber");
	linenumber = duk_get_int(self->ctx, -1);
	duk_pop(self->ctx);

	irc_log_warn("plugin %s: %s:%d", self->plugin.name, self->location, linenumber);

	/* We can't put a '\n' in irc_log_warn so loop for them. */
	for (p = stack; *stack && (token = strtok_r(p, "\n", &p)); )
		irc_log_warn("plugin %s: %s", self->plugin.name, token);

	free(stack);
}

static const char * const *
get_table(duk_context *ctx, const char *name, char ***ptable)
{
	char **list = NULL;
	size_t listsz = 0;

	duk_get_global_string(ctx, name);
	duk_enum(ctx, -1, 0);

	for (size_t i = 0; duk_next(ctx, -1, 1); ++i) {
		list = irc_util_reallocarray(list, ++listsz, sizeof (char *));
		list[i] = irc_util_strdup(duk_to_string(ctx, -2));
		duk_pop_n(ctx, 2);
	}

	duk_pop_n(ctx, 2);

	/* Add a NULL sentinel value. */
	list = irc_util_reallocarray(list, listsz + 1, sizeof (char *));
	list[listsz] = NULL;

	freelist(*ptable);
	*ptable = list;

	return (const char * const *)list;
}

static void
set_key_value(duk_context *ctx, const char *table, const char *key, const char *value)
{
	duk_get_global_string(ctx, table);
	duk_push_string(ctx, value);
	duk_put_prop_string(ctx, -2, key);
	duk_pop(ctx);
}

static const char *
get_value(duk_context *ctx, const char *table, const char *key)
{
	const char *ret;

	duk_get_global_string(ctx, table);
	duk_get_prop_string(ctx, -1, key);
	ret = duk_to_string(ctx, -1);
	duk_pop_n(ctx, 2);

	return ret;
}

static void
set_template(struct irc_plugin *plg, const char *key, const char *value)
{
	struct self *js = plg->data;

	set_key_value(js->ctx, JSAPI_PLUGIN_PROP_TEMPLATES, key, value);
}

static const char *
get_template(struct irc_plugin *plg, const char *key)
{
	struct self *js = plg->data;

	return get_value(js->ctx, JSAPI_PLUGIN_PROP_TEMPLATES, key);
}

static const char * const *
get_templates(struct irc_plugin *plg)
{
	struct self *js = plg->data;

	return get_table(js->ctx, JSAPI_PLUGIN_PROP_TEMPLATES, &js->templates);
}

static void
set_path(struct irc_plugin *plg, const char *key, const char *value)
{
	struct self *js = plg->data;

	set_key_value(js->ctx, JSAPI_PLUGIN_PROP_PATHS, key, value);
}

static const char *
get_path(struct irc_plugin *plg, const char *key)
{
	struct self *js = plg->data;

	return get_value(js->ctx, JSAPI_PLUGIN_PROP_PATHS, key);
}

static const char * const *
get_paths(struct irc_plugin *plg)
{
	struct self *js = plg->data;

	return get_table(js->ctx, JSAPI_PLUGIN_PROP_PATHS, &js->paths);
}

static void
set_option(struct irc_plugin *plg, const char *key, const char *value)
{
	struct self *js = plg->data;

	set_key_value(js->ctx, JSAPI_PLUGIN_PROP_OPTIONS, key, value);
}

static const char *
get_option(struct irc_plugin *plg, const char *key)
{
	struct self *js = plg->data;

	return get_value(js->ctx, JSAPI_PLUGIN_PROP_OPTIONS, key);
}

static const char * const *
get_options(struct irc_plugin *plg)
{
	struct self *js = plg->data;

	return get_table(js->ctx, JSAPI_PLUGIN_PROP_OPTIONS, &js->options);
}

static int
vcall(struct irc_plugin *plg, const char *function, const char *fmt, va_list ap)
{
	struct self *self = plg->data;
	int nargs = 0, ret = 0;

	duk_get_global_string(self->ctx, function);

	if (!duk_is_function(self->ctx, -1)) {
		duk_pop(self->ctx);
		return ret;
	}

	for (const char *f = fmt; *f; ++f) {
		void (*push)(duk_context *, void *);

		switch (*f) {
		case 'S':
			jsapi_server_push(self->ctx, va_arg(ap, struct irc_server *));
			break;
		case 's':
			duk_push_string(self->ctx, va_arg(ap, const char *));
			break;
		case 'x':
			push = va_arg(ap, void (*)(duk_context *, void *));
			push(self->ctx, va_arg(ap, void *));
			break;
		default:
			continue;
		}

		++nargs;
	}

	if (duk_pcall(self->ctx, nargs) != 0) {
		log_trace(plg->data);
		ret = -1;
	}

	duk_pop(self->ctx);

	return ret;
}

static int
call(struct irc_plugin *plg, const char *function, const char *fmt, ...)
{
	va_list ap;
	int ret;

	va_start(ap, fmt);
	ret = vcall(plg, function, fmt, ap);
	va_end(ap);

	return ret;
}

static void
handle(struct irc_plugin *plg, const struct irc_event *ev)
{
	(void)plg;
	(void)ev;

	switch (ev->type) {
	case IRC_EVENT_COMMAND:
		call(plg, "onCommand", "Ss ss", ev->server, ev->message.origin,
		    ev->message.channel, ev->message.message);
		break;
	case IRC_EVENT_CONNECT:
		call(plg, "onConnect", "S", ev->server);
		break;
	case IRC_EVENT_DISCONNECT:
		call(plg, "onDisconnect", "S", ev->server);
		break;
	case IRC_EVENT_INVITE:
		call(plg, "onInvite", "Ss s", ev->server, ev->invite.origin,
		     ev->invite.channel);
		break;
	case IRC_EVENT_JOIN:
		call(plg, "onJoin", "Ss s", ev->server, ev->join.origin,
		    ev->join.channel);
		break;
	case IRC_EVENT_KICK:
		call(plg, "onKick", "Ss sss", ev->server, ev->kick.origin,
		    ev->kick.channel, ev->kick.target, ev->kick.reason);
		break;
	case IRC_EVENT_ME:
		call(plg, "onMe", "Ss ss", ev->server, ev->message.origin,
		    ev->message.channel, ev->message.message);
		break;
	case IRC_EVENT_MESSAGE:
		call(plg, "onMessage", "Ss ss", ev->server, ev->message.origin,
		    ev->message.channel, ev->message.message);
		break;
	case IRC_EVENT_MODE:
		call(plg, "onMode", "Ss ssx", ev->server, ev->mode.origin,
		    ev->mode.channel, ev->mode.mode, push_modes, ev->mode.args);
		break;
	case IRC_EVENT_NAMES:
		call(plg, "onNames", "Ss x", ev->server, ev->names.channel,
		    push_names, ev);
		break;
	case IRC_EVENT_NICK:
		call(plg, "onNick", "Ss s", ev->server, ev->nick.origin,
		    ev->nick.nickname);
		break;
	case IRC_EVENT_NOTICE:
		call(plg, "onNotice", "Ss ss", ev->server, ev->notice.origin,
		    ev->notice.channel, ev->notice.notice);
		break;
	case IRC_EVENT_PART:
		call(plg, "onPart", "Ss ss", ev->server, ev->part.origin,
		    ev->part.channel, ev->part.reason);
		break;
	case IRC_EVENT_TOPIC:
		call(plg, "onTopic", "Ss ss", ev->server, ev->topic.origin,
		    ev->topic.channel, ev->topic.topic);
		break;
	case IRC_EVENT_WHOIS:
		call(plg, "onWhois", "Sx", ev->server, push_whois, ev);
		break;
	default:
		break;
	}
}

static char *
eat(const char *path)
{
	int fd = -1;
	char *ret = NULL;
	struct stat st;

	if ((fd = open(path, O_RDONLY)) < 0)
		goto err;
	if (fstat(fd, &st) < 0)
		goto err;
	if (!(ret = calloc(1, st.st_size + 1)))
		goto err;
	if (read(fd, ret, st.st_size) != st.st_size)
		goto err;

	close(fd);

	return ret;

err:
	if (fd != -1)
		close(fd);

	free(ret);

	return NULL;
}

static void *
wrap_malloc(void *udata, size_t size)
{
	(void)udata;

	return irc_util_malloc(size);
}

static void *
wrap_realloc(void *udata, void *ptr, size_t size)
{
	(void)udata;

	return irc_util_realloc(ptr, size);
}

static void
wrap_free(void *udata, void *ptr)
{
	(void)udata;

	free(ptr);
}

static struct self *
init(const char *name, const char *path, const char *script)
{
	struct self *js;

	js = irc_util_calloc(1, sizeof (*js));
	js->ctx = duk_create_heap(wrap_malloc, wrap_realloc, wrap_free, NULL, NULL);
	irc_util_strlcpy(js->plugin.name, name, sizeof (js->plugin.name));

	/* Copy path because Duktape has no notions of it. */
	irc_util_strlcpy(js->location, path, sizeof (js->location));

	/* Tables used to retrieve data. */
	duk_push_object(js->ctx);
	duk_put_global_string(js->ctx, JSAPI_PLUGIN_PROP_OPTIONS);
	duk_push_object(js->ctx);
	duk_put_global_string(js->ctx, JSAPI_PLUGIN_PROP_TEMPLATES);
	duk_push_object(js->ctx);
	duk_put_global_string(js->ctx, JSAPI_PLUGIN_PROP_PATHS);

	/* Load Javascript APIs. */
	jsapi_load(js->ctx);
	jsapi_chrono_load(js->ctx);
	jsapi_directory_load(js->ctx);
	jsapi_file_load(js->ctx);
	jsapi_hook_load(js->ctx);
	jsapi_logger_load(js->ctx);
	jsapi_plugin_load(js->ctx, &js->plugin);
	jsapi_rule_load(js->ctx);
	jsapi_server_load(js->ctx);
	jsapi_system_load(js->ctx);
	jsapi_timer_load(js->ctx);
	jsapi_unicode_load(js->ctx);
	jsapi_util_load(js->ctx);

	/* Finally execute the script. */
	if (duk_peval_string(js->ctx, script) != 0) {
		log_trace(js);
		duk_destroy_heap(js->ctx);
		free(js);
		return NULL;
	}

	js->plugin.license = js->license = metadata(js->ctx, "license");
	js->plugin.version = js->version = metadata(js->ctx, "version");
	js->plugin.author = js->author = metadata(js->ctx, "author");
	js->plugin.description = js->description = metadata(js->ctx, "summary");

	return js;
}

static int
load(struct irc_plugin *plg)
{
	return call(plg, "onLoad", "");
}

static void
reload(struct irc_plugin *plg)
{
	call(plg, "onReload", "");
}

static void
unload(struct irc_plugin *plg)
{
	call(plg, "onUnload", "");
}

static void
finish(struct irc_plugin *plg)
{
	struct self *self = plg->data;

	if (self->ctx)
		duk_destroy_heap(self->ctx);

	freelist(self->options);
	freelist(self->templates);
	freelist(self->paths);

	free(self->license);
	free(self->version);
	free(self->author);
	free(self->description);
	free(self);
}

static struct irc_plugin *
wrap_open(struct irc_plugin_loader *ldr, const char *name, const char *path)
{
	(void)ldr;

	return js_plugin_open(name, path);
}

duk_context *
js_plugin_get_context(struct irc_plugin *js)
{
	struct self *self = js->data;

	return self->ctx;
}

struct irc_plugin *
js_plugin_open(const char *name, const char *path)
{
	assert(path);

	char *script = NULL;
	struct self *self;

	/*
	 * Duktape can't open script from file path so we need to read the
	 * whole script at once.
	 */
	if (!(script = eat(path))) {
		if (errno != ENOENT)
			irc_log_warn("plugin: %s: %s", path, strerror(errno));

		return NULL;
	}

	/* Init already log errors. */
	if (!(self = init(name, path, script))) {
		free(script);
		return NULL;
	}

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

	/* No longer needed. */
	free(script);

	return &self->plugin;
}

struct irc_plugin_loader *
js_plugin_loader_new(void)
{
	struct irc_plugin_loader *ldr;

	ldr = irc_util_calloc(1, sizeof (*ldr));
	ldr->open = wrap_open;
	irc_util_strlcpy(ldr->extensions, "js", sizeof (ldr->extensions));
	irc_util_strlcpy(ldr->paths, IRCCD_LIBDIR "/irccd", sizeof (ldr->paths));

	return ldr;
}
