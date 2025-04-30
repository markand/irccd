/*
 * main.c -- irccd(1) main file
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

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <ev.h>

#include <utlist.h>

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

static struct ev_loop *loop;
static struct ev_signal sig_int;
static struct ev_signal sig_term;
static const char *config = IRCCD_SYSCONFDIR "/irccd.conf";
static struct peer *peers;

/* conf.y */
void
config_open(const char *);

#if 0
static void
broadcast(const struct irc_event *ev)
{
	char buf[IRC_BUF_LEN];
	struct peer *p;

	if (!irc_event_str(ev, buf, sizeof (buf)))
		return;

	LL_FOREACH(peers, p)
		if (p->is_watching)
			peer_push(p, "%s", buf);
}
#endif

static void
run_info(void)
{
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

	printf("%-16s%s\n", "javascript:", with_js);
	printf("%-16s%s\n", "ssl:", with_ssl);
}

static void
run_paths(void)
{
	struct irc_plugin_loader *ld;
	char paths[IRC_PATHS_LEN], extensions[IRC_EXTENSIONS_LEN], *p, *token;

	printf("%-16s%s\n", "cache:", IRCCD_CACHEDIR);
	printf("%-16s%s\n", "config:", IRCCD_SYSCONFDIR);
	printf("%-16s%s\n", "lib:", IRCCD_LIBDIR);
	printf("%-16s%s\n", "data:", IRCCD_DATADIR);
	printf("\n");

	LL_FOREACH(irccd->plugin_loaders, ld) {
		printf("Plugins with extensions:");
		irc_util_strlcpy(extensions, ld->extensions, sizeof (extensions));
		irc_util_strlcpy(paths, ld->paths, sizeof (paths));

		for (p = extensions; (token = strtok_r(p, ":", &p)); )
			printf(" %s", token);

		printf("\n");

		for (p = paths; (token = strtok_r(p, ":", &p)); )
			printf("  %s\n", token);

		if (ld->next)
			printf("\n");
	}
}

static void
run_version(void)
{
	puts(IRCCD_VERSION);
}

static int
run(int argc, char **argv)
{
	(void)argc;

	static struct {
		const char *name;
		void (*exec)(void);
	} cmds[] = {
		{ "info",       run_info        },
		{ "paths",      run_paths       },
		{ "version",    run_version     },
		{ NULL,         NULL            }
	};

	for (size_t i = 0; cmds[i].name; ++i) {
		if (strcmp(cmds[i].name, argv[0]) == 0) {
			cmds[i].exec();
			return 0;
		}
	}

	irc_util_die("abort: unknown command: %s\n", argv[0]);

	return 1;
}

static void
sig_cb(struct ev_loop *loop, struct ev_signal *self, int revents)
{
	(void)loop;
	(void)revents;

	irc_log_info("irccd: stopping on signal %d", self->signum);
	ev_break(loop, EVBREAK_ALL);
}

static void
init(void)
{
	loop = ev_default_loop(0);

	irc_bot_init(loop);
	irc_bot_plugin_loader_add(dl_plugin_loader_new());

#if defined(IRCCD_WITH_JS)
	irc_bot_plugin_loader_add(js_plugin_loader_new());
#endif

	ev_signal_init(&sig_int, sig_cb, SIGINT);
	ev_signal_init(&sig_term, sig_cb, SIGTERM);
	ev_signal_start(loop, &sig_int);
	ev_signal_start(loop, &sig_term);
}

static inline void
load(void)
{
	config_open(config);
}

static inline void
finish(void)
{
	struct peer *peer, *tmp;

	LL_FOREACH_SAFE(peers, peer, tmp)
		peer_free(peer);

	transport_finish();
	irc_bot_finish();
}

_Noreturn static void
usage(void)
{
	fprintf(stderr, "usage: irccd [-v] [-c config]\n");
	fprintf(stderr, "       irccd info\n");
	fprintf(stderr, "       irccd paths\n");
	fprintf(stderr, "       irccd version\n");
	exit(1);
}

int
main(int argc, char **argv)
{
	int ch, verbose = 0;

	while ((ch = getopt(argc, argv, "c:v")) != -1) {
		switch (ch) {
		case 'c':
			config = optarg;
			break;
		case 'v':
			verbose = 1;
			break;
		default:
			usage();
			break;
			/* NOTREACHED */
		}
	}

	argc -= optind;
	argv += optind;

	init();

	if (argc > 0)
		return run(argc, argv);

	load();

	/* We apply now so it overrides configuration file. */
	if (verbose)
		irc_log_set_verbose(1);

	ev_loop(loop, 0);
	finish();
}
