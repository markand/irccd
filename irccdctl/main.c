/*
 * main.c -- irccdctl(1) main file
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

#include <compat.h>

#include <sys/socket.h>
#include <sys/time.h>
#include <sys/un.h>
#include <assert.h>
#include <err.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>
#include <unistd.h>

#include <stdio.h>

#include <irccd/limits.h>
#include <irccd/util.h>

static bool verbose;
static int sock;
static struct sockaddr_un sockaddr = {
	.sun_family = PF_LOCAL,
	.sun_path = "/tmp/irccd.sock"
};
static char in[IRC_BUF_LEN];
static char out[IRC_BUF_LEN];

static char *
poll(void)
{
	static char ret[IRC_BUF_LEN];
	char *nl;

	while (!(nl = strstr(in, "\n"))) {
		char buf[IRC_BUF_LEN] = {0};
		ssize_t nr;

		if ((nr = recv(sock, buf, sizeof (buf) - 1, 0)) <= 0)
			errc(1, nr == 0 ? ECONNRESET : errno, "abort");
		if (strlcat(in, buf, sizeof (in)) >= sizeof (in))
			errc(1, EMSGSIZE, "abort");
	}

	*nl = '\0';
	strlcpy(ret, in, sizeof (ret));
	memmove(in, nl + 1, sizeof (in) - (nl - in) - 1);

	return ret;
}

static void
dial(void)
{
	const struct timeval tv = {
		.tv_sec = 30
	};

	if ((sock = socket(PF_LOCAL, SOCK_STREAM, 0)) < 0)
		err(1, "socket");
	if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof (tv)) < 0 ||
	    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof (tv)) < 0)
		err(1, "setsockopt");
	if (connect(sock, (const struct sockaddr *)&sockaddr, SUN_LEN(&sockaddr)) < 0)
		err(1, "connect");
}

static void
check(void)
{
	/* Ensure we're talking to irccd. */
	int major, minor, patch;

	if ((sscanf(poll(), "IRCCD %d.%d.%d", &major, &minor, &patch) != 3))
		errx(1, "abort: not irccd instance");
	if (verbose)
		printf("connected to irccd %d.%d.%d\n", major, minor, patch);
}

static void
req(const char *fmt, ...)
{
	char buf[IRC_BUF_LEN];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof (buf), fmt, ap);
	va_end(ap);

	if (strlcat(out, buf, sizeof (out)) >= sizeof (out) ||
	    strlcat(out, "\n", sizeof (out)) >= sizeof (out))
		errc(1, EMSGSIZE, "abort");

	while (out[0]) {
		ssize_t ns, len;

		len = strlen(out);

		if ((ns = send(sock, out, len, MSG_NOSIGNAL)) <= 0)
			err(1, "send");

		if (ns >= len)
			memset(out, 0, sizeof (out));
		else
			memmove(out, out + ns, sizeof (out) - ns);
	}
}

static void
ok(void)
{
	const char *response = poll();

	if (strcmp(response, "OK") != 0)
		errx(1, "abort: %s", response);
}

static void
cmd_server_disconnect(int argc, char **argv)
{
	if (argc == 1)
		req("SERVER-DISCONNECT %s", argv[0]);
	else
		req("SERVER-DISCONNECT");

	ok();
}

static void
cmd_server_list(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	char *list;

	req("SERVER-LIST");

	if (strncmp(list = poll(), "OK ", 3) != 0)
		errx(1, "failed to retrieve server list");

	/* Skip "OK " */
	list += 3;

	/* Since list is separated by spaces, just convert them to \n */
	for (char *p; (p = strchr(list, ' ')); )
		*p = '\n';

	puts(list);
}

static void
cmd_server_message(int argc, char **argv)
{
	(void)argc;

	req("SERVER-MESSAGE %s %s %s", argv[0], argv[1], argv[2]);
	ok();
}

static void
cmd_server_me(int argc, char **argv)
{
	(void)argc;

	req("SERVER-ME %s %s %s", argv[0], argv[1], argv[2]);
	ok();
}

static void
cmd_server_mode(int argc, char **argv)
{
	(void)argc;
	(void)argv;
#if 0
	req("MODE %s %s %s%c%s%c%s%c%s", argv[0], argv[1], argv[2],
		argc >= 4 ? ' ', argv[3] : "",
		argc >= 5 ? ' ', argv[4] : "",
		argc >= 6 ? ' ', argv[5] : "");
	ok();
#endif
}

static void
cmd_server_nick(int argc, char **argv)
{
	(void)argc;

	req("SERVER-NICK %s %s", argv[0], argv[1]);
	ok();
}

