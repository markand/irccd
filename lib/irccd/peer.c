/*
 * peer.c -- client connected to irccd
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
#include <ctype.h>
#include <errno.h>
#include <poll.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <assert.h>

#include "irccd.h"
#include "log.h"
#include "peer.h"
#include "server.h"
#include "util.h"

static size_t
parse(char *line, const char **args, size_t max)
{
	size_t idx;

	/* Skip command. */
	while (*line && !isspace(*line++))
		continue;

	if (!*line)
		return 0;

	for (idx = 0; idx < max; ++idx) {
		char *sp = strchr(line, ' ');

		if (!sp || idx + 1 >= max) {
			args[idx++] = line;
			break;
		}

		*sp = '\0';
		args[idx] = line;
		line = sp + 1;
	}

	return idx;
}

static struct irc_server *
require_server(struct irc_peer *p, const char *id)
{
	struct irc_server *s;

	if (!(s = irc_bot_find_server(id))) {
		irc_peer_send(p, "server %s not found", id);
		return NULL;
	}

	return s;
}

static int
ok(struct irc_peer *p)
{
	irc_peer_send(p, "OK");

	return 0;
}

/*
 * MESSAGE server channel message
 */
static int
cmd_server_message(struct irc_peer *p, char *line)
{
	const char *args[3] = {0};
	struct irc_server *s;

	if (parse(line, args, 3) != 3)
		return EINVAL;
	if (!(s = require_server(p, args[0])))
		return 0;

	irc_server_message(s, args[1], args[2]);

	return ok(p);
}

/*
 * ME server channel message
 */
static int
cmd_server_me(struct irc_peer *p, char *line)
{
	const char *args[3] = {0};
	struct irc_server *s;

	if (parse(line, args, 3) != 3)
		return EINVAL;
	if (!(s = require_server(p, args[0])))
		return 0;

	irc_server_me(s, args[1], args[2]);

	return ok(p);
}

/*
 * MODE server channel mode [limit] [user] [mask]
 */
static int
cmd_server_mode(struct irc_peer *p, char *line)
{
	const char *args[6] = {0};
	struct irc_server *s;

	if (parse(line, args, 6) < 3)
		return EINVAL;
	if (!(s = require_server(p, args[0])))
		return 0;

	irc_server_mode(s, args[1], args[2],
	    args[3][0] ? args[3] : NULL,
	    args[4][0] ? args[4] : NULL,
	    args[5][0] ? args[5] : NULL
	);

	return ok(p);
}

/*
 * NOTICE server channel message
 */
static int
cmd_server_notice(struct irc_peer *p, char *line)
{
	const char *args[3] = {0};
	struct irc_server *s;

	if (parse(line, args, 3) != 3)
		return EINVAL;
	if (!(s = require_server(p, args[0])))
		return 0;

	irc_server_notice(s, args[1], args[2]);

	return ok(p);
}

/*
 * INVITE server channel target
 */
static int
cmd_server_invite(struct irc_peer *p, char *line)
{
	const char *args[3] = {0};
	struct irc_server *s;

	if (parse(line, args, 3) != 3)
		return EINVAL;
	if (!(s = require_server(p, args[0])))
		return 0;

	irc_server_invite(s, args[1], args[2]);

	return ok(p);
}

/*
 * JOIN server channel [password]
 */
static int
cmd_server_join(struct irc_peer *p, char *line)
{
	const char *args[3] = {0};
	struct irc_server *s;

	if (parse(line, args, 3) < 2)
		return EINVAL;
	if (!(s = require_server(p, args[0])))
		return 0;

	irc_server_join(s, args[1], args[2][0] ? args[2] : NULL);

	return ok(p);
}

/*
 * KICK server channel target [reason]
 */
static int
cmd_server_kick(struct irc_peer *p, char *line)
{
	const char *args[4] = {0};
	struct irc_server *s;

	if (parse(line, args, 4) < 3)
		return EINVAL;
	if (!(s = require_server(p, args[0])))
		return 0;

	irc_server_kick(s, args[1], args[2], args[3][0] ? args[3] : NULL);

	return ok(p);
}

static int
cmd_server_list(struct irc_peer *p, char *line)
{
	char out[IRC_BUF_MAX] = "OK ";

	for (size_t i = 0; i < irc.serversz; ++i) {
		if (strlcat(out, irc.servers[i].name, sizeof (out)) >= sizeof (out))
			return EMSGSIZE;
		if (i + 1 < irc.serversz && strlcat(out, " ", sizeof (out)) >= sizeof (out))
			return EMSGSIZE;
	}

	irc_peer_send(p, out);

	return 0;
}

/*
 * PART server channel [reason]
 */
