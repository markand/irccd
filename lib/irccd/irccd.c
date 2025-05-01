/*
 * irccd.c -- main irccd object
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

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <utlist.h>

#include <ev.h>

#include "config.h"
#include "event.h"
#include "hook.h"
#include "irccd.h"
#include "log.h"
#include "plugin.h"
#include "rule.h"
#include "server.h"
#include "util.h"

/* Private bot context. */
static struct {
	struct ev_loop *loop;
} priv = {};

/* Public bot context. */
static struct irccd bot = {};

const struct irccd *irccd = &bot;

static int
is_command(const struct irc_plugin *p, const struct irc_event *ev)
{
	const char *cc;
	size_t ccsz;

	if (ev->type != IRC_EVENT_MESSAGE)
		return 0;

	/* Get the command prefix (e.g !)*/
	cc = ev->server->prefix;
	ccsz = strlen(cc);

	return strncmp(ev->message.message, cc, ccsz) == 0 &&
	       strncmp(ev->message.message + ccsz, p->name, strlen(p->name)) == 0;
}

static struct irc_event *
to_command(const struct irc_plugin *p, const struct irc_event *ev)
{
	static struct irc_event cev;

	/* Convert "!test foo bar" to "foo bar" */
	memcpy(&cev, ev, sizeof (*ev));
	cev.type = IRC_EVENT_COMMAND;
	cev.message.message += strlen(cev.server->prefix) + strlen(p->name);

	while (*cev.message.message && isspace(*cev.message.message))
		++cev.message.message;

	return &cev;
}

static int
invokable(const struct irc_plugin *p, const struct irc_event *ev)
{
	switch (ev->type) {
	case IRC_EVENT_COMMAND:
		return irc_rule_matchlist(bot.rules, ev->server->name,
		    ev->message.channel, ev->message.origin, p->name, "onCommand");
	case IRC_EVENT_CONNECT:
		return irc_rule_matchlist(bot.rules, ev->server->name,
		    NULL, NULL, p->name, "onConnect");
	case IRC_EVENT_DISCONNECT:
		return irc_rule_matchlist(bot.rules, ev->server->name,
		    NULL, NULL, p->name, "onDisconnect");
	case IRC_EVENT_INVITE:
		return irc_rule_matchlist(bot.rules, ev->server->name,
		    ev->invite.channel, ev->invite.origin, p->name, "onInvite");
	case IRC_EVENT_JOIN:
		return irc_rule_matchlist(bot.rules, ev->server->name,
		    ev->join.channel, ev->join.origin, p->name, "onJoin");
	case IRC_EVENT_KICK:
		return irc_rule_matchlist(bot.rules, ev->server->name,
		    ev->kick.channel, ev->kick.origin, p->name, "onKick");
		break;
	case IRC_EVENT_ME:
		return irc_rule_matchlist(bot.rules, ev->server->name,
		    ev->message.channel, ev->message.origin, p->name, "onMe");
	case IRC_EVENT_MESSAGE:
		return irc_rule_matchlist(bot.rules, ev->server->name,
		    ev->message.channel, ev->message.origin, p->name, "onMessage");
	case IRC_EVENT_MODE:
		return irc_rule_matchlist(bot.rules, ev->server->name,
		    ev->mode.channel, ev->mode.origin, p->name, "onMode");
	case IRC_EVENT_NAMES:
		return irc_rule_matchlist(bot.rules, ev->server->name,
		    ev->names.channel, NULL, p->name, "onNames");
	case IRC_EVENT_NICK:
		return irc_rule_matchlist(bot.rules, ev->server->name,
		    NULL, ev->nick.origin, p->name, "onNick");
	case IRC_EVENT_NOTICE:
		return irc_rule_matchlist(bot.rules, ev->server->name,
		    ev->notice.channel, ev->notice.origin, p->name, "onNotice");
	case IRC_EVENT_PART:
		return irc_rule_matchlist(bot.rules, ev->server->name,
		    ev->part.channel, ev->part.origin, p->name, "onPart");
	case IRC_EVENT_TOPIC:
		return irc_rule_matchlist(bot.rules, ev->server->name,
		    ev->topic.channel, ev->topic.origin, p->name, "onTopic");
	case IRC_EVENT_WHOIS:
		return irc_rule_matchlist(bot.rules, ev->server->name,
		    NULL, NULL, p->name, "onWhois");
	default:
		return 1;
	}
}

