/*
 * peer.c -- client connected to irccd
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

#include <sys/socket.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <utlist.h>

#include <irccd/hook.h>
#include <irccd/irccd.h>
#include <irccd/log.h>
#include <irccd/plugin.h>
#include <irccd/rule.h>
#include <irccd/server.h>
#include <irccd/util.h>

#include "peer.h"

typedef void (*plugin_set_fn)(struct irc_plugin *, const char *, const char *);
typedef const char * (*plugin_get_fn)(struct irc_plugin *, const char *);
typedef const char * const * (*plugin_list_fn)(struct irc_plugin *);

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
		peer_push(p, "server %s not found", id);
		return NULL;
	}

	return s;
}

static inline struct irc_plugin *
require_plugin(struct peer *p, const char *id)
{
	struct irc_plugin *plg;

	if (!(plg = irc_bot_plugin_get(id))) {
		peer_push(p, "plugin %s not found", id);
		return NULL;
	}

	return plg;
}

static inline int
ok(struct peer *p)
{
	peer_push(p, "OK");

	return 0;
}

static inline int
error(struct peer *p, const char *fmt, ...)
{
	char buf[IRC_BUF_LEN] = {0};
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof (buf), fmt, ap);
	va_end(ap);

	if (buf[0])
		peer_push(p, "ERROR %s", buf);

	return 0;
}

static int
plugin_list_set(struct peer *p,
                char *line,
                plugin_set_fn set,
                plugin_get_fn get,
                plugin_list_fn list)
{
	const char *args[3] = {0}, *value;
	const char * const *keys;
	char out[IRC_BUF_LEN];
	FILE *fp;
	struct irc_plugin *plg;
	size_t argsz, keysz = 0;

	if ((argsz = parse(line, args, 3)) < 1)
		return EINVAL;
	if (!(plg = require_plugin(p, args[0])))
		return 0;
	if (!(fp = fmemopen(out, sizeof (out) - 1, "w")))
		return errno;

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
		for (const char * const *key = keys; key && *key; ++key)
			keysz++;

		fprintf(fp, "OK %zu\n", keysz);

		for (const char * const *key = keys; key && *key; ++key) {
			value = get(plg, *key);
			fprintf(fp, "%s=%s\n", *key, value ? value : "");
		}
	}

	fclose(fp);
	peer_push(p, "%s", out);

	return 0;
}

#if 0

static const char *
rule_list_to_spaces(char * const *value)
{
	static char buf[IRC_RULE_LEN];

	irc_util_strlcpy(buf, value, sizeof (buf));

	for (char *p = buf; *p; ++p)
		if (*p == ':')
			*p = ' ';

	return buf;
}

#endif

/*
 * HOOK-ADD name path
 */
static int
cmd_hook_add(struct peer *p, char *line)
{
	const char *args[2] = {0};

	if (parse(line, args, 2) != 2)
		return EINVAL;
	if (irc_bot_hook_get(args[0]))
		return EEXIST;

	irc_bot_hook_add(irc_hook_new(args[0], args[1]));

	return ok(p);
}

/*
 * HOOK-LIST
 */
static int
cmd_hook_list(struct peer *p, char *line)
{
	(void)line;

	struct irc_hook *h;
	char out[IRC_BUF_LEN];
	FILE *fp;

	if (!(fp = fmemopen(out, sizeof (out) - 1, "w")))
		return errno;

	fprintf(fp, "OK ");

	LL_FOREACH(irccd->hooks, h) {
		fprintf(fp, "%s", h->name);

		if (h->next)
			fputc(' ', fp);
	}

	fclose(fp);
	peer_push(p, "%s", out);

	return 0;
}

/*
 * HOOK-REMOVE name
 */
