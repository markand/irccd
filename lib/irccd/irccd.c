/*
 * irccd.c -- main irccd object
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

#include <config.h>

#include <assert.h>
#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <poll.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "event.h"
#include "irccd.h"
#include "log.h"
#include "plugin.h"
#include "rule.h"
#include "server.h"
#include "util.h"

struct defer {
	void (*exec)(void *);
	void *data;
};

struct irc irc = {
	.servers = LIST_HEAD_INITIALIZER(),
	.plugins = LIST_HEAD_INITIALIZER(),
	.rules = TAILQ_HEAD_INITIALIZER(irc.rules)
};

static int pipes[2];

static bool
is_command(const struct irc_plugin *p, const struct irc_event *ev)
{
	const char *cc;
	size_t ccsz;

	if (ev->type != IRC_EVENT_MESSAGE)
		return false;

	/* Get the command prefix (e.g !)*/
	cc = ev->server->commandchar;
	ccsz = strlen(cc);

	return strncmp(ev->message.message, cc, ccsz) == 0 &&
	       strncmp(ev->message.message + ccsz, p->name, strlen(p->name)) == 0;
}

static struct irc_event *
to_command(const struct irc_plugin *p, struct irc_event *ev)
{
	char *action;

	/* Convert "!test foo bar" to "foo bar" */
	action = ev->message.message + strlen(ev->server->commandchar) + strlen(p->name);

	while (*action && isspace(*action))
		++action;

	action = strdup(action);
	free(ev->message.message);

	ev->type = IRC_EVENT_COMMAND;
	ev->message.message = action;

	return ev;
}

static bool
invokable(const struct irc_plugin *p, const struct irc_event *ev)
{
	switch (ev->type) {
	case IRC_EVENT_COMMAND:
		return irc_rule_matchlist(&irc.rules, ev->server->name,
		    ev->message.channel, ev->message.origin, p->name, "onCommand");
	case IRC_EVENT_CONNECT:
		return irc_rule_matchlist(&irc.rules, ev->server->name,
		    NULL, NULL, p->name, "onConnect");
	case IRC_EVENT_DISCONNECT:
		return irc_rule_matchlist(&irc.rules, ev->server->name,
		    NULL, NULL, p->name, "onDisconnect");
	case IRC_EVENT_INVITE:
		return irc_rule_matchlist(&irc.rules, ev->server->name,
		    ev->invite.channel, ev->invite.origin, p->name, "onInvite");
	case IRC_EVENT_JOIN:
		return irc_rule_matchlist(&irc.rules, ev->server->name,
		    ev->join.channel, ev->join.origin, p->name, "onJoin");
	case IRC_EVENT_KICK:
		return irc_rule_matchlist(&irc.rules, ev->server->name,
		    ev->kick.channel, ev->kick.origin, p->name, "onKick");
		break;
	case IRC_EVENT_ME:
		return irc_rule_matchlist(&irc.rules, ev->server->name,
		    ev->message.channel, ev->message.origin, p->name, "onMe");
	case IRC_EVENT_MESSAGE:
		return irc_rule_matchlist(&irc.rules, ev->server->name,
		    ev->message.channel, ev->message.origin, p->name, "onMessage");
	case IRC_EVENT_MODE:
		return irc_rule_matchlist(&irc.rules, ev->server->name,
		    ev->mode.channel, ev->mode.origin, p->name, "onMode");
	case IRC_EVENT_NAMES:
		return irc_rule_matchlist(&irc.rules, ev->server->name,
		    ev->names.channel, NULL, p->name, "onNames");
	case IRC_EVENT_NICK:
		return irc_rule_matchlist(&irc.rules, ev->server->name,
		    NULL, ev->nick.origin, p->name, "onNick");
	case IRC_EVENT_NOTICE:
		return irc_rule_matchlist(&irc.rules, ev->server->name,
		    ev->notice.channel, ev->notice.origin, p->name, "onNotice");
	case IRC_EVENT_PART:
		return irc_rule_matchlist(&irc.rules, ev->server->name,
		    ev->part.channel, ev->part.origin, p->name, "onPart");
	case IRC_EVENT_TOPIC:
		return irc_rule_matchlist(&irc.rules, ev->server->name,
		    ev->topic.channel, ev->topic.origin, p->name, "onTopic");
	case IRC_EVENT_WHOIS:
		return irc_rule_matchlist(&irc.rules, ev->server->name,
		    NULL, NULL, p->name, "onWhois");
	default:
		return true;
	}
}

