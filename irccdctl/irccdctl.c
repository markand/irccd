/*
 * main.c -- irccdctl(1) main file
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

#define _XOPEN_SOURCE 800
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/un.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>
#include <unistd.h>

#include <irccd/irccd.h>
#include <irccd/util.h>

static int verbose;
static int sock;
static struct sockaddr_un sockaddr = {
	.sun_family = AF_UNIX,
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
		char buf[IRC_BUF_LEN] = {};
		ssize_t nr;

		if ((nr = recv(sock, buf, sizeof (buf) - 1, 0)) <= 0)
			irc_util_die("abort: recv: %s\n", strerror(nr == 0 ? ECONNRESET : errno));
		if (irc_util_strlcat(in, buf, sizeof (in)) >= sizeof (in))
			irc_util_die("abort: recv: %s\n", strerror(EMSGSIZE));
	}

	*nl = '\0';
	irc_util_strlcpy(ret, in, sizeof (ret));
	memmove(in, nl + 1, sizeof (in) - (nl - in) - 1);

	return ret;
}

static void
dial(void)
{
	const struct timeval tv = {
		.tv_sec = 30
	};

	if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
		irc_util_die("abort: socket: %s\n", strerror(errno));
	if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof (tv)) < 0 ||
	    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof (tv)) < 0)
		irc_util_die("abort: setsockopt: %s\n", strerror(errno));
	if (connect(sock, (const struct sockaddr *)&sockaddr, sizeof (sockaddr)) < 0)
		irc_util_die("abort: connect: %s\n", strerror(errno));
}

static void
check(void)
{
	/* Ensure we're talking to irccd. */
	int major, minor, patch;

	if (sscanf(poll(), "IRCCD %d.%d.%d", &major, &minor, &patch) != 3)
		irc_util_die("abort: not irccd instance\n");
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

	if (irc_util_strlcat(out, buf, sizeof (out)) >= sizeof (out) ||
	    irc_util_strlcat(out, "\n", sizeof (out)) >= sizeof (out))
		irc_util_die("abort: %s\n", strerror(EMSGSIZE));

	while (out[0]) {
		ssize_t ns, len;

		len = strlen(out);

		if ((ns = send(sock, out, len, MSG_NOSIGNAL)) <= 0)
			irc_util_die("abort: send: %s\n", strerror(errno));

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
		irc_util_die("abort: %s\n", response);

	/* Skip "OK". */
	response += 2;

	while (*response && isspace(*response))
		response++;

	return response;
}

static void
show_connect(char *line)
{
	const char *args[2] = {};

	if (irc_util_split(line, args, 2, ' ') == 2) {
		printf("%-16s%s\n", "event:", "onConnect");
		printf("%-16s%s\n", "server:", args[0]);
	}
}

static void
show_disconnect(char *line)
{
	const char *args[2] = {};

	if (irc_util_split(line, args, 2, ' ') == 2) {
		printf("%-16s%s\n", "event:", "onDisonnect");
		printf("%-16s%s\n", "server:", args[0]);
	}
}

static void
show_invite(char *line)
{
	const char *args[5] = {};

	if (irc_util_split(line, args, 5, ' ') == 5) {
		printf("%-16s%s\n", "event:", "onInvite");
		printf("%-16s%s\n", "server:", args[1]);
		printf("%-16s%s\n", "origin:", args[2]);
		printf("%-16s%s\n", "channel:", args[3]);
		printf("%-16s%s\n", "nickname:", args[4]);
	}
}

static void
show_join(char *line)
{
	const char *args[4] = {};

	if (irc_util_split(line, args, 4, ' ') == 4) {
		printf("%-16s%s\n", "event:", "onJoin");
		printf("%-16s%s\n", "server:", args[1]);
		printf("%-16s%s\n", "origin:", args[2]);
		printf("%-16s%s\n", "channel:", args[3]);
	}
}

static void
show_kick(char *line)
{
	const char *args[6] = {};

	if (irc_util_split(line, args, 6, ' ') >= 5) {
		printf("%-16s%s\n", "event:", "onKick");
		printf("%-16s%s\n", "server:", args[1]);
		printf("%-16s%s\n", "origin:", args[2]);
		printf("%-16s%s\n", "channel:", args[3]);
		printf("%-16s%s\n", "target:", args[4]);
		printf("%-16s%s\n", "reason:", args[5] ? args[5] : "");
	}
}

