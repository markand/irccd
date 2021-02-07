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

#include <irccd/irccd.h>
#include <irccd/log.h>
#include <irccd/server.h>
#include <irccd/util.h>

#include "peer.h"

typedef void (*plugin_set_fn)(struct irc_plugin *, const char *, const char *);
typedef const char * (*plugin_get_fn)(struct irc_plugin *, const char *);
typedef const char ** (*plugin_list_fn)(struct irc_plugin *);

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

static inline struct irc_server *
require_server(struct peer *p, const char *id)
{
	struct irc_server *s;

	if (!(s = irc_bot_server_get(id))) {
		peer_send(p, "server %s not found", id);
		return NULL;
	}

	return s;
}

static inline struct irc_plugin *
require_plugin(struct peer *p, const char *id)
{
	struct irc_plugin *plg;

	if (!(plg = irc_bot_plugin_get(id))) {
		peer_send(p, "plugin %s not found", id);
		return NULL;
	}

	return plg;
}

static int
ok(struct peer *p)
{
	peer_send(p, "OK");

	return 0;
}

static int
plugin_list_set(struct peer *p,
                char *line,
                plugin_set_fn set,
                plugin_get_fn get,
                plugin_list_fn list)
{
	const char *args[3] = {0}, *value, **keys;
	char out[IRC_BUF_LEN];
	FILE *fp;
	struct irc_plugin *plg;
	size_t argsz, keysz = 0;

	if ((argsz = parse(line, args, 3)) < 1)
		return EINVAL;
	if (!(plg = require_plugin(p, args[0])))
		return 0;

	fp = fmemopen(out, sizeof (out) - 1, "w");

	if (argsz == 3) {
		set(plg, args[1], args[2]);
		fprintf(fp, "OK");
	} else if (argsz == 2) {
		if ((value = get(plg, args[1])))
			fprintf(fp, "OK 1\n%s", value);
		else
			fprintf(fp, "ERROR key not found");
	} else {
		keys = list(plg);

		/* Compute the number of keys found. */
		for (const char **key = keys; key && *key; ++key)
			keysz++;

		fprintf(fp, "OK %zu\n", keysz);

		for (const char **key = keys; key && *key; ++key) {
			value = get(plg, *key);
			fprintf(fp, "%s=%s\n", *key, value ? value : "");
		}
	}

	fclose(fp);
	peer_send(p, out);

	return 0;
}

/*
 * PLUGIN-CONFIG plugin [var [value]]
 */
static int
cmd_plugin_config(struct peer *p, char *line)
{
	return plugin_list_set(p, line,
	    irc_plugin_set_option, irc_plugin_get_option, irc_plugin_get_options);
}

/*
 * PLUGIN-INFO plugin
 */
static int
cmd_plugin_info(struct peer *p, char *line)
{
	struct irc_plugin *plg;
	const char *args[1];

	if (parse(line, args, 1) != 1)
		return EINVAL;
	if (!(plg = require_plugin(p, args[0])))
		return 0;

	peer_send(p, "OK %s\n%s\n%s\n%s\n%s", plg->name, plg->description,
	    plg->version, plg->license, plg->author);

	return 0;
}

/*
 * PLUGIN-LOAD plugin
 */
static int
cmd_plugin_load(struct peer *p, char *line)
{
	struct irc_plugin *plg;

	if (!(plg = irc_bot_plugin_find(line, NULL)))
		peer_send(p, "could not load plugin: %s", strerror(errno));

	/* TODO: report error if fails to open. */
	irc_bot_plugin_add(plg);

	return ok(p);
}

/*
 * PLUGIN-PATH plugin [var [value]]
 */
static int
cmd_plugin_path(struct peer *p, char *line)
{
	return plugin_list_set(p, line,
	    irc_plugin_set_path, irc_plugin_get_path, irc_plugin_get_paths);
}

/*
 * PLUGIN-LIST
 */
static int
cmd_plugin_list(struct peer *p, char *line)
{
	(void)line;

	struct irc_plugin *plg;
	FILE *fp;
	char out[IRC_BUF_LEN];

	fp = fmemopen(out, sizeof (out) - 1, "w");

	fprintf(fp, "OK ");

	LIST_FOREACH(plg, &irc.plugins, link) {
		fprintf(fp, "%s", plg->name);

		if (LIST_NEXT(plg, link))
			fputc(' ', fp);
	}

	fclose(fp);
	peer_send(p, out);

	return 0;
}