static struct irc_plugin *
try_plugin(struct irc_plugin_loader *ldr, const char *base, const char *name, const char *ext)
{
	char path[PATH_MAX] = {};

	if (ext)
		snprintf(path, sizeof (path), "%s/%s.%s", base, name, ext);
	else
		snprintf(path, sizeof (path), "%s/%s", base, name);

	irc_log_info("irccd: trying %s", path);

	return irc_plugin_loader_open(ldr, name, path);
}

static struct irc_plugin *
find_plugin(struct irc_plugin_loader *ldr, const char *base, const char *name)
{
	char *extensions = NULL, *t, *ext;
	struct irc_plugin *p;

	/* Copy the extensions to iterate over ':' */
	if (ldr->extensions) {
		extensions = irc_util_strdup(ldr->extensions);

		for (t = extensions; (ext = strtok_r(t, ":", &t)); )
			if ((p = try_plugin(ldr, base, name, ext)))
				break;

		free(extensions);
	} else {
		/*
		 * No extension? weird but allow a unique direct filename in the
		 * directory mentioned.
		 */
		p = try_plugin(ldr, base, name, NULL);
	}

	return p;
}

static inline struct irc_plugin *
open_plugin(struct irc_plugin_loader *ldr, const char *name, const char *path)
{
	return irc_plugin_loader_open(ldr, name, path);
}

static inline int
is_extension_valid(const struct irc_plugin_loader *ldr, const char *path)
{
	char exts[IRC_EXTENSIONS_LEN], *token, *p, *ext;

	irc_util_strlcpy(exts, ldr->extensions, sizeof (exts));

	/* If we're unable to find an extension, assume it's allowed. */
	if (!(ext = strrchr(path, '.')))
		return 1;

	ext++;

	for (p = exts; (token = strtok_r(p, ":", &p)); )
		if (strcmp(token, ext) == 0)
			return 1;

	return 0;
}

void
irc_bot_init(struct ev_loop *loop)
{
	irc_log_to_console();

	if (!loop)
		loop = ev_default_loop(0);

	priv.loop = loop;
}

struct ev_loop *
irc_bot_loop(void)
{
	return priv.loop;
}

int
irc_bot_server_add(struct irc_server *s)
{
	assert(s);

	if (irc_bot_server_get(s->name)) {
		irc_log_warn("irccd: server %s already exists", s->name);
		return -1;
	}

	irc_log_info("irccd: added new server: %s", s->name);

	irc_server_incref(s);
	irc_server_connect(s);

	LL_APPEND(bot.servers, s);

	return 0;
}

struct irc_server *
irc_bot_server_get(const char *name)
{
	struct irc_server *s;

	DL_FOREACH(bot.servers, s)
		if (strcmp(s->name, name) == 0)
			return s;

	return NULL;
}

void
irc_bot_server_remove(const char *name)
{
	struct irc_server *s;

	if (!(s = irc_bot_server_get(name)))
		return;

	irc_server_disconnect(s);

	/* Don't forget to notify plugins. */
	irc_bot_dispatch(&(struct irc_event) {
		.type = IRC_EVENT_DISCONNECT,
		.server = s
	});

	LL_DELETE(bot.servers, s);
	irc_server_decref(s);
}

void
irc_bot_server_clear(void)
{
	struct irc_server *s, *tmp;

	LL_FOREACH_SAFE(bot.servers, s, tmp)
		irc_bot_server_remove(s->name);

	bot.servers = NULL;
}