static void
invoke(struct irc_event *ev)
{
	struct irc_plugin *p, *tmp, *plgcmd = NULL;

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
	LIST_FOREACH_SAFE(p, &irc.plugins, link, tmp) {
		if (is_command(p, ev))
			plgcmd = p;
		else if (invokable(p, ev))
			irc_plugin_handle(p, ev);
	}

	if (plgcmd && invokable(plgcmd, ev))
		irc_plugin_handle(plgcmd, to_command(plgcmd, ev));
}

static void
pipe_flush(const struct pollfd *fd)
{
	struct defer df = {0};

	if (fd->fd != pipes[0] || !(fd->revents & POLLIN))
		return;

	if (read(fd->fd, &df, sizeof (df)) != sizeof (df))
		err(1, "read");

	df.exec(df.data);
}

static struct irc_plugin *
find_plugin(struct irc_plugin_loader *ldr, const char *base, const char *name)
{
	char path[PATH_MAX], buf[IRC_EXTENSIONS_LEN], *t, *ext;
	struct irc_plugin *p;

	strlcpy(buf, ldr->extensions, sizeof (buf));

	for (t = buf; (ext = strtok_r(t, ":", &t)); ) {
		snprintf(path, sizeof (path), "%s/%s.%s", base, name, ext);
		irc_log_info("irccd: trying %s", path);

		if ((p = irc_plugin_loader_open(ldr, path)))
			return p;
	}

	return NULL;
}

static inline struct irc_plugin *
open_plugin(struct irc_plugin_loader *ldr, const char *path)
{
	return irc_plugin_loader_open(ldr, path);
}

static inline size_t
rulescount(void)
{
	const struct irc_rule *r;
	size_t total = 0;

	TAILQ_FOREACH(r, &irc.rules, link)
		total++;

	return total;
}

void
irc_bot_init(void)
{
	irc_log_to_console();

	if (pipe(pipes) < 0)
		err(1, "pipe");
}

void
irc_bot_server_add(struct irc_server *s)
{
	assert(s);

	irc_log_info("irccd: added new server: %s", s->name);

	irc_server_incref(s);
	irc_server_connect(s);

	LIST_INSERT_HEAD(&irc.servers, s, link);
}

struct irc_server *
irc_bot_server_get(const char *name)
{
	struct irc_server *s;

	LIST_FOREACH(s, &irc.servers, link)
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
	invoke(&(struct irc_event) {
		.type = IRC_EVENT_DISCONNECT,
		.server = s
	});

	LIST_REMOVE(s, link);
	irc_server_decref(s);
}

void
irc_bot_server_clear(void)
{
	struct irc_server *s, *tmp;

	LIST_FOREACH_SAFE(s, &irc.servers, link, tmp)
		irc_bot_server_remove(s->name);
	LIST_INIT(&irc.servers);
}

void
irc_bot_plugin_add(struct irc_plugin *p)
{
	assert(p);

	LIST_INSERT_HEAD(&irc.plugins, p, link);

	irc_log_info("irccd: add new plugin: %s", p->name, p->description);
	irc_log_info("irccd: %s: version %s, from %s (%s license)", p->name,
	    p->version, p->author, p->license);

	irc_plugin_load(p);
}