static void
cmd_server_notice(int argc, char **argv)
{
	(void)argc;

	req("SERVER-NOTICE %s %s %s", argv[0], argv[1], argv[2]);
	ok();
}

static void
cmd_server_part(int argc, char **argv)
{
	(void)argc;

	/* Let's advertise irccd a bit. */
	req("SERVER-PART %s %s %s", argv[0], argv[1],
	    argc >= 3 ? argv[2] : "irccd is shutting down");
	ok();
}

static void
cmd_server_topic(int argc, char **argv)
{
	(void)argc;

	req("SERVER-TOPIC %s %s %s", argv[0], argv[1], argv[2]);
	ok();
}

static void
show_connect(char *line)
{
	const char *args[2] = {0};

	if (irc_util_split(line, args, 2) == 2) {
		printf("event:     onConnect\n");
		printf("server:    %s\n", args[0]);
	}
}

static void
show_disconnect(char *line)
{
	const char *args[2] = {0};

	if (irc_util_split(line, args, 2) == 2) {
		printf("event:     onDisonnect\n");
		printf("server:    %s\n", args[0]);
	}
}

static void
show_invite(char *line)
{
	const char *args[5] = {0};

	if (irc_util_split(line, args, 5) == 5) {
		printf("event:     onInvite\n");
		printf("server:    %s\n", args[1]);
		printf("origin:    %s\n", args[2]);
		printf("channel:   %s\n", args[3]);
		printf("nickname:  %s\n", args[4]);
	}
}

static void
show_join(char *line)
{
	const char *args[4] = {0};

	if (irc_util_split(line, args, 4) == 4) {
		printf("event:     onJoin\n");
		printf("server:    %s\n", args[1]);
		printf("origin:    %s\n", args[2]);
		printf("channel:   %s\n", args[3]);
	}
}

static void
show_kick(char *line)
{
	const char *args[6] = {0};

	if (irc_util_split(line, args, 6) >= 5) {
		printf("event:     onKick\n");
		printf("server:    %s\n", args[1]);
		printf("origin:    %s\n", args[2]);
		printf("channel:   %s\n", args[3]);
		printf("target:    %s\n", args[4]);
		printf("reason:    %s\n", args[5] ? args[5] : "");
	}
}

static void
show_me(char *line)
{
	const char *args[5] = {0};

	if (irc_util_split(line, args, 5) == 5) {
		printf("event:     onMe\n");
		printf("server:    %s\n", args[1]);
		printf("origin:    %s\n", args[2]);
		printf("channel:   %s\n", args[3]);
		printf("message:   %s\n", args[4]);
	}
}

static void
show_message(char *line)
{
	const char *args[5] = {0};

	if (irc_util_split(line, args, 5) == 5) {
		printf("event:     onMessage\n");
		printf("server:    %s\n", args[1]);
		printf("origin:    %s\n", args[2]);
		printf("channel:   %s\n", args[3]);
		printf("message:   %s\n", args[4]);
	}
}

static void
show_mode(char *line)
{
	const char *args[8] = {0};

	if (irc_util_split(line, args, 8) >= 5) {
		printf("event:     onMode\n");
		printf("server:    %s\n", args[1]);
		printf("origin:    %s\n", args[2]);
		printf("channel:   %s\n", args[3]);
		printf("mode:      %s\n", args[4]);
		printf("limit:     %s\n", (args[5] ? args[5] : ""));
		printf("user:      %s\n", (args[6] ? args[6] : ""));
		printf("mask:      %s\n", (args[7] ? args[7] : ""));
	}
}

static void
show_nick(char *line)
{
	const char *args[4] = {0};

	if (irc_util_split(line, args, 4) == 4) {
		printf("event:     onNick\n");
		printf("server:    %s\n", args[1]);
		printf("origin:    %s\n", args[2]);
		printf("nickname:  %s\n", args[3]);
	}
}

static void
show_notice(char *line)
{
	const char *args[5] = {0};

	if (irc_util_split(line, args, 5) == 5) {
		printf("event:     onNotice\n");
		printf("server:    %s\n", args[1]);
		printf("origin:    %s\n", args[2]);
		printf("channel:   %s\n", args[3]);
		printf("message:   %s\n", args[4]);
	}
}

static void
show_part(char *line)
{
	const char *args[5] = {0};

	if (irc_util_split(line, args, 5) >= 4) {
		printf("event:     onPart\n");
		printf("server:    %s\n", args[1]);
		printf("origin:    %s\n", args[2]);
		printf("channel:   %s\n", args[3]);
		printf("reason:    %s\n", (args[4] ? args[4] : ""));
	}
}

