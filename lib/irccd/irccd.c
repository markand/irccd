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
#include <poll.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "event.h"
#include "irccd.h"
#include "log.h"
#include "plugin.h"
#include "server.h"
#include "peer.h"
#include "transport.h"
#include "util.h"

/* TODO: TMP */
#include <stdio.h>

#define APPEND(a, l, o, f)                                              \
do {                                                                    \
        a = irc_util_reallocarray(a, ++l, sizeof (*o));                 \
        memcpy(&a[l - 1], o, sizeof (*o));                              \
        qsort(a, l, sizeof (*o), f);                                    \
} while (0)

#define REMOVE(a, l, f)                                                 \
do {                                                                    \
        if (--l == 0) {                                                 \
                free(a);                                                \
                a = NULL;                                               \
        } else {                                                        \
                qsort(a, l + 1, sizeof (*a), f);                        \
                a = irc_util_reallocarray(a, --l, sizeof (*a));         \
        }                                                               \
} while (0)

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
cmp_server(const void *d1, const void *d2)
{
	return strcmp(
		((const struct irc_server *)d1)->name,
		((const struct irc_server *)d2)->name
	);
}

static int
cmp_plugin(const void *d1, const void *d2)
{
	return strcmp(
		((const struct irc_plugin *)d1)->name,
		((const struct irc_plugin *)d2)->name
	);
}

static int
cmp_peer(const void *d1, const void *d2)
{
	return ((const struct irc_peer *)d2)->fd - ((const struct irc_peer *)d1)->fd;
}

static int
cmp_server_name(const void *d1, const void *d2)
{
	return strcmp(d1, ((const struct irc_server *)d2)->name);
}

static int
cmp_plugin_name(const void *d1, const void *d2)
{
	return strcmp(d1, ((const struct irc_plugin *)d2)->name);
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

	for (size_t s = 0; s < irc.serversz; ++s)
		irc_server_prepare(&irc.servers[s], &pkg.fds[i++]);

	return pkg;
}

static void
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

	if (poll(pkg->fds, pkg->fdsz, 1000) < 0)
		err(1, "poll");

	/*
	 * We can't to what file descriptors belong to so pass every file to
	 * all services and they must check if they are associated to it or
	 * not.
	 */
	for (size_t i = 0; i < pkg->fdsz; ++i) {
		struct irc_peer peer;

		pipe_flush(&pkg->fds[i]);

		for (size_t s = 0; s < irc.serversz; ++s)
			irc_server_flush(&irc.servers[s], &pkg->fds[i]);

		/* Accept new transport client. */
		if (irc_transport_flush(&pkg->fds[i], &peer))
			APPEND(irc.peers, irc.peersz, &peer, cmp_peer);

		/* Flush clients. */
		for (size_t p = 0; p < irc.peersz; ++p) {
			if (!irc_peer_flush(&irc.peers[p], &pkg->fds[i])) {
				irc_peer_finish(&irc.peers[p]);
				REMOVE(irc.peers, irc.peersz, cmp_peer);
			}
		}
	}

	/*
	 * For every server, poll any kind of new event and pass them to the
	 * plugin unless the rules explicitly disallow us to do so.
	 */
	for (size_t s = 0; s < irc.serversz; ++s) {
		struct irc_event ev;

		while (irc_server_poll(&irc.servers[s], &ev))
			invoke(&ev);
	}
}

static void
clean(struct pkg *pkg)
{
	free(pkg->fds);
}

void
irc_init(void)
{
	irc_log_to_console();

	if (pipe(pipes) < 0)
		err(1, "pipe");
}

void
irc_add_server(const struct irc_server *s)
{
	assert(s);

	APPEND(irc.servers, irc.serversz, s, cmp_server);

	irc_server_connect(&irc.servers[irc.serversz - 1]);
}

struct irc_server *
irc_find_server(const char *name)
{
	return bsearch(name, irc.servers, irc.serversz, sizeof (*irc.servers),
	    cmp_server_name);
}

void
irc_del_server(const char *name)
{
	struct irc_server *s;

	if (!(s = irc_find_server(name)))
		return;

	irc_server_disconnect(s);

	/* Don't forget to notify plugins. */
	invoke(&(const struct irc_event) {
		.type = IRC_EVENT_DISCONNECT,
		.server = s
	});

	/* Finally remove from array. */
	REMOVE(irc.servers, irc.serversz, cmp_server);
}

void
irc_add_plugin(const struct irc_plugin *p)
{
	assert(p);

	APPEND(irc.plugins, irc.pluginsz, p, cmp_plugin);

	irc_log_info("plugin %s: %s", p->name, p->description);
	irc_log_info("plugin %s: version %s, from %s (%s license)", p->name,
	    p->version, p->author, p->license);

	irc_plugin_load(&irc.plugins[irc.pluginsz - 1]);
}

struct irc_plugin *
irc_find_plugin(const char *name)
{
	return bsearch(name, irc.plugins, irc.pluginsz, sizeof (*irc.plugins),
	    cmp_plugin_name);
}

void
irc_del_plugin(const char *name)
{
	struct irc_plugin *p;

	if (!(p = irc_find_plugin(name)))
		return;

	irc_plugin_unload(p);
	irc_plugin_finish(p);

	REMOVE(irc.plugins, irc.pluginsz, cmp_plugin);
}

void
irc_post(void (*exec)(void *), void *data)
{
	struct defer df = {
		.exec = exec,
		.data = data
	};

	if (write(pipes[1], &df, sizeof (df)) != sizeof (df))
		err(1, "write");
}

void
irc_run(void)
{
	struct pkg pkg;

	for (;;) {
		pkg = prepare();
		process(&pkg);
		clean(&pkg);
	}
}