int
irc_bot_plugin_add(struct irc_plugin *p)
{
	assert(p);

	int rc;

	if (irc_bot_plugin_get(p->name)) {
		irc_log_warn("irccd: plugin %s already exists", p->name);
		return -1;
	}

	if ((rc = irc_plugin_load(p)) == 0) {
		LL_PREPEND(bot.plugins, p);
		irc_log_info("irccd: add new plugin: %s (%s)", p->name, p->description);
		irc_log_info("irccd: %s: version %s, from %s (%s license)", p->name,
		    p->version, p->author, p->license);
	} else
		irc_log_warn("irccd: plugin %s failed to load", p->name);

	return rc;
}

struct irc_plugin *
irc_bot_plugin_search(const char *name, const char *path)
{
	assert(name);

	char *paths = NULL, pathbuf[PATH_MAX], *t, *token;
	struct irc_plugin *p = NULL;
	struct irc_plugin_loader *ldr;

	if (!path)
		irc_log_info("irccd: trying to find plugin %s", name);
	else
		irc_log_info("irccd: opening plugin %s", name);

	LL_FOREACH(bot.plugin_loaders, ldr) {
		if (p)
			break;

		if (path) {
			if (!is_extension_valid(ldr, path))
				continue;

			p = open_plugin(ldr, name, path);
		} else if (paths) {
			/* Copy the paths to tokenize it. */
			paths = irc_util_strdup(ldr->paths);

			/*
			 * For every directory (separated by colon) call find_plugin
			 * which will append the extension and try to open it.
			 */
			for (t = paths; (token = strtok_r(t, ":", &t)); ) {
				if ((p = find_plugin(ldr, token, name)))
					break;
			}
		}
	}

	free(paths);

	if (!p) {
		irc_log_warn("irccd: could not find plugin %s", name);
		return NULL;
	}

	/* Set default paths if they are not set. */
	irc_plugin_set_path(p, "cache", irc_util_printf(pathbuf, sizeof (pathbuf),
	    "%s/plugin/%s", IRCCD_CACHEDIR, p->name));
	irc_plugin_set_path(p, "data", irc_util_printf(pathbuf, sizeof (pathbuf),
	    "%s/plugin/%s", IRCCD_DATADIR, p->name));
	irc_plugin_set_path(p, "config", irc_util_printf(pathbuf, sizeof (pathbuf),
	    "%s/irccd/plugin/%s", IRCCD_SYSCONFDIR, p->name));

	return p;
}

struct irc_plugin *
irc_bot_plugin_get(const char *name)
{
	struct irc_plugin *p;

	LL_FOREACH(bot.plugins, p)
		if (strcmp(p->name, name) == 0)
			return p;

	return NULL;
}

void
irc_bot_plugin_remove(const char *name)
{
	struct irc_plugin *p;

	if (!(p = irc_bot_plugin_get(name)))
		return;

	LL_DELETE(bot.plugins, p);
	irc_plugin_unload(p);
	irc_plugin_finish(p);
}

void
irc_bot_plugin_loader_add(struct irc_plugin_loader *ldr)
{
	assert(ldr);

	LL_PREPEND(bot.plugin_loaders, ldr);
}

void
irc_bot_plugin_clear(void)
{
	struct irc_plugin *p, *tmp;

	LL_FOREACH_SAFE(bot.plugins, p, tmp)
		irc_bot_plugin_remove(p->name);

	bot.plugins = NULL;
}

void
irc_bot_rule_insert(struct irc_rule *rule, size_t index)
{
	assert(rule);

	if (index == 0)
		DL_PREPEND(bot.rules, rule);
	else if (index >= irc_bot_rule_size())
		DL_APPEND(bot.rules, rule);
	else {
		struct irc_rule *pos;

		for (pos = bot.rules; --index; )
			pos = pos->next;

		DL_APPEND_ELEM(bot.rules, pos, rule);
	}
}

struct irc_rule *
irc_bot_rule_get(size_t index)
{
	assert(index < irc_bot_rule_size());

	struct irc_rule *rule;

	for (rule = bot.rules; index-- != 0; )
		rule = rule->next;

	return rule;
}

