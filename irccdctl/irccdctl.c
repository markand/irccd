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

#include <sys/socket.h>
#include <sys/time.h>
#include <sys/un.h>
#include <assert.h>
#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>
#include <unistd.h>

#include <ketopt.h>

#include <irccd/limits.h>
#include <irccd/util.h>

static int verbose;
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

static char *
ok(void)
{
	char *response = poll();

	if (strncmp(response, "OK", 2) != 0)
		errx(1, "abort: %s", response);

	/* Skip "OK". */
	response += 2;

	while (*response && isspace(*response))
		response++;

	return response;
}

static void
show_connect(char *line)
{
	const char *args[2] = {0};

	if (irc_util_split(line, args, 2, ' ') == 2) {
		printf("%-16s: %s\n", "event", "onConnect");
		printf("%-16s: %s\n", "server", args[0]);
	}
}

static void
show_disconnect(char *line)
{
	const char *args[2] = {0};

	if (irc_util_split(line, args, 2, ' ') == 2) {
		printf("%-16s: %s\n", "event", "onDisonnect");
		printf("%-16s: %s\n", "server", args[0]);
	}
}

static void
show_invite(char *line)
{
	const char *args[5] = {0};

	if (irc_util_split(line, args, 5, ' ') == 5) {
		printf("%-16s: %s\n", "event", "onInvite");
		printf("%-16s: %s\n", "server", args[1]);
		printf("%-16s: %s\n", "origin", args[2]);
		printf("%-16s: %s\n", "channel", args[3]);
		printf("%-16s: %s\n", "nickname", args[4]);
	}
}

static void
show_join(char *line)
{
	const char *args[4] = {0};

	if (irc_util_split(line, args, 4, ' ') == 4) {
		printf("%-16s: %s\n", "event", "onJoin");
		printf("%-16s: %s\n", "server", args[1]);
		printf("%-16s: %s\n", "origin", args[2]);
		printf("%-16s: %s\n", "channel", args[3]);
	}
}

static void
show_kick(char *line)
{
	const char *args[6] = {0};

	if (irc_util_split(line, args, 6, ' ') >= 5) {
		printf("%-16s: %s\n", "event", "onKick");
		printf("%-16s: %s\n", "server", args[1]);
		printf("%-16s: %s\n", "origin", args[2]);
		printf("%-16s: %s\n", "channel", args[3]);
		printf("%-16s: %s\n", "target", args[4]);
		printf("%-16s: %s\n", "reason", args[5] ? args[5] : "");
	}
}

static void
show_me(char *line)
{
	const char *args[5] = {0};

	if (irc_util_split(line, args, 5, ' ') == 5) {
		printf("%-16s: %s\n", "event", "onMe");
		printf("%-16s: %s\n", "server", args[1]);
		printf("%-16s: %s\n", "origin", args[2]);
		printf("%-16s: %s\n", "channel", args[3]);
		printf("%-16s: %s\n", "message", args[4]);
	}
}

static void
show_message(char *line)
{
	const char *args[5] = {0};

	if (irc_util_split(line, args, 5, ' ') == 5) {
		printf("%-16s: %s\n", "event", "onMessage");
		printf("%-16s: %s\n", "server", args[1]);
		printf("%-16s: %s\n", "origin", args[2]);
		printf("%-16s: %s\n", "channel", args[3]);
		printf("%-16s: %s\n", "message", args[4]);
	}
}

static void
show_mode(char *line)
{
	const char *args[8] = {0};

	if (irc_util_split(line, args, 8, ' ') >= 5) {
		printf("%-16s: %s\n", "event", "onMode");
		printf("%-16s: %s\n", "server", args[1]);
		printf("%-16s: %s\n", "origin", args[2]);
		printf("%-16s: %s\n", "channel", args[3]);
		printf("%-16s: %s\n", "mode", args[4]);
		printf("%-16s: %s\n", "limit", (args[5] ? args[5] : ""));
		printf("%-16s: %s\n", "user", (args[6] ? args[6] : ""));
		printf("%-16s: %s\n", "mask", (args[7] ? args[7] : ""));
	}
}

static void
show_nick(char *line)
{
	const char *args[4] = {0};

	if (irc_util_split(line, args, 4, ' ') == 4) {
		printf("%-16s: %s\n", "event", "onNick");
		printf("%-16s: %s\n", "server", args[1]);
		printf("%-16s: %s\n", "origin", args[2]);
		printf("%-16s: %s\n", "nickname", args[3]);
	}
}