static void
show_topic(char *line)
{
	const char *args[5] = {0};

	if (irc_util_split(line, args, 5) >= 4) {
		printf("event:     onTopic\n");
		printf("server:    %s\n", args[1]);
		printf("origin:    %s\n", args[2]);
		printf("channel:   %s\n", args[3]);
		printf("topic:     %s\n", args[4]);
	}
}

static void
show_whois(char *line)
{
	const char *args[6] = {0};
	//char *p, *token;

	if (irc_util_split(line, args, 6) >= 4) {
		printf("event:     onWhois\n");
		printf("server:    %s\n", args[1]);
		printf("nickname:  %s\n", args[2]);
		printf("username:  %s\n", args[3]);
		printf("hostname:  %s\n", args[4]);
		printf("username:  %s\n", args[5]);
		//printf("channels:  %s\n", args[6]);
	}
}

static const struct {
	const char *event;
	void (*show)(char *);
} watchtable[] = {
	{ "EVENT-CONNECT",      show_connect    },
	{ "EVENT-DISCONNECT",   show_disconnect },
	{ "EVENT-INVITE",       show_invite     },
	{ "EVENT-JOIN",         show_join       },
	{ "EVENT-KICK",         show_kick       },
	{ "EVENT-MESSAGE",      show_message    },
	{ "EVENT-ME",           show_me         },
	{ "EVENT-MODE",         show_mode       },
	{ "EVENT-NICK",         show_nick       },
	{ "EVENT-NOTICE",       show_notice     },
	{ "EVENT-PART",         show_part       },
	{ "EVENT-TOPIC",        show_topic      },
	{ "EVENT-WHOIS",        show_whois      }
};

static void
show(char *ev)
{
	for (size_t i = 0; i < IRC_UTIL_SIZE(watchtable); ++i) {
		if (strncmp(watchtable[i].event, ev, strlen(watchtable[i].event)) == 0) {
			watchtable[i].show(ev);
			printf("\n");
			break;
		}
	}
}

static void
cmd_watch(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	struct timeval tv = {0};
	char *ev;

	/* Enable watch. */
	req("WATCH");
	ok();

	/* Turn off timeout to receive indefinitely. */
	if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof (tv)) < 0)
		err(1, "setsockopt");

	while ((ev = poll()))
		show(ev);
}

static const struct cmd {
	const char *name;
	int minargs;
	int maxargs;
	void (*exec)(int, char **);
} cmds[] = {
	/* name                 min     max     exec                   */
	{ "server-disconnect",  0,      1,      cmd_server_disconnect   },
	{ "server-list",        0,      0,      cmd_server_list         },
	{ "server-me",          3,      3,      cmd_server_me           },
	{ "server-message",     3,      3,      cmd_server_message      },
	{ "server-mode",        3,      6,      cmd_server_mode         },
	{ "server-nick",        2,      2,      cmd_server_nick         },
	{ "server-notice",      3,      3,      cmd_server_notice       },
	{ "server-part",        3,      3,      cmd_server_part         },
	{ "server-topic",       3,      3,      cmd_server_topic        },
	{ "watch",              0,      0,      cmd_watch               }
};

static int
cmp_cmd(const void *d1, const void *d2)
{
	return strcmp(d1, ((const struct cmd *)d2)->name);
}

static const struct cmd *
find_cmd(const char *name)
{
	return bsearch(name, cmds, IRC_UTIL_SIZE(cmds), sizeof (cmds[0]), cmp_cmd);
}

static void
run(int argc, char **argv)
{
	const struct cmd *c;

	if (!(c = find_cmd(argv[0])))
		errx(1, "abort: command not found");

	--argc;
	++argv;

	if (argc < c->minargs || argc > c->maxargs)
		errx(1, "abort: invalid number of arguments");

	c->exec(argc, argv);
}

static noreturn void
usage(void)
{
	fprintf(stderr, "usage: %s [-v] [-s sock] command [arguments...]\n", getprogname());
	exit(1);
}

int
main(int argc, char **argv)
{
	setprogname("irccdctl");

	for (int ch; (ch = getopt(argc, argv, "s:v")) != -1; ) {
		switch (ch) {
		case 's':
			strlcpy(sockaddr.sun_path, optarg, sizeof (sockaddr.sun_path));
			break;
		case 'v':
			verbose = true;
			break;
		default:
			break;
		}
	}

	argc -= optind;
	argv += optind;

	if (argc < 1)
		usage();

	dial();
	check();
	run(argc, argv);
}