static void
show_me(char *line)
{
	const char *args[5] = {};

	if (irc_util_split(line, args, 5, ' ') == 5) {
		printf("%-16s%s\n", "event:", "onMe");
		printf("%-16s%s\n", "server:", args[1]);
		printf("%-16s%s\n", "origin:", args[2]);
		printf("%-16s%s\n", "channel:", args[3]);
		printf("%-16s%s\n", "message:", args[4]);
	}
}

static void
show_message(char *line)
{
	const char *args[5] = {};

	if (irc_util_split(line, args, 5, ' ') == 5) {
		printf("%-16s%s\n", "event:", "onMessage");
		printf("%-16s%s\n", "server:", args[1]);
		printf("%-16s%s\n", "origin:", args[2]);
		printf("%-16s%s\n", "channel:", args[3]);
		printf("%-16s%s\n", "message:", args[4]);
	}
}

static void
show_mode(char *line)
{
	const char *args[8] = {};

	if (irc_util_split(line, args, 8, ' ') >= 5) {
		printf("%-16s%s\n", "event:", "onMode");
		printf("%-16s%s\n", "server:", args[1]);
		printf("%-16s%s\n", "origin:", args[2]);
		printf("%-16s%s\n", "channel:", args[3]);
		printf("%-16s", "mode: ");

		for (int i = 4; i < 8 && args[i]; ++i)
			printf("%s ", args[i]);

		printf("\n");
	}
}

static void
show_nick(char *line)
{
	const char *args[4] = {};

	if (irc_util_split(line, args, 4, ' ') == 4) {
		printf("%-16s%s\n", "event:", "onNick");
		printf("%-16s%s\n", "server:", args[1]);
		printf("%-16s%s\n", "origin:", args[2]);
		printf("%-16s%s\n", "nickname:", args[3]);
	}
}

static void
show_notice(char *line)
{
	const char *args[5] = {};

	if (irc_util_split(line, args, 5, ' ') == 5) {
		printf("%-16s%s\n", "event:", "onNotice");
		printf("%-16s%s\n", "server:", args[1]);
		printf("%-16s%s\n", "origin:", args[2]);
		printf("%-16s%s\n", "channel:", args[3]);
		printf("%-16s%s\n", "message:", args[4]);
	}
}

static void
show_part(char *line)
{
	const char *args[5] = {};

	if (irc_util_split(line, args, 5, ' ') >= 4) {
		printf("%-16s%s\n", "event:", "onPart");
		printf("%-16s%s\n", "server:", args[1]);
		printf("%-16s%s\n", "origin:", args[2]);
		printf("%-16s%s\n", "channel:", args[3]);
		printf("%-16s%s\n", "reason:", (args[4] ? args[4] : ""));
	}
}

static void
show_topic(char *line)
{
	const char *args[5] = {};

	if (irc_util_split(line, args, 5, ' ') >= 4) {
		printf("%-16s%s\n", "event:", "onTopic");
		printf("%-16s%s\n", "server:", args[1]);
		printf("%-16s%s\n", "origin:", args[2]);
		printf("%-16s%s\n", "channel:", args[3]);
		printf("%-16s%s\n", "topic:", args[4]);
	}
}