static void
show_notice(char *line)
{
	const char *args[5] = {0};

	if (irc_util_split(line, args, 5, ' ') == 5) {
		printf("%-16s: %s\n", "event", "onNotice");
		printf("%-16s: %s\n", "server", args[1]);
		printf("%-16s: %s\n", "origin", args[2]);
		printf("%-16s: %s\n", "channel", args[3]);
		printf("%-16s: %s\n", "message", args[4]);
	}
}

static void
show_part(char *line)
{
	const char *args[5] = {0};

	if (irc_util_split(line, args, 5, ' ') >= 4) {
		printf("%-16s: %s\n", "event", "onPart");
		printf("%-16s: %s\n", "server", args[1]);
		printf("%-16s: %s\n", "origin", args[2]);
		printf("%-16s: %s\n", "channel", args[3]);
		printf("%-16s: %s\n", "reason", (args[4] ? args[4] : ""));
	}
}

static void
show_topic(char *line)
{
	const char *args[5] = {0};

	if (irc_util_split(line, args, 5, ' ') >= 4) {
		printf("%-16s: %s\n", "event", "onTopic");
		printf("%-16s: %s\n", "server", args[1]);
		printf("%-16s: %s\n", "origin", args[2]);
		printf("%-16s: %s\n", "channel", args[3]);
		printf("%-16s: %s\n", "topic", args[4]);
	}
}