void
irc_bot_rule_move(size_t from, size_t to)
{
	assert(from < irc_bot_rule_size());

	struct irc_rule *f, *t;

	if (from == to)
		return;

	f = t = bot.rules;

	while (from--)
		f = f->next;

	DL_DELETE(bot.rules, f);

	if (to == 0)
		DL_PREPEND(bot.rules, f);
	else {
		while (t && to--)
			t = t->next;

		DL_APPEND_ELEM(bot.rules, t, f);
	}
}

void
irc_bot_rule_remove(size_t index)
{
	assert(index < irc_bot_rule_size());

	struct irc_rule *pos = bot.rules;

	for (size_t i = 0; i < index; ++i)
		pos = pos->next;

	DL_DELETE(bot.rules, pos);
}

size_t
irc_bot_rule_size(void)
{
	const struct irc_rule *r;
	size_t total = 0;

	DL_FOREACH(bot.rules, r)
		total++;

	return total;
}

void
irc_bot_rule_clear(void)
{
	struct irc_rule *r, *tmp;

	DL_FOREACH_SAFE(bot.rules, r, tmp)
		irc_rule_free(r);

	bot.rules = NULL;
}

int
irc_bot_hook_add(struct irc_hook *h)
{
	assert(h);

	if (irc_bot_hook_get(h->name)) {
		irc_log_warn("irccd: hook %s already exists", h->name);
		return -1;
	}

	LL_PREPEND(bot.hooks, h);

	return 0;
}

struct irc_hook *
irc_bot_hook_get(const char *name)
{
	struct irc_hook *h;

	LL_FOREACH(bot.hooks, h)
		if (strcmp(h->name, name) == 0)
			return h;

	return NULL;
}

void
irc_bot_hook_remove(const char *name)
{
	assert(name);

	struct irc_hook *h;

	if ((h = irc_bot_hook_get(name))) {
		LL_DELETE(bot.hooks, h);
		irc_hook_free(h);
	}
}

void
irc_bot_hook_clear(void)
{
	struct irc_hook *h, *tmp;

	LL_FOREACH_SAFE(bot.hooks, h, tmp)
		irc_hook_free(h);

	bot.hooks = NULL;
}

void
irc_bot_dispatch(const struct irc_event *ev)
{
	struct irc_plugin *p, *ptmp, *plgcmd = NULL;
	struct irc_hook *h, *htmp;

	LL_FOREACH_SAFE(bot.hooks, h, htmp)
		irc_hook_invoke(h, ev);

	/*
	 * Invoke for every plugin the event verbatim. Then, the event may match
	 * a plugin name command in that case we need to modify the event but
	 * only one plugin can match by its identifier. For example, the
	 * following plugins are loaded:
	 *
	 * - ask
	 * - hangman
	 * - logger
	 *
	 * If the message is "!ask will I be reach?" then it will invoke
	 * onMessage for hangman and logger but onCommand for ask. As such call
	 * hangman and logger first and modify event before ask.
	 */
	LL_FOREACH_SAFE(bot.plugins, p, ptmp) {
		if (is_command(p, ev))
			plgcmd = p;
		else if (invokable(p, ev))
			irc_plugin_handle(p, ev);
	}

	if (plgcmd && invokable(plgcmd, ev))
		irc_plugin_handle(plgcmd, to_command(plgcmd, ev));
}

void
irc_bot_finish(void)
{
	struct irc_plugin_loader *ld, *ldtmp;

	/*
	 * First remove all loaders to make sure plugins won't try to load
	 * new plugins.
	 */
	LL_FOREACH_SAFE(bot.plugin_loaders, ld, ldtmp)
		irc_plugin_loader_finish(ld);

	bot.plugin_loaders = NULL;

	irc_bot_server_clear();
	irc_bot_plugin_clear();
	irc_bot_hook_clear();
	irc_bot_rule_clear();
}