/*
 * PLUGIN-RELOAD plugin
 */
static int
cmd_plugin_reload(struct peer *p, char *line)
{
	struct irc_plugin *plg;

	if (!(plg = irc_bot_plugin_get(line)))
		peer_send(p, "could not reload plugin: %s", strerror(errno));

	/* TODO: report error if fails to reload. */

	return ok(p);
}

/*
 * PLUGIN-TEMPLATE plugin [var [value]]
 */
static int
cmd_plugin_template(struct peer *p, char *line)
{
	return plugin_list_set(p, line,
	    irc_plugin_set_template, irc_plugin_get_template, irc_plugin_get_templates);
}

/*
 * PLUGIN-UNLOAD [plugin]
 */
static int
cmd_plugin_unload(struct peer *p, char *line)
{
	const char *args[1] = {0};

	/* TODO report error if plugin not found. */
	if (parse(line, args, 1) == 0)
		irc_bot_plugin_clear();
	else
		irc_bot_plugin_remove(args[0]);

	return ok(p);
}

/*
 * SERVER-DISCONNECT [server]
 */
static int
cmd_server_disconnect(struct peer *p, char *line)
{
	const char *args[1] = {0};
	struct irc_server *s;

	if (parse(line, args, 1) == 1) {
		if (!(s = require_server(p, args[0])))
			return 0;

		irc_server_disconnect(s);
	} else
		irc_bot_server_clear();

	return ok(p);
}

/*
 * SERVER-MESSAGE server channel message
 */
static int
cmd_server_message(struct peer *p, char *line)
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
 * SERVER-ME server channel message
 */
static int
cmd_server_me(struct peer *p, char *line)
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
 * SERVER-MODE server channel mode [limit] [user] [mask]
 */
static int
cmd_server_mode(struct peer *p, char *line)
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
 * SERVER-NOTICE server channel message
 */
static int
cmd_server_notice(struct peer *p, char *line)
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
 * SERVER-INFO server
 */
static int
cmd_server_info(struct peer *p, char *line)
{
	const char *args[1] = {0};
	const struct irc_server *s;
	const struct irc_channel *c;
	char out[IRC_BUF_LEN];
	FILE *fp;

	if (parse(line, args, 1) != 1)
		return EINVAL;
	if (!(s = require_server(p, args[0])))
		return 0;

	fp = fmemopen(out, sizeof (out) - 1, "w");

	fprintf(fp, "OK %s\n", s->name);
	fprintf(fp, "%s %u%s\n", s->conn.hostname, s->conn.port,
	    s->flags & IRC_SERVER_FLAGS_SSL ? " ssl" : "");
	fprintf(fp, "%s %s %s\n", s->ident.nickname, s->ident.username, s->ident.realname);

	LIST_FOREACH(c, &s->channels, link) {
		fprintf(fp, "%s", c->name);

		if (LIST_NEXT(c, link))
			fputc(' ', fp);
	}

	fclose(fp);
	peer_send(p, out);

	return 0;
}

/*
 * SERVER-INVITE server channel target
 */
static int
cmd_server_invite(struct peer *p, char *line)
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
 * SERVER-JOIN server channel [password]
 */
static int
cmd_server_join(struct peer *p, char *line)
{
	const char *args[3] = {0};
	struct irc_server *s;

	if (parse(line, args, 3) < 2)
		return EINVAL;
	if (!(s = require_server(p, args[0])))
		return 0;

	irc_server_join(s, args[1], args[2] ? args[2] : NULL);

	return ok(p);
}

/*
 * SERVER-KICK server channel target [reason]
 */
static int
cmd_server_kick(struct peer *p, char *line)
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

/*
 * SERVER-LIST
 */
static int
cmd_server_list(struct peer *p, char *line)
{
	(void)line;

	struct irc_server *s;
	FILE *fp;
	char *out;
	size_t outsz;

	fp = open_memstream(&out, &outsz);

	fprintf(fp, "OK ");

	LIST_FOREACH(s, &irc.servers, link) {
		fprintf(fp, "%s", s->name);

		if (LIST_NEXT(s, link))
			fputc(' ', fp);
	}

	fclose(fp);
	peer_send(p, out);
	free(out);

	return 0;
}