static void
show_whois(char *line)
{
	const char *args[6] = {0};
	//char *p, *token;

	if (irc_util_split(line, args, 6, ' ') >= 4) {
		printf("%-16s: %s\n", "event", "onWhois");
		printf("%-16s: %s\n", "server", args[1]);
		printf("%-16s: %s\n", "nickname", args[2]);
		printf("%-16s: %s\n", "username", args[3]);
		printf("%-16s: %s\n", "hostname", args[4]);
		printf("%-16s: %s\n", "username", args[5]);
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
plugin_list_set(int argc, char **argv, const char *cmd)
{
	char *line, *p;
	size_t num = 0;

	if (argc == 3) {
		req("%s %s %s %s", cmd, argv[0], argv[1], argv[2]);
		ok();
		return;
	}

	if (argc == 2)
		req("%s %s %s", cmd, argv[0], argv[1]);
	else
		req("%s %s", cmd, argv[0]);

	if (sscanf(line = ok(), "%zu", &num) != 1)
		errx(1, "could not retrieve list");

	if (argc == 2)
		puts(poll());
	else {
		while (num-- != 0 && (line = poll())) {
			if (!(p = strchr(line, '=')))
				continue;

			*p = '\0';
			printf("%-16s: %s\n", line, p + 1);
		}
	}
}

static void
response_list(const char *cmd)
{
	char *list;

	req(cmd);

	if (strncmp(list = poll(), "OK ", 3) != 0)
		errx(1, "failed to retrieve plugin list");

	list += 3;

	for (char *p; (p = strchr(list, ' ')); )
		*p = '\n';

	if (*list)
		puts(list);
}

static void
cmd_hook_add(int argc, char **argv)
{
	(void)argc;

	req("HOOK-ADD %s %s", argv[0], argv[1]);
	ok();
}

static void
cmd_hook_list(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	response_list("HOOK-LIST");
}

static void
cmd_hook_remove(int argc, char **argv)
{
	(void)argc;

	req("HOOK-REMOVE %s", argv[0]);
	ok();
}

static void
cmd_plugin_config(int argc, char **argv)
{
	return plugin_list_set(argc, argv, "PLUGIN-CONFIG");
}

/*
 * Response:
 *
 *     OK name
 *     summary
 *     version
 *     license
 *     author
 */
static void
cmd_plugin_info(int argc, char **argv)
{
	(void)argc;

	const char *response;

	req("PLUGIN-INFO %s", argv[0]);

	if (strncmp((response = poll()), "OK ", 3) != 0)
		errx(1, "failed to retrieve plugin information");

	printf("%-16s: %s\n", "name", response + 3);
	printf("%-16s: %s\n", "summary", poll());
	printf("%-16s: %s\n", "version", poll());
	printf("%-16s: %s\n", "license", poll());
	printf("%-16s: %s\n", "author", poll());
}

static void
cmd_plugin_list(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	response_list("PLUGIN-LIST");
}

static void
cmd_plugin_load(int argc, char **argv)
{
	(void)argc;

	req("PLUGIN-LOAD %s", argv[0]);
	ok();
}

static void
cmd_plugin_path(int argc, char **argv)
{
	return plugin_list_set(argc, argv, "PLUGIN-PATH");
}

static void
cmd_plugin_reload(int argc, char **argv)
{
	if (argc == 1)
		req("PLUGIN-RELOAD %s", argv[0]);
	else
		req("PLUGIN-RELOAD");

	ok();
}

static void
cmd_plugin_template(int argc, char **argv)
{
	return plugin_list_set(argc, argv, "PLUGIN-TEMPLATE");
}

static void
cmd_plugin_unload(int argc, char **argv)
{
	if (argc == 1)
		req("PLUGIN-UNLOAD %s", argv[0]);
	else
		req("PLUGIN-UNLOAD");

	ok();
}

static void
cmd_rule_add(int argc, char **argv)
{
	ketopt_t ko = KETOPT_INIT;
	FILE *fp;
	char out[IRC_BUF_LEN];

	if (!(fp = fmemopen(out, sizeof (out) - 1, "w")))
		err(1, "fmemopen");

	/* TODO: invalid option. */
	for (int ch; (ch = ketopt(&ko, argc, argv, 0, "c:e:i:o:p:s:", NULL)) != -1; )
		fprintf(fp, "%c=%s ", ch, ko.arg);

	argc -= ko.ind;
	argv += ko.ind;

	if (argc < 1)
		errx(1, "missing accept or drop rule action");

	fprintf(fp, "%s", argv[0]);

	if (ferror(fp) || feof(fp))
		err(1, "fprintf");

	fclose(fp);
	req("RULE-ADD %s %s", argv[0], out);
	ok();
}

static void
cmd_rule_edit(int argc, char **argv)
{
	ketopt_t ko = KETOPT_INIT;
	FILE *fp;
	char out[IRC_BUF_LEN];

	if (!(fp = fmemopen(out, sizeof (out) - 1, "w")))
		err(1, "fmemopen");

	/* TODO: invalid option. */
	for (int ch; (ch = ketopt(&ko, argc, argv, 0, "a:C:c:E:e:O:o:P:p:S:s:", NULL)) != -1; ) {
		if (ch == 'a')
			fprintf(fp, "a=%s ", ko.arg);
		else
			fprintf(fp, "%c%c%s ", tolower(ch), isupper(ch) ? '-' : '+', ko.arg);
	}

	argc -= ko.ind;
	argv += ko.ind;

	if (argc < 1)
		errx(1, "missing rule index");

	if (ferror(fp) || feof(fp))
		err(1, "fprintf");

	fclose(fp);
	req("RULE-EDIT %s %s", argv[0], out);
	ok();
}

/*
 * Response:
 *
 *     OK <n>
 *     accept
 *     server1 server2 server3 ...
 *     channel1 channel2 channel3 ...
 *     origin1 origin2 origin3 ...
 *     plugin1 plugin2 plugin3 ...
 *     event1 event2 plugin3 ...
 *     (repeat for every rule in <n>)
 */
static void
cmd_rule_list(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	size_t num = 0;

	req("RULE-LIST");

	if (sscanf(ok(), "%zu", &num) != 1)
		errx(1, "could not retrieve rule list");

	for (size_t i = 0; i < num; ++i) {
		printf("%-16s: %zu\n", "index", i);
		printf("%-16s: %s\n", "action", poll());
		printf("%-16s: %s\n", "servers", poll());
		printf("%-16s: %s\n", "channels", poll());
		printf("%-16s: %s\n", "origins", poll());
		printf("%-16s: %s\n", "plugins", poll());
		printf("%-16s: %s\n", "events", poll());
		printf("\n");
	}
}

static void
cmd_rule_move(int argc, char **argv)
{
	(void)argc;

	long long from, to;
	const char *errstr;

	if ((from = strtonum(argv[0], 0, LLONG_MAX, &errstr)) == 0 && errstr)
		err(1, "%s", argv[0]);
	if ((to = strtonum(argv[1], 0, LLONG_MAX, &errstr)) == 0 && errstr)
		err(1, "%s", argv[1]);

	req("RULE-MOVE %lld %lld", from, to);
	ok();
}

static void
cmd_rule_remove(int argc, char **argv)
{
	(void)argc;

	req("RULE-REMOVE %s", argv[0]);
	ok();
}

static void
cmd_server_connect(int argc, char **argv)
{
	ketopt_t ko = KETOPT_INIT;
	int ssl = 0;
	const char *nickname = "irccd",
	           *username = "irccd",
	           *realname = "IRC Client Daemon",
	           *port = "6667";

	for (int ch; (ch = ketopt(&ko, argc, argv, 0, "sn:r:u:p:", NULL)) != -1; ) {
		switch (ch) {
		case 's':
			ssl = 1;
			break;
		case 'n':
			nickname = ko.arg;
			break;
		case 'r':
			realname = ko.arg;
			break;
		case 'u':
			username = ko.arg;
			break;
		case 'p':
			port = ko.arg;
			break;
		default:
			break;
		}
	}

	argc -= ko.ind;
	argv += ko.ind;

	if (argc < 2)
		errx(1, "missing id and/or host");

	req("SERVER-CONNECT %s %s %s%s %s %s %s", argv[0], argv[1], (ssl ? "+" : ""),
	    port, nickname, username, realname);
	ok();
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

/*
 * Response:
 *
 *     OK name
 *     hostname port [ssl]
 *     nickname username realname
 *     chan1 chan2 chanN
 */
static void
cmd_server_info(int argc, char **argv)
{
	(void)argc;

	char *list;
	const char *args[16] = {0};

	req("SERVER-INFO %s", argv[0]);

	if (strncmp(list = poll(), "OK ", 3) != 0)
		errx(1, "failed to retrieve server information");

	printf("%-16s: %s\n", "name", list + 3);

	if (irc_util_split((list = poll()), args, 3, ' ') < 2)
		errx(1, "malformed server connection");

	printf("%-16s: %s\n", "hostname", args[0]);
	printf("%-16s: %s\n", "port", args[1]);

	if (args[2])
		printf("%-16s: %s\n", "ssl", "true");

	if (irc_util_split((list = poll()), args, 3, ' ') != 3)
		errx(1, "malformed server ident");

	printf("%-16s: %s\n", "nickname", args[0]);
	printf("%-16s: %s\n", "username", args[0]);
	printf("%-16s: %s\n", "realname", args[0]);
	printf("%-16s: %s\n", "channels", poll());
}

static void
cmd_server_join(int argc, char **argv)
{
	if (argc >= 3)
		req("SERVER-JOIN %s %s %s", argv[0], argv[1], argv[2]);
	else
		req("SERVER-JOIN %s %s", argv[0], argv[1]);

	ok();
}

static void
cmd_server_list(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	response_list("SERVER-LIST");
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
	req("SERVER-MODE %s %s %s%c%s", argv[0], argv[1], argv[2],
	    argc >= 4 ? ' '     : '\0',
	    argc >= 4 ? argv[3] : "");
	ok();
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
	{ "hook-add",           2,      2,      cmd_hook_add            },
	{ "hook-list",          0,      0,      cmd_hook_list           },
	{ "hook-remove",        1,      1,      cmd_hook_remove         },
	{ "plugin-config",      1,      3,      cmd_plugin_config       },
	{ "plugin-info",        1,      1,      cmd_plugin_info         },
	{ "plugin-list",        0,      0,      cmd_plugin_list         },
	{ "plugin-load",        1,      1,      cmd_plugin_load         },
	{ "plugin-path",        0,      3,      cmd_plugin_path         },
	{ "plugin-reload",      0,      1,      cmd_plugin_reload       },
	{ "plugin-template",    1,      3,      cmd_plugin_template     },
	{ "plugin-unload",      0,      1,      cmd_plugin_unload       },
	{ "rule-add",          -1,     -1,      cmd_rule_add            },
	{ "rule-edit",         -1,     -1,      cmd_rule_edit           },
	{ "rule-list",          0,      0,      cmd_rule_list           },
	{ "rule-move",          2,      2,      cmd_rule_move           },
	{ "rule-remove",        1,      1,      cmd_rule_remove         },
	{ "server-connect",    -1,     -1,      cmd_server_connect      },
	{ "server-disconnect",  0,      1,      cmd_server_disconnect   },
	{ "server-info",        1,      1,      cmd_server_info         },
	{ "server-join",        2,      3,      cmd_server_join         },
	{ "server-list",        0,      0,      cmd_server_list         },
	{ "server-me",          3,      3,      cmd_server_me           },
	{ "server-message",     3,      3,      cmd_server_message      },
	{ "server-mode",        3,      4,      cmd_server_mode         },
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

	if ((c->minargs != -1 && argc < c->minargs) || (c->minargs != -1 && argc > c->maxargs))
		errx(1, "abort: invalid number of arguments");

	c->exec(argc, argv);
}

noreturn static void
usage(void)
{
	fprintf(stderr, "usage: %s [-v] [-s sock] command [options...] [arguments...]\n", getprogname());
	exit(1);
}

noreturn static void
help(void)
{
	fprintf(stderr, "usage: %s hook-add name path\n", getprogname());
	fprintf(stderr, "       %s hook-list\n", getprogname());
	fprintf(stderr, "       %s hook-remove id\n", getprogname());
	fprintf(stderr, "       %s plugin-config id [variable [value]]\n", getprogname());
	fprintf(stderr, "       %s plugin-info id\n", getprogname());
	fprintf(stderr, "       %s plugin-list\n", getprogname());
	fprintf(stderr, "       %s plugin-load name\n", getprogname());
	fprintf(stderr, "       %s plugin-path [variable [value]]\n", getprogname());
	fprintf(stderr, "       %s plugin-template [variable [value]]\n", getprogname());
	fprintf(stderr, "       %s plugin-reload [plugin]\n", getprogname());
	fprintf(stderr, "       %s plugin-unload [plugin]\n", getprogname());
	fprintf(stderr, "       %s rule-add [-c channel] [-e event] [-i index] [-o origin] [-p plugin] [-s server] accept|drop\n", getprogname());
	fprintf(stderr, "       %s rule-edit [-a accept|drop] [-c|C channel] [-e|E event] [-o|O origin] [-s|S server] index\n", getprogname());
	fprintf(stderr, "       %s rule-list\n", getprogname());
	fprintf(stderr, "       %s rule-move from to\n", getprogname());
	fprintf(stderr, "       %s rule-remove index\n", getprogname());
	fprintf(stderr, "       %s server-connect [-n nickname] [-r realname] [-u username] [-p port] id hostname\n", getprogname());
	fprintf(stderr, "       %s server-disconnect [server]\n", getprogname());
	fprintf(stderr, "       %s server-info server\n", getprogname());
	fprintf(stderr, "       %s server-invite server target channel\n", getprogname());
	fprintf(stderr, "       %s server-join server channel [password]\n", getprogname());
	fprintf(stderr, "       %s server-kick server target channel [reason]\n", getprogname());
	fprintf(stderr, "       %s server-list\n", getprogname());
	fprintf(stderr, "       %s server-me server target message\n", getprogname());
	fprintf(stderr, "       %s server-message server target message\n", getprogname());
	fprintf(stderr, "       %s server-mode server target mode [args]\n", getprogname());
	fprintf(stderr, "       %s server-nick server nickname\n", getprogname());
	fprintf(stderr, "       %s server-notice server target message\n", getprogname());
	fprintf(stderr, "       %s server-part server channel [reason]\n", getprogname());
	fprintf(stderr, "       %s server-reconnect [server]\n", getprogname());
	fprintf(stderr, "       %s server-topic server channel topic\n", getprogname());
	fprintf(stderr, "       %s watch\n", getprogname());
	exit(1);
}

int
main(int argc, char **argv)
{
	ketopt_t ko = KETOPT_INIT;

	setprogname("irccdctl");

	--argc;
	++argv;

	for (int ch; (ch = ketopt(&ko, argc, argv, 0, "s:v", NULL)) != -1; ) {
		switch (ch) {
		case 's':
			strlcpy(sockaddr.sun_path, ko.arg, sizeof (sockaddr.sun_path));
			break;
		case 'v':
			verbose = 1;
			break;
		default:
			usage();
			break;
		}
	}

	argc -= ko.ind;
	argv += ko.ind;

	if (argc < 1)
		usage();
	else if (strcmp(argv[0], "help") == 0)
		help();

	dial();
	check();
	run(argc, argv);
}
