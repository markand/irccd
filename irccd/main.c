/*
 * main.c -- irccd(1) main file
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

#include <poll.h>
#include <stdio.h>
#include <err.h>

#include <irccd/irccd.h>
#include <irccd/log.h>
#include <irccd/server.h>
#include <irccd/transport.h>
#include <irccd/util.h>
#include <irccd/plugin.h>
#include <irccd/rule.h>

#include "dl-plugin.h"
#include "js-plugin.h"
#include "peer.h"

static struct peers peers;
static int running = 1;
static int has_transport;

static void
broadcast(const struct irc_event *ev)
{
	char buf[IRC_BUF_LEN];
	struct peer *p;

	if (!irc_event_str(ev, buf, sizeof (buf)))
		return;

	LIST_FOREACH(p, &peers, link)
		if (p->is_watching)
			peer_send(p, buf);
}

static inline size_t
poll_count(void)
{
	struct peer *p;
	size_t i = 1;

	if (!has_transport)
		return 0;

	LIST_FOREACH(p, &peers, link)
		++i;

	return i;
}

static int
run(int argc, char **argv)
{
	(void)argc;

	if (strcmp(argv[0], "version") == 0)
		puts(IRCCD_VERSION);

	return 0;
}

static inline void
init(void)
{
	irc_bot_init();
}

static void
prepare(struct pollfd *fd)
{
	struct peer *p;

	if (!has_transport)
		return;

	transport_prepare(fd++);

	LIST_FOREACH(p, &peers, link)
		peer_prepare(p, fd++);
}

static void
flush(const struct pollfd *fd)
{
	struct peer *peer, *tmp;

	if (!has_transport)
		return;

	transport_flush(fd++);

	LIST_FOREACH_SAFE(peer, &peers, link, tmp) {
		if (peer_flush(peer, fd++) < 0) {
			LIST_REMOVE(peer, link);
			peer_finish(peer);
		}
	}
}

static void
loop(void)
{
	struct irc_event ev;
	struct pollfd *fds;
	size_t botcount, owncount;

	while (running) {
		/*
		 * Compute how much fd the bot requires and append our own
		 * transport and peers.
		 */
		botcount = irc_bot_poll_count();
		owncount = poll_count();
		fds = irc_util_calloc(botcount + owncount, sizeof (*fds));

		irc_bot_prepare(fds);
		prepare(&fds[botcount]);

		irc_bot_flush(fds);
		flush(&fds[botcount]);

		while (irc_bot_dequeue(&ev))
			broadcast(&ev);

		free(fds);
	}
}

static inline void
finish(void)
{
	struct peer *peer, *tmp;

	LIST_FOREACH_SAFE(peer, &peers, link, tmp)
		peer_finish(peer);

	transport_finish();
}

int
main(int argc, char **argv)
{
	--argc;
	++argv;

	if (argc > 0)
		return run(argc, argv);

	init();
	loop();
	finish();
}
