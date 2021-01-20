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

#include <assert.h>
#include <err.h>
#include <errno.h>
#include <poll.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "event.h"
#include "irccd.h"
#include "list.h"
#include "log.h"
#include "peer.h"
#include "plugin.h"
#include "rule.h"
#include "server.h"
#include "set.h"
#include "transport.h"
#include "util.h"

struct pkg {
	struct pollfd *fds;
	size_t fdsz;
};

struct defer {
	void (*exec)(void *);
	void (*data);
};

struct irc irc;

static int pipes[2];

static int
cmp_plugin(const struct irc_plugin *p1, const struct irc_plugin *p2)
{
	return strcmp(p1->name, p2->name);
}

static int
cmp_peer(const struct irc_peer *p1, const struct irc_peer *p2)
{
	return p1->fd - p2->fd;
}

static struct pkg
prepare(void)
{
	struct pkg pkg = {0};
	size_t i = 0;

	pkg.fdsz += 1;                  /* pipe  */
	pkg.fdsz += 1;                  /* transport fd */
	pkg.fdsz += irc.serversz;       /* servers */
	pkg.fdsz += irc.peersz;         /* transport peers */

	pkg.fds = irc_util_calloc(pkg.fdsz, sizeof (*pkg.fds));

	/* pipe */
	pkg.fds[i].fd = pipes[0];
	pkg.fds[i++].events = POLLIN;

	/* transport */
	irc_transport_prepare(&pkg.fds[i++]);

	for (size_t p = 0; p < irc.peersz; ++p)
		irc_peer_prepare(&irc.peers[p], &pkg.fds[i++]);
	for (struct irc_server *s = irc.servers; s; s = s->next)
		irc_server_prepare(s, &pkg.fds[i++]);

	return pkg;
}

static inline void
broadcast(const struct irc_event *ev)
{
	char buf[IRC_MESSAGE_MAX];

	if (!irc_event_str(ev, buf, sizeof (buf)))
		return;

	for (size_t i = 0; i < irc.peersz; ++i)
		if (irc.peers[i].is_watching)
			irc_peer_send(&irc.peers[i], buf);
}

static inline void
invoke(const struct irc_event *ev)
{
	for (size_t i = 0; i < irc.pluginsz; ++i)
		irc_plugin_handle(&irc.plugins[i], ev);
}

static void
pipe_flush(struct pollfd *fd)
{
	struct defer df = {0};

	if (fd->fd != pipes[0] || !(fd->revents & POLLIN))
		return;

	if (read(fd->fd, &df, sizeof (df)) != sizeof (df))
		err(1, "read");

	df.exec(df.data);
}

static void
process(struct pkg *pkg)
{
	struct irc_server *s;
	struct irc_peer peer;
	struct irc_event ev;

	if (poll(pkg->fds, pkg->fdsz, 1000) < 0 && errno != EINTR)
		err(1, "poll");

	/*
	 * We can't to what file descriptors belong to so pass every file to
	 * all services and they must check if they are associated to it or
	 * not.
	 */
	for (size_t i = 0; i < pkg->fdsz; ++i) {
		pipe_flush(&pkg->fds[i]);

		IRC_LIST_FOREACH(irc.servers, s)
			irc_server_flush(s, &pkg->fds[i]);

		/* Accept new transport client. */
		if (irc_transport_flush(&pkg->fds[i], &peer))
			IRC_SET_ALLOC_PUSH(&irc.peers, &irc.peersz, &peer, cmp_peer);

		/* Flush clients. */
		for (size_t p = 0; p < irc.peersz; ) {
			if (!irc_peer_flush(&irc.peers[p], &pkg->fds[i])) {
				irc_peer_finish(&irc.peers[p]);
				IRC_SET_ALLOC_REMOVE(&irc.peers, &irc.peersz, &irc.peers[p]);
			} else
				++p;
		}
	}

	/*
	 * For every server, poll any kind of new event and pass them to the
	 * plugin unless the rules explicitly disallow us to do so.
	 */
	IRC_LIST_FOREACH(irc.servers, s) {
		while (irc_server_poll(s, &ev)) {
			broadcast(&ev);
			invoke(&ev);
		}
	}
}

static void
clean(struct pkg *pkg)
{
	free(pkg->fds);
}

void
irc_bot_init(void)
{
	irc_log_to_console();

	if (pipe(pipes) < 0)
		err(1, "pipe");
}

void
irc_bot_add_server(struct irc_server *s)
{
	assert(s);

	irc_server_incref(s);
	irc_server_connect(s);

	IRC_LIST_ADD(irc.servers, s);

	irc.serversz++;
}

struct irc_server *
irc_bot_find_server(const char *name)
{
	struct irc_server *s;

	for (s = irc.servers; s; s = s->next)
		if (strcmp(s->name, name) == 0)
			return s;

	return NULL;
}

void
irc_bot_remove_server(const char *name)
{
	struct irc_server *s;

	if (!(s = irc_bot_find_server(name)))
		return;

	irc_server_disconnect(s);

	/* Don't forget to notify plugins. */
	invoke(&(const struct irc_event) {
		.type = IRC_EVENT_DISCONNECT,
		.server = s
	});

	IRC_LIST_REMOVE(irc.servers, s);

	irc_server_decref(s);
	irc.serversz--;
}

void
irc_bot_clear_servers(void)
{
	struct irc_server *s, *next;

	IRC_LIST_FOREACH_SAFE(irc.servers, s, next)
		irc_bot_remove_server(s->name);
}

void
irc_bot_add_plugin(const struct irc_plugin *p)
{
	assert(p);

	IRC_SET_ALLOC_PUSH(&irc.plugins, &irc.pluginsz, p, cmp_plugin);

	irc_log_info("plugin %s: %s", p->name, p->description);
	irc_log_info("plugin %s: version %s, from %s (%s license)", p->name,
	    p->version, p->author, p->license);

	irc_plugin_load(&irc.plugins[irc.pluginsz - 1]);
}

struct irc_plugin *
irc_bot_find_plugin(const char *name)
{
	struct irc_plugin key = {0};

	strlcpy(key.name, name, sizeof (key.name));

	return IRC_SET_FIND(irc.plugins, irc.pluginsz, &key, cmp_plugin);
}

void
irc_bot_remove_plugin(const char *name)
{
	struct irc_plugin *p;

	if (!(p = irc_bot_find_plugin(name)))
		return;

	irc_plugin_unload(p);
	irc_plugin_finish(p);

	IRC_SET_ALLOC_REMOVE(&irc.plugins, &irc.pluginsz, p);
}

bool
irc_bot_insert_rule(const struct irc_rule *rule, size_t i)
{
	assert(rule);

	if (irc.rulesz >= IRC_RULE_MAX) {
		errno = ENOMEM;
		return false;
	}

	if (i >= irc.rulesz)
		i = irc.rulesz;

	memmove(&irc.rules[i + 1], &irc.rules[i], sizeof (*irc.rules) * (irc.rulesz++ - i));
	memcpy(&irc.rules[i], rule, sizeof (*rule));

	return true;
}

void
irc_bot_remove_rule(size_t i)
{
	assert(i < irc.rulesz);

	if (i + 1 >= irc.rulesz)
		irc.rulesz--;
	else
		memmove(&irc.rules[i], &irc.rules[i + 1], sizeof (*irc.rules) * (irc.rulesz-- - i));
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
irc_bot_run(void)
{
	struct pkg pkg;

	for (;;) {
		pkg = prepare();
		process(&pkg);
		clean(&pkg);
	}
}
