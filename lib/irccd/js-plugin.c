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

#include <duktape.h>

#include "channel.h"
#include "event.h"
#include "js-plugin.h"
#include "jsapi-file.h"
#include "jsapi-irccd.h"
#include "jsapi-logger.h"
#include "jsapi-plugin.h"
#include "jsapi-plugin.h"
#include "jsapi-server.h"
#include "jsapi-system.h"
#include "jsapi-timer.h"
#include "jsapi-unicode.h"
#include "log.h"
#include "plugin.h"
#include "server.h"
#include "util.h"

struct self {
	duk_context *ctx;
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
	const struct irc_channel *ch = irc_server_find(ev->server, ev->msg.args[0]);

	duk_push_array(ctx);

	for (size_t i = 0; i < ch->usersz; ++i) {
		duk_push_string(ctx, ch->users[i].nickname);
		duk_put_prop_index(ctx, -2, i);
	}
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
	struct self *js = plg->data;

	set_key_value(js->ctx, IRC_JSAPI_PLUGIN_PROP_TEMPLATES, key, value);
}

static const char *
get_template(struct irc_plugin *plg, const char *key)
{
	struct self *js = plg->data;

	return get_value(js->ctx, IRC_JSAPI_PLUGIN_PROP_TEMPLATES, key);
}

static const char **
get_templates(struct irc_plugin *plg)
{
	struct self *js = plg->data;

	return get_table(js->ctx, IRC_JSAPI_PLUGIN_PROP_TEMPLATES, &js->templates);
}

static void
set_path(struct irc_plugin *plg, const char *key, const char *value)
{
	struct self *js = plg->data;

	set_key_value(js->ctx, IRC_JSAPI_PLUGIN_PROP_PATHS, key, value);
}

static const char *
get_path(struct irc_plugin *plg, const char *key)
{
	struct self *js = plg->data;

	return get_value(js->ctx, IRC_JSAPI_PLUGIN_PROP_PATHS, key);
}

static const char **
get_paths(struct irc_plugin *plg)
{
	struct self *js = plg->data;

	return get_table(js->ctx, IRC_JSAPI_PLUGIN_PROP_PATHS, &js->paths);
}

static void
set_option(struct irc_plugin *plg, const char *key, const char *value)
{
	struct self *js = plg->data;

	set_key_value(js->ctx, IRC_JSAPI_PLUGIN_PROP_OPTIONS, key, value);
}

static const char *
get_option(struct irc_plugin *plg, const char *key)
{
	struct self *js = plg->data;

	return get_value(js->ctx, IRC_JSAPI_PLUGIN_PROP_OPTIONS, key);
}

static const char **
get_options(struct irc_plugin *plg)
{
	struct self *js = plg->data;

	return get_table(js->ctx, IRC_JSAPI_PLUGIN_PROP_OPTIONS, &js->options);
}

static void
vcall(struct irc_plugin *plg, const char *function, const char *fmt, va_list ap)
{
	struct self *self = plg->data;
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
	case IRC_EVENT_CONNECT:
		call(plg, "onConnect", "S", ev->server);
		break;
	case IRC_EVENT_DISCONNECT:
		call(plg, "onDisconnect", "S", ev->server);
		break;
	case IRC_EVENT_INVITE:
		call(plg, "onInvite", "Ss s", ev->server, ev->msg.prefix,
		     ev->msg.args[1]);
		break;
	case IRC_EVENT_JOIN:
		call(plg, "onJoin", "Ss s", ev->server, ev->msg.prefix,
		    ev->msg.args[0]);
		break;
	case IRC_EVENT_KICK:
		call(plg, "onKick", "Ss sss", ev->server, ev->msg.prefix,
		    ev->msg.args[0], ev->msg.args[1], ev->msg.args[2]);
		break;
	case IRC_EVENT_ME:
		call(plg, "onMe", "Ss ss", ev->server, ev->msg.prefix,
		    ev->msg.args[0], ev->msg.args[1]);
		break;
	case IRC_EVENT_MESSAGE:
		call(plg, "onMessage", "Ss ss", ev->server, ev->msg.prefix,
		    ev->msg.args[0], ev->msg.args[1]);
		break;
	case IRC_EVENT_MODE:
		call(plg, "onMode", "Ss sss ss", ev->server, ev->msg.prefix,
		    ev->msg.args[0], ev->msg.args[1], ev->msg.args[2],
		    ev->msg.args[3], ev->msg.args[4]);
		break;
	case IRC_EVENT_NAMES:
		call(plg, "onNames", "Ss x", ev->server, ev->msg.args[1],
		    push_names);
		break;
	case IRC_EVENT_NICK:
		call(plg, "onNick", "Ss s", ev->server, ev->msg.prefix,
		    ev->msg.args[0]);
		break;
	case IRC_EVENT_NOTICE:
		call(plg, "onNotice", "Ss ss", ev->server, ev->msg.prefix,
		    ev->msg.args[0], ev->msg.args[1]);
		break;
	case IRC_EVENT_PART:
		call(plg, "onPart", "Ss ss", ev->server, ev->msg.prefix,
		    ev->msg.args[0], ev->msg.args[1]);
		break;
	case IRC_EVENT_TOPIC:
		call(plg, "onTopic", "Ss ss", ev->server, ev->msg.prefix,
		    ev->msg.args[0], ev->msg.args[1]);
		break;
	case IRC_EVENT_WHOIS:
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

static bool
init(struct irc_plugin *plg, const char *script)
{
	struct self js = {0};

	/* Load all modules. */
	js.ctx = duk_create_heap(wrap_malloc, wrap_realloc, wrap_free, NULL, NULL);
	irc_jsapi_load(js.ctx);
	irc_jsapi_file_load(js.ctx);
	irc_jsapi_logger_load(js.ctx);
	irc_jsapi_plugin_load(js.ctx, plg);
	irc_jsapi_server_load(js.ctx);
	irc_jsapi_system_load(js.ctx);
	irc_jsapi_timer_load(js.ctx);
	irc_jsapi_unicode_load(js.ctx);

	if (duk_peval_string(js.ctx, script) != 0) {
		irc_log_warn("plugin %s: %s", plg->name, duk_to_string(js.ctx, -1));
		duk_destroy_heap(js.ctx);
		return false;
	}

	plg->license = js.license = metadata(js.ctx, "license");
	plg->version = js.version = metadata(js.ctx, "version");
	plg->author = js.author = metadata(js.ctx, "author");
	plg->description = js.description = metadata(js.ctx, "summary");
	plg->data = irc_util_memdup(&js, sizeof (js));

	return true;
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
	struct self *self = plg->data;

	if (self->ctx)
		duk_destroy_heap(self->ctx);

	free(self->license);
	free(self->version);
	free(self->author);
	free(self->description);

	memset(self, 0, sizeof (*self));
}

bool
irc_js_plugin_open(struct irc_plugin *plg, const char *path)
{
	assert(plg);
	assert(path);

	char *script = NULL;

	if (!(script = eat(path))) {
		irc_log_warn("plugin: %s", strerror(errno));
		return false;
	}

	if (!(init(plg, script))) {
		free(script);
		return false;
	}

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

	/* No longer needed. */
	free(script);

	/* If error occured, init() has logged. */
	return plg->data != NULL;
}