struct irc_plugin *
irc_bot_plugin_find(const char *name, const char *path)
{
	assert(name);

	char buf[IRC_PATHS_LEN], pathbuf[PATH_MAX], *t, *token;
	struct irc_plugin *p = NULL;
	struct irc_plugin_loader *ldr;

	if (!path)
		irc_log_info("irccd: trying to find plugin %s", name);
	else
		irc_log_info("irccd: opening plugin %s", name);

	SLIST_FOREACH(ldr, &irc.plugin_loaders, link) {
		if (path) {
			if ((p = open_plugin(ldr, path)))
				break;
		} else {
			/* Copy the paths to tokenize it. */
			strlcpy(buf, ldr->paths, sizeof (buf));

			/*
			 * For every directory (separated by colon) call find_plugin
			 * which will append the extension and try to open it.
			 */
			for (t = buf; (token = strtok_r(t, ":", &t)); ) {
				if ((p = find_plugin(ldr, token, name)))
					break;
			}
		}
	}

	if (!p)
		irc_log_warn("irccd: could not find plugin %s", name);

	strlcpy(p->name, name, sizeof (p->name));

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

	LIST_FOREACH(p, &irc.plugins, link)
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

	irc_plugin_unload(p);
	irc_plugin_finish(p);

	LIST_REMOVE(p, link);
}

void
irc_bot_plugin_loader_add(struct irc_plugin_loader *ldr)
{
	assert(ldr);

	SLIST_INSERT_HEAD(&irc.plugin_loaders, ldr, link);
}

void
irc_bot_plugin_clear(void)
{
	struct irc_plugin *p, *tmp;

	LIST_FOREACH_SAFE(p, &irc.plugins, link, tmp)
		irc_bot_plugin_remove(p->name);
	LIST_INIT(&irc.plugins);
}

void
irc_bot_rule_insert(struct irc_rule *rule, size_t index)
{
	assert(rule);

	if (index == 0)
		TAILQ_INSERT_HEAD(&irc.rules, rule, link);
	else if (index >= rulescount())
		TAILQ_INSERT_TAIL(&irc.rules, rule, link);
	else {
		struct irc_rule *pos = TAILQ_FIRST(&irc.rules);

		for (size_t i = 0; i < index; ++i)
			pos = TAILQ_NEXT(pos, link);

		TAILQ_INSERT_AFTER(&irc.rules, pos, rule, link);
	}
}

void
irc_bot_rule_remove(size_t index)
{
	assert(index < rulescount());

	struct irc_rule *pos = TAILQ_FIRST(&irc.rules);

	for (size_t i = 0; i < index; ++i)
		pos = TAILQ_NEXT(pos, link);

	TAILQ_REMOVE(&irc.rules, pos, link);
}

void
irc_bot_rule_clear(void)
{
	struct irc_rule *r, *tmp;

	TAILQ_FOREACH_SAFE(r, &irc.rules, link, tmp)
		irc_rule_finish(r);
	TAILQ_INIT(&irc.rules);
}

size_t
irc_bot_poll_count(void)
{
	size_t i = 1;
	struct irc_server *s;

	LIST_FOREACH(s, &irc.servers, link)
		++i;

	return i;
}

void
irc_bot_prepare(struct pollfd *fds)
{
	assert(fds);

	struct irc_server *s;
	size_t i = 1;

	fds[0].fd = pipes[0];
	fds[0].events = POLLIN;

	LIST_FOREACH(s, &irc.servers, link)
		irc_server_prepare(s, &fds[i++]);
}

void
irc_bot_flush(const struct pollfd *fds)
{
	assert(fds);

	struct irc_server *s;
	size_t i = 1;

	pipe_flush(&fds[0]);

	LIST_FOREACH(s, &irc.servers, link)
		irc_server_flush(s, &fds[i++]);
}

int
irc_bot_dequeue(struct irc_event *ev)
{
	struct irc_server *s;

	LIST_FOREACH(s, &irc.servers, link)
		if (irc_server_poll(s, ev))
			return 1;

	return 0;
}

void
irc_bot_post(void (*exec)(void *), void *data)
{
	struct defer df = {
		.exec = exec,
		.data = data
	};

	if (write(pipes[1], &df, sizeof (df)) != sizeof (df))
		err(1, "write");
}

void
irc_bot_finish(void)
{
	struct irc_plugin_loader *ld, *ldtmp;

	/*
	 * First remove all loaders to mkae sure plugins won't try to load
	 * new plugins.
	 */
	SLIST_FOREACH_SAFE(ld, &irc.plugin_loaders, link, ldtmp)
		irc_plugin_loader_finish(ld);

	irc_bot_server_clear();
	irc_bot_plugin_clear();
	irc_bot_rule_clear();
}