static int
cmd_hook_remove(struct peer *p, char *line)
{
	const char *args[1] = {0};

	if (parse(line, args, 1) != 1)
		return EINVAL;

	irc_bot_hook_remove(args[0]);

	return ok(p);
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

	peer_push(p, "OK %s\n%s\n%s\n%s\n%s", plg->name, plg->description,
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
	const char *args[1];

	if (parse(line, args, 1) != 1)
		return EINVAL;
	if (!(plg = irc_bot_plugin_search(args[0], NULL)))
		peer_push(p, "could not load plugin: %s", strerror(errno));
	else
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

	if (!(fp = fmemopen(out, sizeof (out) - 1, "w")))
		return errno;

	fprintf(fp, "OK ");

	LL_FOREACH(irccd->plugins, plg) {
		fprintf(fp, "%s", plg->name);

		if (plg->next)
			fputc(' ', fp);
	}

	fclose(fp);
	peer_push(p, "%s", out);

	return 0;
}

/*
 * PLUGIN-RELOAD [plugin]
 */
static int
cmd_plugin_reload(struct peer *p, char *line)
{
	struct irc_plugin *plg;
	const char *args[1] = {0};

	if (parse(line, args, 1) == 1) {
		if (!(plg = irc_bot_plugin_get(args[0])))
			return peer_push(p, "could not reload plugin: %s", strerror(ENOENT)), 0;

		irc_plugin_reload(plg);
	} else
		DL_FOREACH(irccd->plugins, plg)
			irc_plugin_reload(plg);

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
 * RULE-ADD accept|drop [(ceiops)=value ...]
 */
static int
cmd_rule_add(struct peer *p, char *line)
{
	char *token, *ptr, key;
	enum irc_rule_action act;
	struct irc_rule *rule;
	unsigned long long index = -1;
	void (*add)(struct irc_rule *, const char *);

	if (sscanf(line, "RULE-ADD %*s") == EOF)
		return EINVAL;

	line += strlen("RULE-ADD ");

	if (strncmp(line, "accept", 6) == 0)
		act = IRC_RULE_ACCEPT;
	else if (strncmp(line, "drop", 4) == 0)
		act = IRC_RULE_DROP;
	else
		return error(p, "invalid action");

	rule = irc_rule_new(act);

	/* Skip action value. */
	while (*line && !isspace(*line))
		++line;
	while (*line && isspace(*line))
		++line;

	for (ptr = line; (token = strtok_r(ptr, " ", &ptr)); ) {
		if (sscanf(token, "%c=%*s", &key) != 1) {
			errno = EINVAL;
			goto fail;
		}

		switch (*token) {
		case 'c':
			add = irc_rule_add_channel;
			break;
		case 'e':
			add = irc_rule_add_event;
			break;
			break;
		case 'i':
			if (irc_util_stou(token + 2, &index) < 0)
				goto fail;
			break;
		case 'o':
			add = irc_rule_add_origin;
			break;
		case 'p':
			add = irc_rule_add_plugin;
			break;
		case 's':
			add = irc_rule_add_server;
			break;
		default:
			/* TODO: error here. */
			add = NULL;
			break;
		}

		if (add)
			add(rule, token + 2);
	}

	irc_bot_rule_insert(rule, index);

	return ok(p);

fail:
	irc_rule_free(rule);

	return error(p, strerror(errno));
}

/*
 * RULE-EDIT index [((ceops)(+-)value)|(a=accept|drop) ...]
 */
static int
cmd_rule_edit(struct peer *p, char *line)
{
	char *token, *ptr, key, attr;
	struct irc_rule *rule;
	size_t index = -1;
	void (*add)(struct irc_rule *, const char *);
	void (*remove)(struct irc_rule *, const char *);

	/*
	 * Looks like strtonum does not accept when there is text after the
	 * number.
	 */
	if (sscanf(line, "RULE-EDIT %zu", &index) != 1)
		return EINVAL;

	if (index >= irc_bot_rule_size())
		return ERANGE;

	/* Skip command and index value. */
	line += strlen("RULE-EDIT ");

	while (*line && !isspace(*line))
		++line;
	while (*line && isspace(*line))
		++line;

	rule = irc_bot_rule_get(index);

	for (ptr = line; (token = strtok_r(ptr, " ", &ptr)); ) {
		key = attr = 0;

		if (sscanf(token, "%c%c%*s", &key, &attr) != 2)
			return EINVAL;

		if (key == 'a') {
			if (attr != '=')
				return EINVAL;

			if (strncmp(token + 2, "accept", 6) == 0)
				rule->action = IRC_RULE_ACCEPT;
			else if (strncmp(token + 2, "drop", 4) == 0)
				rule->action = IRC_RULE_DROP;
			else
				return error(p, "invalid action");
		} else {
			add = NULL;
			remove = NULL;

			switch (key) {
			case 'c':
				add = irc_rule_add_channel;
				remove = irc_rule_remove_channel;
				break;
			case 'e':
				add = irc_rule_add_event;
				remove = irc_rule_remove_event;
				break;
			case 'o':
				add = irc_rule_add_origin;
				remove = irc_rule_remove_origin;
				break;
			case 'p':
				add = irc_rule_add_plugin;
				remove = irc_rule_remove_plugin;
				break;
			case 's':
				add = irc_rule_add_server;
				remove = irc_rule_remove_server;
				break;
			default:
				return EINVAL;
			}

			if (attr == '+')
				add(rule, token + 2);
			else if (attr == '-')
				remove(rule, token + 2);
			else
				return EINVAL;
		}
	}

	return ok(p);
}

/*
 * RULE-LIST
 */
static int
cmd_rule_list(struct peer *p, char *line)
{
	(void)line;

	struct irc_rule *rule;
	char out[IRC_BUF_LEN];
	FILE *fp;
	size_t rulesz = 0;

	if (!(fp = fmemopen(out, sizeof (out), "w")))
		return error(p, "%s", strerror(errno));

	DL_FOREACH(irccd->rules, rule)
		rulesz++;

	fprintf(fp, "OK %zu\n", rulesz);

	DL_FOREACH(irccd->rules, rule) {
		fprintf(fp, "%s\n", rule->action == IRC_RULE_ACCEPT ? "accept" : "drop");
#if 0
		fprintf(fp, "%s\n", rule_list_to_spaces(rule->servers));
		fprintf(fp, "%s\n", rule_list_to_spaces(rule->channels));
		fprintf(fp, "%s\n", rule_list_to_spaces(rule->origins));
		fprintf(fp, "%s\n", rule_list_to_spaces(rule->plugins));
		fprintf(fp, "%s\n", rule_list_to_spaces(rule->events));
#endif
	}

	if (feof(fp) || ferror(fp)) {
		fclose(fp);
		return EMSGSIZE;
	}

	fclose(fp);
	peer_push(p, "%s", out);

	return 0;
}

/*
 * RULE-MOVE from to
 */
static int
cmd_rule_move(struct peer *p, char *line)
{
	const char *args[2];
	unsigned long long from, to;

	if (parse(line, args, 2) != 2)
		return EINVAL;
	if (irc_util_stou(args[0], &from) < 0)
		return ERANGE;
	if (irc_util_stou(args[1], &to) < 0)
		return ERANGE;
	if (from >= irc_bot_rule_size())
		return ERANGE;

	irc_bot_rule_move(from, to);

	return ok(p);
}

/*
 * RULE-REMOVE index
 */
static int
cmd_rule_remove(struct peer *p, char *line)
{
	const char *args[1] = {0};
	unsigned long long index;

	if (parse(line, args, 1) != 1)
		return EINVAL;
	if (irc_util_stou(args[0], &index) < 0 || index >= irc_bot_rule_size())
		return ERANGE;

	irc_bot_rule_remove(index);

	return ok(p);
}

/*
 * SERVER-CONNECT server host [+]port nickname username realname
 */
static int
cmd_server_connect(struct peer *p, char *line)
{
	(void)line;

#if 0
	const char *args[6] = {0};
	int ssl;
	struct irc_server *s;

	if (parse(line, args, 6) != 6)
		return EINVAL;
	if (irc_bot_server_get(args[0]))
		return EEXIST;

	/* If port starts with +, it means SSL support. */
	if ((ssl = args[2][0] == '+'))
		++args[2];

	s = irc_server_new(args[0], args[3], args[4], args[5], args[1], atoi(args[2]));
	s->flags |= IRC_SERVER_FLAGS_AUTO_RECO;

	if (ssl)
		s->flags |= IRC_SERVER_FLAGS_SSL;

	irc_bot_server_add(s);
#endif

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
 * SERVER-MODE server channel mode [args...]
 */
static int
cmd_server_mode(struct peer *p, char *line)
{
	const char *args[4] = {0};
	struct irc_server *s;

	if (parse(line, args, 4) < 3)
		return EINVAL;
	if (!(s = require_server(p, args[0])))
		return 0;

	irc_server_mode(s, args[1], args[2], args[3]);

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
	if (!(fp = fmemopen(out, sizeof (out) - 1, "w")))
		return errno;

	fprintf(fp, "OK %s\n", s->name);
	fprintf(fp, "%s %u%s\n", s->hostname, s->port,
	    s->flags & IRC_SERVER_FLAGS_SSL ? " ssl" : "");
	fprintf(fp, "%s %s %s\n", s->nickname, s->username, s->realname);

	LL_FOREACH(s->channels, c) {
		const struct irc_channel_user *user = irc_channel_get(c, s->nickname);

		/* Prefix all our own modes on this channel. */
		for (size_t i = 0; i < s->prefixesz; ++i)
			if (user && (user->modes & 1 << i))
				fputc(s->prefixes[i].symbol, fp);

		if (c->flags & IRC_CHANNEL_FLAGS_JOINED)
			fprintf(fp, "%s", c->name);
		else
			fprintf(fp, "(%s)", c->name);

		if (c->next)
			fputc(' ', fp);
	}

	fclose(fp);
	peer_push(p, "%s", out);

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
	char out[IRC_BUF_LEN];
	FILE *fp;

	if (!(fp = fmemopen(out, sizeof (out), "w")))
		return error(p, "%s", strerror(errno));

	fprintf(fp, "OK ");

	DL_FOREACH(irccd->servers, s) {
		fprintf(fp, "%s", s->name);

		if (s->next)
			fputc(' ', fp);
	}

	fclose(fp);
	peer_push(p, "%s", out);

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
 * SERVER-RECONNECT [server]
 */
static int
cmd_server_reconnect(struct peer *p, char *line)
{
	const char *args;
	struct irc_server *s;

	if (parse(line, &args, 1) == 1) {
		if (!(s = require_server(p, args)))
			return 0;

		irc_server_reconnect(s);
	} else
		DL_FOREACH(irccd->servers, s)
			irc_server_reconnect(s);

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
	{ "HOOK-ADD",           cmd_hook_add            },
	{ "HOOK-LIST",          cmd_hook_list           },
	{ "HOOK-REMOVE",        cmd_hook_remove         },
	{ "PLUGIN-CONFIG",      cmd_plugin_config       },
	{ "PLUGIN-INFO",        cmd_plugin_info         },
	{ "PLUGIN-LIST",        cmd_plugin_list         },
	{ "PLUGIN-LOAD",        cmd_plugin_load         },
	{ "PLUGIN-PATH",        cmd_plugin_path         },
	{ "PLUGIN-RELOAD",      cmd_plugin_reload       },
	{ "PLUGIN-TEMPLATE",    cmd_plugin_template     },
	{ "PLUGIN-UNLOAD",      cmd_plugin_unload       },
	{ "RULE-ADD",           cmd_rule_add            },
	{ "RULE-EDIT",          cmd_rule_edit           },
	{ "RULE-LIST",          cmd_rule_list           },
	{ "RULE-MOVE",          cmd_rule_move           },
	{ "RULE-REMOVE",        cmd_rule_remove         },
	{ "SERVER-CONNECT",     cmd_server_connect      },
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
	{ "SERVER-RECONNECT",   cmd_server_reconnect    },
	{ "SERVER-TOPIC",       cmd_server_topic        },
	{ "WATCH",              cmd_watch               }
};

static int
cmp_cmd(const void *key, const void *data)
{
	return strcmp(key, ((const struct cmd *)data)->name);
}

static const struct cmd *
find(const char *line)
{
	char cmd[32] = {0};

	/* Extract the initial part of the line. */
	sscanf(line, "%31s", cmd);

	return bsearch(cmd, cmds, IRC_UTIL_SIZE(cmds),
	    sizeof (cmds[0]), cmp_cmd);
}

static void
invoke(struct peer *p, char *line)
{
	const struct cmd *c = find(line);
	int er;

	if (!c)
		peer_push(p, "command not found");
	else if ((er = c->call(p, line)) != 0)
		peer_push(p, "%s", strerror(er));
}

static void
dispatch(struct peer *p)
{
	char *pos;
	size_t length;

	while ((pos = memchr(p->in, '\n', p->insz))) {
		/* Turn end of the string at delimiter. */
		*pos = 0;
		length = pos - p->in;

		if (length > 0)
			invoke(p, p->in);

		memmove(p->in, pos + 1, sizeof (p->in) - (length + 1));
		p->insz -= length;
	}
}

static int
input(struct peer *p)
{
	ssize_t nr;
	size_t cap;

	cap = sizeof (p->in) - p->insz;

	if (cap == 0) {
		irc_log_warn("transport: peer input full");
		return -1;
	}

	while ((nr = recv(p->fd, &p->in[p->insz], cap, MSG_NOSIGNAL)) < 0 && errno == EINTR)
		continue;

	if (nr < 0 && errno != EAGAIN && errno != EWOULDBLOCK)
		return -1;
	if (nr == 0) {
		irc_log_info("transport: client disconnect");
		return -1;
	}

	dispatch(p);

	return 0;
}

static int
output(struct peer *p)
{
	ssize_t ns;

	while ((ns = send(p->fd, p->out, p->outsz, MSG_NOSIGNAL)) < 0 && errno == EINTR)
		continue;

	if (ns < 0 && errno != EAGAIN && errno != EWOULDBLOCK)
		return -1;

	if ((size_t)ns >= p->outsz) {
		memset(p->out, 0, sizeof (p->out));
		p->outsz = 0;
	} else {
		memmove(p->out, &p->out[ns], sizeof (p->out) - ns);
		p->outsz -= ns;
	}

	return 0;
}

static void
io_cb(struct ev_loop *loop, struct ev_io *self, int revents)
{
	struct peer *peer;
	int rc = 0;

	peer = IRC_CONTAINER_OF(self, struct peer, io_fd);

	if (revents & EV_READ)
		rc = input(peer);
	if (rc == 0 && (revents & EV_WRITE))
		rc = output(peer);

	if (rc < 0) {
		ev_io_stop(loop, &peer->io_fd);
		peer->on_close(peer);
	}
}

struct peer *
peer_new(int fd)
{
	struct peer *p;

	p = irc_util_calloc(1, sizeof (*p));
	p->fd = fd;

	ev_io_init(&p->io_fd, io_cb, fd, EV_READ);
	ev_io_start(irc_bot_loop(), &p->io_fd);

	return p;
}

int
peer_push(struct peer *p, const char *fmt, ...)
{
	assert(p);
	assert(fmt);

	char buf[IRC_BUF_LEN] = {};
	va_list ap;
	size_t len;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof (buf) - 1, fmt, ap);
	va_end(ap);

	len = strlen(buf);

	/* We add a '\n' to the end of the message. */
	if (len + 1 > sizeof (p->out) - p->outsz)
		return -1;

	/* Replace NUL terminator by \n' */
	buf[len++] = '\n';
	memcpy(&p->out[p->outsz], buf, len);
	p->outsz += len;

	ev_io_stop(irc_bot_loop(), &p->io_fd);
	ev_io_modify(&p->io_fd, EV_READ | EV_WRITE);
	ev_io_start(irc_bot_loop(), &p->io_fd);

	return 0;
}

void
peer_free(struct peer *p)
{
	assert(p);

	ev_io_stop(irc_bot_loop(), &p->io_fd);

	if (p->fd != -1) {
		close(p->fd);
		p->fd = -1;
	}

	free(p);
}
