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

#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <irccd/config.h>
#include <irccd/irccd.h>
#include <irccd/log.h>
#include <irccd/plugin.h>
#include <irccd/rule.h>
#include <irccd/server.h>
#include <irccd/util.h>

#include "dl-plugin.h"

#include "peer.h"
#include "transport.h"

#if defined(IRCCD_WITH_JS)
#       include "js-plugin.h"
#endif

struct pollables {
	struct pollfd *fds;
	size_t fdsz;
	size_t botsz;
	size_t localsz;
};

static const char *config = IRCCD_SYSCONFDIR "/irccd.conf";
static struct peers peers = LIST_HEAD_INITIALIZER();
static int running = 1;

/* conf.y */
void
config_open(const char *);

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

	LIST_FOREACH(p, &peers, link)
		++i;

	return i;
}

static int
run(int argc, char **argv)
{
	(void)argc;

#if defined(IRCCD_WITH_JS)
	const char *with_js = "yes";
#else
	const char *with_js = "no";
#endif
#if defined(IRCCD_WITH_SSL)
	const char *with_ssl = "yes";
#else
	const char *with_ssl = "no";
#endif

	if (strcmp(argv[0], "version") == 0)
		puts(IRCCD_VERSION);
	else if (strcmp(argv[0], "info") == 0) {
		printf("%-16s%s\n", "javascript:", with_js);
		printf("%-16s%s\n", "ssl:", with_ssl);
	}

	return 0;
}

static void
stop(int signum)
{
	irc_log_info("irccd: stopping on signal %d", signum);
	running = 0;
}

static void
init(void)
{
	struct sigaction sig;

	irc_bot_init();
	irc_bot_plugin_loader_add(dl_plugin_loader_new());

#if defined(IRCCD_WITH_JS)
	irc_bot_plugin_loader_add(js_plugin_loader_new());
#endif

	sig.sa_handler = stop;
	sig.sa_flags = SA_RESTART;
	sigemptyset(&sig.sa_mask);

	if (sigaction(SIGINT, &sig, NULL) < 0 || sigaction(SIGTERM, &sig, NULL) < 0)
		irc_util_die("sigaction: %s\n", strerror(errno));
}

static void
prepare(struct pollables *pb)
{
	struct peer *p;
	struct pollfd *fd = pb->fds + pb->botsz;

	irc_bot_prepare(pb->fds);
	transport_prepare(fd++);

	LIST_FOREACH(p, &peers, link)
		peer_prepare(p, fd++);
}

static void
flush(const struct pollables *pb)
{
	struct peer *peer, *tmp;
	struct pollfd *fd = pb->fds + pb->botsz + 1;

	irc_bot_flush(pb->fds);

	LIST_FOREACH_SAFE(peer, &peers, link, tmp) {
		if (peer_flush(peer, fd++) < 0) {
			LIST_REMOVE(peer, link);
			peer_finish(peer);
		}
	}

	/*
	 * Add a new client only now because we would iterate over a list
	 * of pollfd that is smaller than the client list.
	 */
	if ((peer = transport_flush(pb->fds + pb->botsz)))
		LIST_INSERT_HEAD(&peers, peer, link);
}

static inline void
load(void)
{
	config_open(config);
}

static struct pollables
pollables(void)
{
	struct pollables pb = {0};

	pb.botsz = irc_bot_poll_count();
	pb.localsz = poll_count();
	pb.fdsz = pb.botsz + pb.localsz;
	pb.fds = irc_util_calloc(pb.fdsz, sizeof (*pb.fds));

	prepare(&pb);

	return pb;
}

static void
loop(void)
{
	struct pollables pb;
	struct irc_event ev;

	while (running) {
		pb = pollables();

		if (poll(pb.fds, pb.fdsz, 1000) < 0 && errno != EINTR)
			irc_util_die("poll: %s\n", strerror(errno));

		flush(&pb);

		while (irc_bot_dequeue(&ev)) {
			broadcast(&ev);
			irc_event_finish(&ev);
		}

		free(pb.fds);
	}
}

static inline void
finish(void)
{
	struct peer *peer, *tmp;

	LIST_FOREACH_SAFE(peer, &peers, link, tmp)
		peer_finish(peer);

	transport_finish();
	irc_bot_finish();
}

static void
usage(void)
{
	fprintf(stderr, "usage: %s [-c config]\n", getprogname());
	exit(1);
}

int
main(int argc, char **argv)
{
	int ch;

	setprogname("irccd");

	while ((ch = getopt(argc, argv, "c:")) != -1) {
		switch (ch) {
		case 'c':
			config = optarg;
			break;
		default:
			usage();
			break;
			/* NOTREACHED */
		}
	}

	argc -= optind;
	argv += optind;

	if (argc > 0)
		return run(argc, argv);

	init();
	load();
	loop();
	finish();
}
