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
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#include "channel.h"
#include "event.h"
#include "js-plugin.h"
#include "jsapi-chrono.h"
#include "jsapi-directory.h"
#include "jsapi-file.h"
#include "jsapi-irccd.h"
#include "jsapi-logger.h"
#include "jsapi-plugin.h"
#include "jsapi-plugin.h"
#include "jsapi-server.h"
#include "jsapi-system.h"
#include "jsapi-timer.h"
#include "jsapi-unicode.h"
#include "jsapi-util.h"
#include "log.h"
#include "plugin.h"
#include "server.h"
#include "util.h"

static void
freelist(char **table)
{
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
push_names(duk_context *ctx, const struct irc_event *ev)
{
	const char *token;
	char *p = ev->names.names;

	duk_push_array(ctx);

	for (size_t i = 0; (token = strtok_r(p, " ", &p)); ++i) {
		irc_server_strip(ev->server, &token, NULL, NULL);
		duk_push_string(ctx, token);
		duk_put_prop_index(ctx, -2, i);
	}
}

static void
push_whois(duk_context *ctx, const struct irc_event *ev)
{
	const char *token;
	char *p = ev->whois.channels;

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
	for (size_t i = 0; (token = strtok_r(p, " ", &p)); ++i) {
		char mode = 0, prefix = 0;

		irc_server_strip(ev->server, &token, &mode, &prefix);
		duk_push_object(ctx);
		duk_push_string(ctx, token);
		duk_put_prop_string(ctx, -2, "channel");
		duk_push_sprintf(ctx, "%c", mode);
		duk_put_prop_string(ctx, -2, "mode");
		duk_push_sprintf(ctx, "%c", prefix);
		duk_put_prop_string(ctx, -2, "prefix");
		duk_put_prop_index(ctx, -2, i);
	}
	duk_put_prop_string(ctx, -2, "channels");
}

static const char **
get_table(duk_context *ctx, const char *name, char ***ptable)
{
	char **list;
	size_t listsz;

	duk_get_global_string(ctx, name);

	if (!(listsz = duk_get_length(ctx, -1))) {
		duk_pop(ctx);
		return NULL;
	}

	list = irc_util_calloc(listsz + 1, sizeof (char *));

	duk_enum(ctx, -1, 0);

	for (size_t i = 0; i < listsz && duk_next(ctx, -1, true); ++i) {
		list[i] = irc_util_strdup(duk_to_string(ctx, -2));
		duk_pop_n(ctx, 2);
	}

	duk_pop_n(ctx, 2);

	freelist(*ptable);
	*ptable = list;

	return (const char **)list;
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
	struct irc_js_plugin_data *js = plg->data;

	set_key_value(js->ctx, IRC_JSAPI_PLUGIN_PROP_TEMPLATES, key, value);
}

static const char *
get_template(struct irc_plugin *plg, const char *key)
{
	struct irc_js_plugin_data *js = plg->data;

	return get_value(js->ctx, IRC_JSAPI_PLUGIN_PROP_TEMPLATES, key);
}

static const char **
get_templates(struct irc_plugin *plg)
{
	struct irc_js_plugin_data *js = plg->data;

	return get_table(js->ctx, IRC_JSAPI_PLUGIN_PROP_TEMPLATES, &js->templates);
}

static void
set_path(struct irc_plugin *plg, const char *key, const char *value)
{
	struct irc_js_plugin_data *js = plg->data;

	set_key_value(js->ctx, IRC_JSAPI_PLUGIN_PROP_PATHS, key, value);
}

static const char *
get_path(struct irc_plugin *plg, const char *key)
{
	struct irc_js_plugin_data *js = plg->data;

	return get_value(js->ctx, IRC_JSAPI_PLUGIN_PROP_PATHS, key);
}

static const char **
get_paths(struct irc_plugin *plg)
{
	struct irc_js_plugin_data *js = plg->data;

	return get_table(js->ctx, IRC_JSAPI_PLUGIN_PROP_PATHS, &js->paths);
}

static void
set_option(struct irc_plugin *plg, const char *key, const char *value)
{
	struct irc_js_plugin_data *js = plg->data;

	set_key_value(js->ctx, IRC_JSAPI_PLUGIN_PROP_OPTIONS, key, value);
}

static const char *
get_option(struct irc_plugin *plg, const char *key)
{
	struct irc_js_plugin_data *js = plg->data;

	return get_value(js->ctx, IRC_JSAPI_PLUGIN_PROP_OPTIONS, key);
}

static const char **
get_options(struct irc_plugin *plg)
{
	struct irc_js_plugin_data *js = plg->data;

	return get_table(js->ctx, IRC_JSAPI_PLUGIN_PROP_OPTIONS, &js->options);
}

static void
vcall(struct irc_plugin *plg, const char *function, const char *fmt, va_list ap)
{
	struct irc_js_plugin_data *self = plg->data;
	int nargs = 0;

	duk_get_global_string(self->ctx, function);

	if (!duk_is_function(self->ctx, -1)) {
		duk_pop(self->ctx);
		return;
	}

	for (const char *f = fmt; *f; ++f) {
		void (*push)(duk_context *, void *);

		switch (*f) {
		case 'S':
			irc_jsapi_server_push(self->ctx, va_arg(ap, struct irc_server *));
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

	if (duk_pcall(self->ctx, nargs) != 0)
		irc_log_warn("plugin %s: %s\n", duk_to_string(self->ctx, -1));

	duk_pop(self->ctx);
}

static void
call(struct irc_plugin *plg, const char *function, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vcall(plg, function, fmt, ap);
	va_end(ap);
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
		call(plg, "onMode", "Ss sss ss", ev->server, ev->mode.origin,
		    ev->mode.channel, ev->mode.mode, ev->mode.limit,
		    ev->mode.user, ev->mode.mask);
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

	return false;
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

static struct irc_js_plugin_data *
init(const char *path, const char *script)
{
	struct irc_js_plugin_data *js;

	js = irc_util_calloc(1, sizeof (*js));
	js->ctx = duk_create_heap(wrap_malloc, wrap_realloc, wrap_free, NULL, NULL);

	irc_jsapi_load(js->ctx);
	irc_jsapi_chrono_load(js->ctx);
	irc_jsapi_directory_load(js->ctx);
	irc_jsapi_file_load(js->ctx);
	irc_jsapi_logger_load(js->ctx);
	irc_jsapi_plugin_load(js->ctx, &js->plugin);
	irc_jsapi_server_load(js->ctx);
	irc_jsapi_system_load(js->ctx);
	irc_jsapi_timer_load(js->ctx);
	irc_jsapi_unicode_load(js->ctx);
	irc_jsapi_util_load(js->ctx);

	if (duk_peval_string(js->ctx, script) != 0) {
		irc_log_warn("plugin: %s: %s", path, duk_to_string(js->ctx, -1));
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

static void
load(struct irc_plugin *plg)
{
	call(plg, "onLoad", "");
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
	struct irc_js_plugin_data *self = plg->data;

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
wrap_open(struct irc_plugin_loader *ldr, const char *path)
{
	(void)ldr;

	return irc_js_plugin_open(path);
}

struct irc_plugin *
irc_js_plugin_open(const char *path)
{
	assert(path);

	char *script = NULL;
	struct irc_js_plugin_data *self;

	/*
	 * Duktape can't open script from file path so we need to read the
	 * whole script at once.
	 */
	if (!(script = eat(path))) {
		if (errno != ENOENT)
			irc_log_warn("irccd: %s: %s", path, strerror(errno));

		return NULL;
	}

	/* Init already log errors. */
	if (!(self = init(path, script))) {
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
irc_js_plugin_loader_new(void)
{
	struct irc_plugin_loader *ldr;

	ldr = irc_util_calloc(1, sizeof (*ldr));
	ldr->open = wrap_open;
	strlcpy(ldr->extensions, "js", sizeof (ldr->extensions));
	strlcpy(ldr->paths, IRCCD_LIBDIR "/irccd", sizeof (ldr->paths));

	return ldr;
}