static int
cmd_server_part(struct irc_peer *p, char *line)
{
	const char *args[3] = {0};
	struct irc_server *s;

	if (parse(line, args, 3) < 2)
		return EINVAL;
	if (!(s = require_server(p, args[0])))
		return 0;

	irc_server_part(s, args[1], args[2][0] ? args[2] : NULL);

	return ok(p);
}

/*
 * TOPIC server channel topic
 */
static int
cmd_server_topic(struct irc_peer *p, char *line)
{
	const char *args[3] = {0};
	struct irc_server *s;

	if (parse(line, args, 3) != 3)
		return EINVAL;
	if (!(s = require_server(p, args[0])))
		return 0;

	irc_server_topic(s, args[1], args[2]);

	return ok(p);
}

static const struct cmd {
	const char *name;
	int (*call)(struct irc_peer *, char *);
} cmds[] = {
	{ "SERVER-INVITE",      cmd_server_invite       },
	{ "SERVER-JOIN",        cmd_server_join         },
	{ "SERVER-KICK",        cmd_server_kick         },
	{ "SERVER-LIST",        cmd_server_list         },
	{ "SERVER-ME",          cmd_server_me           },
	{ "SERVER-MESSAGE",     cmd_server_message      },
	{ "SERVER-MODE",        cmd_server_mode         },
	{ "SERVER-NOTICE",      cmd_server_notice       },
	{ "SERVER-PART",        cmd_server_part         },
	{ "SERVER-TOPIC",       cmd_server_topic        },
};

static int
cmp_cmd(const void *d1, const void *d2)
{
	const char *key = d1;
	const struct cmd *cmd = d2;

	return strncmp(key, cmd->name, strlen(cmd->name));
}

static const struct cmd *
find(const char *line)
{
	return bsearch(line, cmds, IRC_UTIL_SIZE(cmds), sizeof (struct cmd), cmp_cmd);
}

static void
invoke(struct irc_peer *p, char *line)
{
	const struct cmd *c = find(line);
	int er;

	if (!c)
		irc_peer_send(p, "command not found");
	else if ((er = c->call(p, line)) != 0)
		irc_peer_send(p, "%s", strerror(errno));
}

static void
dispatch(struct irc_peer *p)
{
	char *pos;
	size_t length;

	while ((pos = strstr(p->in, "\n"))) {
		/* Turn end of the string at delimiter. */
		*pos = 0;
		length = pos - p->in;

		if (length > 0)
			invoke(p, p->in);

		memmove(p->in, pos + 1, sizeof (p->in) - (length + 1));
	}
}

static bool
input(struct irc_peer *p)
{
	char buf[BUFSIZ + 1];
	ssize_t nr;

	if ((nr = recv(p->fd, buf, BUFSIZ, 0)) <= 0) {
		irc_log_info("transport: client disconnect");
		return false;
	}

	buf[nr] = '\0';

	if (strlcat(p->in, buf, sizeof (p->in)) >= sizeof (p->in)) {
		errno = EMSGSIZE;
		return false;
	}

	dispatch(p);

	return true;
}

static bool
output(struct irc_peer *p)
{
	ssize_t ns;
	size_t len = strlen(p->out);

	if ((ns = send(p->fd, p->out, len, 0)) < 0)
		return false;

	if (ns >= len)
		memset(p->out, 0, sizeof (p->out));
	else
		memmove(p->out, p->out + ns, sizeof (p->out) - ns);

	return true;
}

bool
irc_peer_send(struct irc_peer *p, const char *fmt, ...)
{
	assert(p);
	assert(fmt);

	char buf[IRC_BUF_MAX];
	va_list ap;
	size_t len, avail, required;

	va_start(ap, fmt);
	required = vsnprintf(buf, sizeof (buf), fmt, ap);
	va_end(ap);

	len = strlen(p->out);
	avail = sizeof (p->out) - len;

	/* Don't forget \n. */
	if (required + 1 >= avail)
		return false;

	strlcat(p->out, buf, sizeof (p->out));
	strlcat(p->out, "\n", sizeof (p->out));

	return true;
}

void
irc_peer_prepare(struct irc_peer *p, struct pollfd *fd)
{
	assert(p);
	assert(fd);

	fd->fd = p->fd;;
	fd->events = POLLIN;

	if (p->out[0])
		fd->events |= POLLOUT;
}

bool
irc_peer_flush(struct irc_peer *p, const struct pollfd *fd)
{
	assert(p);
	assert(fd);

	char buf[IRC_BUF_MAX];

	if (fd->fd != p->fd)
		return true;

	if (fd->revents & POLLIN && !input(p))
		return false;
	if (fd->revents & POLLOUT && !output(p))
		return false;

	return true;
}

void
irc_peer_finish(struct irc_peer *p)
{
	assert(p);

	close(p->fd);
	memset(p, 0, sizeof (*p));
}
