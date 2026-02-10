/*
 * transport.c -- remote command support
 *
 * Copyright (c) 2013-2026 David Demelier <markand@malikania.fr>
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

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <assert.h>
#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/un.h>
#include <unistd.h>

#include <utlist.h>

#include <nce/io.h>

#include <irccd/config.h>
#include <irccd/log.h>
#include <irccd/util.h>

#include "peer.h"
#include "transport.h"

static struct sockaddr_un addr;
static int fd = -1;
static struct nce_io_coro fd_co;
static struct peer *peers;

static void
transport_entry(struct nce_coro *)
{
	struct peer *peer, *tmp;
	int clt;

	while (nce_io_wait(&fd_co.io)) {
		if ((clt = accept(fd, NULL, 0)) < 0) {
			switch (errno) {
			case -EINTR:
				irc_log_debug("transport: accept: %s", strerror(errno));
				/* Ignore. */
				break;
			}
		} else {
			irc_log_debug("transport: new client (%d)", clt);
			peer = peer_new(clt);
			LL_APPEND(peers, peer);
		}

		/* Cleanup zombies. */
		LL_FOREACH_SAFE(peers, peer, tmp) {
			if (!nce_stream_active(&peer->stream.stream)) {
				irc_log_debug("transport: reap client (%d)", peer->fd);
				LL_DELETE(peers, peer);
				peer_free(peer);
			}
		}
	}
}

int
transport_start(const char *path, long long uid, long long gid)
{
	assert(path);

	int oldumask;

	addr.sun_family = AF_UNIX;

	if (irc_util_strlcpy(addr.sun_path, path, sizeof (addr.sun_path)) >= sizeof (addr.sun_path)) {
		errno = ENAMETOOLONG;
		goto err;
	}

	/* Silently remove the file first. */
	unlink(path);

	if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
		goto err;

	/* -ux, -gx, -orwx */
	oldumask = umask(S_IXUSR | S_IXGRP | S_IRWXO);

	if (bind(fd, (const struct sockaddr *)&addr, sizeof (addr)) < 0)
		goto err;
	if (uid != -1 && gid != -1 && chown(path, uid, gid) < 0)
		goto err;

	umask(oldumask);

	if (listen(fd, 16) < 0)
		goto err;

	irc_log_info("transport: listening on %s", path);

	if (uid != -1 && gid != -1)
		irc_log_info("transport: uid=%lld, gid=%lld", uid, gid);

	fd_co.coro.name = "transport.entry";
	fd_co.coro.entry = transport_entry;
	nce_io_coro_spawn(&fd_co, fd, EV_READ);

	return 0;

err:
	irc_log_warn("transport: %s: %s", path, strerror(errno));

	if (fd != -1) {
		close(fd);
		fd = -1;
	}

	return -1;
}

void
transport_broadcast(const char *data)
{
	assert(data);

	struct peer *peer;

	LL_FOREACH(peers, peer)
		if (peer->is_watching)
			peer_push(peer, "%s", data);
}

void
transport_stop(void)
{
	struct peer *peer, *tmp;

	nce_coro_destroy(&fd_co.coro);

	/* Connection socket. */
	if (fd != -1)
		close(fd);

	unlink(addr.sun_path);

	LL_FOREACH_SAFE(peers, peer, tmp) {
		LL_DELETE(peers, peer);
		peer_free(peer);
	}
}