/*
 * SERVER-PART server channel [reason]
 */
static int
cmd_server_part(struct peer *p, char *line)
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
 * SERVER-TOPIC server channel topic
 */
static int
cmd_server_topic(struct peer *p, char *line)
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

static int
cmd_watch(struct peer *p, char *line)
{
	(void)line;

	p->is_watching = 1;

	return ok(p);
}

static const struct cmd {
	const char *name;
	int (*call)(struct peer *, char *);
} cmds[] = {
	{ "PLUGIN-CONFIG",      cmd_plugin_config       },
	{ "PLUGIN-INFO",        cmd_plugin_info         },
	{ "PLUGIN-LIST",        cmd_plugin_list         },
	{ "PLUGIN-LOAD",        cmd_plugin_load         },
	{ "PLUGIN-PATH",        cmd_plugin_path         },
	{ "PLUGIN-RELOAD",      cmd_plugin_reload       },
	{ "PLUGIN-TEMPLATE",    cmd_plugin_template     },
	{ "PLUGIN-UNLOAD",      cmd_plugin_unload       },
	{ "SERVER-DISCONNECT",  cmd_server_disconnect   },
	{ "SERVER-INFO",        cmd_server_info         },
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
	{ "WATCH",              cmd_watch               }
};

static int
cmp_cmd(const char *key, const struct cmd *cmd)
{
	return strncmp(key, cmd->name, strlen(cmd->name));
}

static const struct cmd *
find(const char *line)
{
	return bsearch(line, cmds, IRC_UTIL_SIZE(cmds),
	    sizeof (cmds[0]), (irc_cmp)cmp_cmd);
}

static void
invoke(struct peer *p, char *line)
{
	const struct cmd *c = find(line);
	int er;

	if (!c)
		peer_send(p, "command not found");
	else if ((er = c->call(p, line)) != 0)
		peer_send(p, "%s", strerror(errno));
}

static void
dispatch(struct peer *p)
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

static int
input(struct peer *p)
{
	char buf[BUFSIZ + 1];
	ssize_t nr;

	if ((nr = recv(p->fd, buf, BUFSIZ, 0)) <= 0) {
		irc_log_info("transport: client disconnect");
		return -1;
	}

	buf[nr] = '\0';

	if (strlcat(p->in, buf, sizeof (p->in)) >= sizeof (p->in)) {
		errno = EMSGSIZE;
		return -1;
	}

	dispatch(p);

	return 0;
}

static int
output(struct peer *p)
{
	ssize_t ns;
	size_t len = strlen(p->out);

	if ((ns = send(p->fd, p->out, len, 0)) < 0)
		return -1;

	if ((size_t)ns >= len)
		memset(p->out, 0, sizeof (p->out));
	else
		memmove(p->out, p->out + ns, sizeof (p->out) - ns);

	return 0;
}

struct peer *
peer_new(int fd)
{
	struct peer *p;

	p = irc_util_calloc(1, sizeof (*p));
	p->fd = fd;

	return p;
}

int
peer_send(struct peer *p, const char *fmt, ...)
{
	assert(p);
	assert(fmt);

	char buf[IRC_BUF_LEN];
	va_list ap;
	size_t len, avail, required;

	va_start(ap, fmt);
	required = vsnprintf(buf, sizeof (buf), fmt, ap);
	va_end(ap);

	len = strlen(p->out);
	avail = sizeof (p->out) - len;

	/* Don't forget \n. */
	if (required + 1 >= avail)
		return -1;

	strlcat(p->out, buf, sizeof (p->out));
	strlcat(p->out, "\n", sizeof (p->out));

	return 0;
}

void
peer_prepare(struct peer *p, struct pollfd *fd)
{
	assert(p);
	assert(fd);

	fd->fd = p->fd;;
	fd->events = POLLIN;

	if (p->out[0])
		fd->events |= POLLOUT;
}

int
peer_flush(struct peer *p, const struct pollfd *fd)
{
	assert(p);
	assert(fd);

	if (fd->fd != p->fd)
		return -1;
	if (fd->revents & POLLIN && input(p) < 0)
		return -1;
	if (fd->revents & POLLOUT && output(p) < 0)
		return -1;

	return 0;
}

void
peer_finish(struct peer *p)
{
	assert(p);

	close(p->fd);
	free(p);
}
