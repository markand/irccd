/*
 * transport.c -- remote command support
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

#include <sys/socket.h>
#include <sys/types.h>
#include <assert.h>
#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/un.h>
#include <unistd.h>

#include <config.h>

#include <irccd/log.h>
#include <irccd/transport.h>
#include <irccd/util.h>

#include "peer.h"

static struct sockaddr_un addr;
static int fd = -1;

int
transport_bind(const char *path)
{
	assert(path);

	addr.sun_family = PF_LOCAL;

	if (strlcpy(addr.sun_path, path, sizeof (addr.sun_path)) >= sizeof (addr.sun_path)) {
		errno = ENAMETOOLONG;
		goto err;
	}

	/* Silently remove the file first. */
	unlink(addr.sun_path);

	if ((fd = socket(PF_LOCAL, SOCK_STREAM, 0)) < 0)
		goto err;
	if (bind(fd, (const struct sockaddr *)&addr, sizeof (addr)) < 0)
		goto err;
	if (listen(fd, 16) < 0)
		goto err;

	irc_log_info("transport: listening on %s", path);
	irc_log_debug("transport: file descriptor %d", fd);

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
transport_prepare(struct pollfd *pfd)
{
	assert(pfd);

	if (fd < 0)
		return;

	pfd->fd = fd;
	pfd->events = POLLIN;
}

struct peer *
transport_flush(const struct pollfd *pfd)
{
	assert(pfd);

	struct peer *peer;
	int newfd;

	if (fd < 0 || pfd->fd != fd || !(pfd->revents & POLLIN))
		return NULL;

	if ((newfd = accept(fd, NULL, NULL)) < 0) {
		irc_log_warn("transport: %s", strerror(errno));
		return NULL;
	}

	peer = peer_new(newfd);

	irc_log_info("transport: new client connected");
	peer_send(peer, "IRCCD %d.%d.%d", IRCCD_VERSION_MAJOR,
	    IRCCD_VERSION_MINOR, IRCCD_VERSION_PATCH);

	return peer;
}

void
transport_finish(void)
{
	/* Connection socket. */
	if (fd != -1)
		close(fd);

	unlink(addr.sun_path);
	memset(&addr, 0, sizeof (addr));
}