static void
show_whois(char *line)
{
	const char *args[6] = {};

	if (irc_util_split(line, args, 6, ' ') >= 4) {
		printf("%-16s%s\n", "event:", "onWhois");
		printf("%-16s%s\n", "server:", args[1]);
		printf("%-16s%s\n", "nickname:", args[2]);
		printf("%-16s%s\n", "username:", args[3]);
		printf("%-16s%s\n", "hostname:", args[4]);
		printf("%-16s%s\n", "username:", args[5]);
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
	char *line, *p, name[16];
	size_t num = 0;

	--argc;
	++argv;

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
		irc_util_die("abort: could not retrieve list\n");

	if (argc == 2)
		puts(poll());
	else {
		while (num-- != 0 && (line = poll())) {
			if (!(p = strchr(line, '=')))
				continue;

			*p = '\0';
			name[0] = '\0';
			snprintf(name, sizeof (name), "%s:", line);
			printf("%-16s%s\n", name, p + 1);
		}
	}
}

static void
response_list(const char *cmd)
{
	char *list;

	req(cmd);

	if (strncmp(list = poll(), "OK ", 3) != 0)
		irc_util_die("abort: failed to retrieve list\n");

	list += 3;

	for (char *p; (p = strchr(list, ' ')); )
		*p = '\n';

	if (*list)
		puts(list);
}

static void
cmd_hook_add(int, char **argv)
{
	req("HOOK-ADD %s %s", argv[1], argv[2]);
	ok();
}

static void
cmd_hook_list(int, char **)
{
	response_list("HOOK-LIST");
}

static void
cmd_hook_remove(int, char **argv)
{
	req("HOOK-REMOVE %s", argv[1]);
	ok();
}

static void
cmd_plugin_config(int argc, char **argv)
{
	plugin_list_set(argc, argv, "PLUGIN-CONFIG");
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
cmd_plugin_info(int, char **argv)
{
	const char *response;

	req("PLUGIN-INFO %s", argv[1]);

	if (strncmp((response = poll()), "OK ", 3) != 0)
		irc_util_die("abort: failed to retrieve plugin information\n");

	printf("%-16s%s\n", "name:", response + 3);
	printf("%-16s%s\n", "summary:", poll());
	printf("%-16s%s\n", "version:", poll());
	printf("%-16s%s\n", "license:", poll());
	printf("%-16s%s\n", "author:", poll());
}

static void
cmd_plugin_list(int, char **)
{
	response_list("PLUGIN-LIST");
}

static void
cmd_plugin_load(int, char **argv)
{
	req("PLUGIN-LOAD %s", argv[1]);
	ok();
}

static void
cmd_plugin_path(int argc, char **argv)
{
	plugin_list_set(argc, argv, "PLUGIN-PATH");
}

static void
cmd_plugin_reload(int argc, char **argv)
{
	if (argc == 2)
		req("PLUGIN-RELOAD %s", argv[1]);
	else
		req("PLUGIN-RELOAD");

	ok();
}

static void
cmd_plugin_template(int argc, char **argv)
{
	plugin_list_set(argc, argv, "PLUGIN-TEMPLATE");
}

static void
cmd_plugin_unload(int argc, char **argv)
{
	if (argc == 2)
		req("PLUGIN-UNLOAD %s", argv[1]);
	else
		req("PLUGIN-UNLOAD");

	ok();
}

static void
cmd_rule_add(int argc, char **argv)
{
	char out[IRC_BUF_LEN] = {};
	FILE *fp;
	int ch;

	if (!(fp = fmemopen(out, sizeof (out) - 1, "w")))
		irc_util_die("abort: fmemopen: %s\n", strerror(errno));

	/* TODO: invalid option. */
	while ((ch = getopt(argc, argv, "c:e:i:o:p:s:")) != -1)
		fprintf(fp, "%c=%s ", ch, optarg);

	argc -= optind;
	argv += optind;

	if (argc < 1)
		irc_util_die("abort: missing accept or drop rule action\n");

	fprintf(fp, "%s", argv[0]);

	if (ferror(fp) || feof(fp))
		irc_util_die("abort: fprintf: %s\n", strerror(errno));

	fclose(fp);
	req("RULE-ADD %s %s", argv[0], out);
	ok();
}

static void
cmd_rule_edit(int argc, char **argv)
{
	FILE *fp;
	char out[IRC_BUF_LEN];
	int ch;

	if (!(fp = fmemopen(out, sizeof (out) - 1, "w")))
		irc_util_die("abort: fmemopen: %s\n", strerror(errno));

	/* TODO: invalid option. */
	while ((ch = getopt(argc, argv, "a:C:c:E:e:O:o:P:p:S:s:")) != -1) {
		if (ch == 'a')
			fprintf(fp, "a=%s ", optarg);
		else
			fprintf(fp, "%c%c%s ", tolower(ch), isupper(ch) ? '-' : '+', optarg);
	}

	argc -= optind;
	argv += optind;

	if (argc < 1)
		irc_util_die("abort: missing rule index\n");
	if (ferror(fp) || feof(fp))
		irc_util_die("abort: fprintf: %s\n", strerror(errno));

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
cmd_rule_list(int, char **)
{
	size_t num = 0;

	req("RULE-LIST");

	if (sscanf(ok(), "%zu", &num) != 1)
		irc_util_die("abort: could not retrieve rule list\n");

	for (size_t i = 0; i < num; ++i) {
		printf("%-16s%zu\n", "index:", i);
		printf("%-16s%s\n", "action:", poll());
		printf("%-16s%s\n", "servers:", poll());
		printf("%-16s%s\n", "channels:", poll());
		printf("%-16s%s\n", "origins:", poll());
		printf("%-16s%s\n", "plugins:", poll());
		printf("%-16s%s\n", "events:", poll());

		if (i + 1 < num)
			printf("\n");
	}
}

static void
cmd_rule_move(int argc, char **argv)
{
	(void)argc;

	++argv;

	long long from, to;

	if (irc_util_stoi(argv[0], &from) < 0)
		irc_util_die("abort: %s: %s\n", argv[0], strerror(errno));
	if (irc_util_stoi(argv[1], &to) < 0)
		irc_util_die("abort: %s: %s\n", argv[1], strerror(errno));

	req("RULE-MOVE %lld %lld", from, to);
	ok();
}

static void
cmd_rule_remove(int, char **argv)
{
	req("RULE-REMOVE %s", argv[1]);
	ok();
}

static void
cmd_server_connect(int argc, char **argv)
{
	int ssl = 0, ch;
	const char *nickname = "irccd",
	           *username = "irccd",
	           *realname = "IRC Client Daemon",
	           *port = "6667";

	while ((ch = getopt(argc, argv, "sn:r:u:p:")) != -1) {
		switch (ch) {
		case 's':
			ssl = 1;
			break;
		case 'n':
			nickname = optarg;
			break;
		case 'r':
			realname = optarg;
			break;
		case 'u':
			username = optarg;
			break;
		case 'p':
			port = optarg;
			break;
		default:
			break;
		}
	}

	argc -= optind;
	argv += optind;

	if (argc < 2)
		irc_util_die("abort: missing id and/or host\n");

	req("SERVER-CONNECT %s %s %s%s %s %s %s", argv[0], argv[1], (ssl ? "+" : ""),
	    port, nickname, username, realname);
	ok();
}

static void
cmd_server_disconnect(int argc, char **argv)
{
	if (argc == 2)
		req("SERVER-DISCONNECT %s", argv[1]);
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
cmd_server_info(int, char **argv)
{
	char *list;
	const char *args[16] = {};

	req("SERVER-INFO %s", argv[0]);

	if (strncmp(list = poll(), "OK ", 3) != 0)
		irc_util_die("abort: failed to retrieve server information\n");

	printf("%-16s%s\n", "name:", list + 3);

	if (irc_util_split((list = poll()), args, 3, ' ') < 2)
		irc_util_die("abort: malformed server connection\n");

	printf("%-16s%s\n", "hostname:", args[0]);
	printf("%-16s%s\n", "port:", args[1]);

	if (args[2])
		printf("%-16s%s\n", "ssl:", "true");

	if (irc_util_split((list = poll()), args, 3, ' ') != 3)
		irc_util_die("abort: malformed server ident\n");

	printf("%-16s%s\n", "nickname:", args[0]);
	printf("%-16s%s\n", "username:", args[0]);
	printf("%-16s%s\n", "realname:", args[0]);
	printf("%-16s%s\n", "channels:", poll());
}

static void
cmd_server_join(int argc, char **argv)
{
	if (argc >= 4)
		req("SERVER-JOIN %s %s %s", argv[1], argv[2], argv[3]);
	else
		req("SERVER-JOIN %s %s", argv[1], argv[2]);

	ok();
}

static void
cmd_server_list(int, char **)
{
	response_list("SERVER-LIST");
}

static void
cmd_server_message(int, char **argv)
{
	req("SERVER-MESSAGE %s %s %s", argv[1], argv[2], argv[3]);
	ok();
}

static void
cmd_server_me(int, char **argv)
{
	req("SERVER-ME %s %s %s", argv[1], argv[2], argv[3]);
	ok();
}

static void
cmd_server_mode(int argc, char **argv)
{
	req("SERVER-MODE %s %s %s%c%s", argv[1], argv[2], argv[3],
	    argc >= 5 ? ' '     : '\0',
	    argc >= 5 ? argv[4] : "");
	ok();
}

static void
cmd_server_nick(int, char **argv)
{
	req("SERVER-NICK %s %s", argv[1], argv[2]);
	ok();
}

static void
cmd_server_notice(int, char **argv)
{
	req("SERVER-NOTICE %s %s %s", argv[1], argv[2], argv[3]);
	ok();
}

static void
cmd_server_part(int argc, char **argv)
{
	/* Let's advertise irccd a bit. */
	req("SERVER-PART %s %s %s", argv[1], argv[2],
	    argc >= 4 ? argv[3] : "irccd is shutting down");
	ok();
}

static void
cmd_server_reconnect(int argc, char **argv)
{
	if (argc == 2)
		req("SERVER-RECONNECT %s", argv[1]);
	else
		req("SERVER-RECONNECT");

	ok();
}

static void
cmd_server_topic(int, char **argv)
{
	req("SERVER-TOPIC %s %s %s", argv[1], argv[2], argv[3]);
	ok();
}

static void
cmd_watch(int, char **)
{
	struct timeval tv = {};
	char *ev;

	/* Enable watch. */
	req("WATCH");
	ok();

	/* Turn off timeout to receive indefinitely. */
	if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof (tv)) < 0)
		irc_util_die("abort: setsockopt: %s\n", strerror(errno));

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
	{ "server-part",        2,      3,      cmd_server_part         },
	{ "server-reconnect",   0,      1,      cmd_server_reconnect    },
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
		irc_util_die("abort: command not found\n");
	if ((c->minargs != -1 && argc - 1 < c->minargs) || (c->minargs != -1 && argc - 1 > c->maxargs))
		irc_util_die("abort: invalid number of arguments\n");

	c->exec(argc, argv);
}

noreturn static void
usage(void)
{
	fprintf(stderr, "usage: irccdctl [-v] [-s sock] command [options...] [arguments...]\n");
	exit(1);
}

noreturn static void
help(void)
{
	fprintf(stderr, "usage: irccdctl hook-add name path\n");
	fprintf(stderr, "       irccdctl hook-list\n");
	fprintf(stderr, "       irccdctl hook-remove id\n");
	fprintf(stderr, "       irccdctl plugin-config id [variable [value]]\n");
	fprintf(stderr, "       irccdctl plugin-info id\n");
	fprintf(stderr, "       irccdctl plugin-list\n");
	fprintf(stderr, "       irccdctl plugin-load name\n");
	fprintf(stderr, "       irccdctl plugin-path id [variable [value]]\n");
	fprintf(stderr, "       irccdctl plugin-template id [variable [value]]\n");
	fprintf(stderr, "       irccdctl plugin-reload [id]\n");
	fprintf(stderr, "       irccdctl plugin-unload [id]\n");
	fprintf(stderr, "       irccdctl rule-add [-c channel] [-e event] [-i index] [-o origin] [-p plugin] [-s server] accept|drop\n");
	fprintf(stderr, "       irccdctl rule-edit [-a accept|drop] [-c|C channel] [-e|E event] [-o|O origin] [-s|S server] index\n");
	fprintf(stderr, "       irccdctl rule-list\n");
	fprintf(stderr, "       irccdctl rule-move from to\n");
	fprintf(stderr, "       irccdctl rule-remove index\n");
	fprintf(stderr, "       irccdctl server-connect [-n nickname] [-r realname] [-u username] [-p port] id hostname\n");
	fprintf(stderr, "       irccdctl server-disconnect [server]\n");
	fprintf(stderr, "       irccdctl server-info server\n");
	fprintf(stderr, "       irccdctl server-invite server target channel\n");
	fprintf(stderr, "       irccdctl server-join server channel [password]\n");
	fprintf(stderr, "       irccdctl server-kick server target channel [reason]\n");
	fprintf(stderr, "       irccdctl server-list\n");
	fprintf(stderr, "       irccdctl server-me server target message\n");
	fprintf(stderr, "       irccdctl server-message server target message\n");
	fprintf(stderr, "       irccdctl server-mode server target mode [args]\n");
	fprintf(stderr, "       irccdctl server-nick server nickname\n");
	fprintf(stderr, "       irccdctl server-notice server target message\n");
	fprintf(stderr, "       irccdctl server-part server channel [reason]\n");
	fprintf(stderr, "       irccdctl server-reconnect [server]\n");
	fprintf(stderr, "       irccdctl server-topic server channel topic\n");
	fprintf(stderr, "       irccdctl watch\n");
	exit(1);
}

int
main(int argc, char **argv)
{
	int ch;

	putenv("POSIXLY_CORRECT=1");

	while ((ch = getopt(argc, argv, "s:v")) != -1) {
		switch (ch) {
		case 's':
			irc_util_strlcpy(sockaddr.sun_path, optarg, sizeof (sockaddr.sun_path));
			break;
		case 'v':
			verbose = 1;
			break;
		default:
			usage();
			break;
		}
	}

	argc -= optind;
	argv += optind;

	/* Reset options for subcommands. */
	optind = 1;

	if (argc < 1)
		usage();
	else if (strcmp(argv[0], "help") == 0)
		help();

	dial();
	check();
	run(argc, argv);
}
