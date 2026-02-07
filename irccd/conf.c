/*
 * conf.c -- configuration file parser
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

#include <sys/stat.h>
#include <assert.h>
#include <ctype.h>
#include <err.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <nce/nce.h>

#include <utlist.h>

#include <irccd/hook.h>
#include <irccd/irccd.h>
#include <irccd/log.h>
#include <irccd/plugin.h>
#include <irccd/rule.h>
#include <irccd/server.h>
#include <irccd/util.h>

#include "conf.h"
#include "transport.h"

#define CONF(Ptr, Field) \
        (IRC_UTIL_CONTAINER_OF(Ptr, struct conf, Field))

#define CONF_EQ(A, B) \
        (strcmp((A), (B)) == 0)

enum token_type {
	TOKEN_BLK_BEGIN = '{',
	TOKEN_BLK_END = '}',
	TOKEN_COMMA = ',',
	TOKEN_COMMENT = '#',
	TOKEN_EOF = '\0',
	TOKEN_STRING = '"'
};

struct token {
	enum token_type type;
	char *data;
};

enum log_type {
	LOG_TYPE_CONSOLE,
	LOG_TYPE_FILE,
	LOG_TYPE_SYSLOG
};

struct conf {
	const char *path;
	char *text;
	char *off;
	size_t line;
	size_t column;
	struct nce_coro lex;
	struct nce_coro parser;

	/*
	 * Data types are loaded but not registered yet because the user
	 * configuration can be defined in any order.
	 */
	struct irc_server *servers;
	struct irc_hook *hooks;
	struct irc_plugin *plugins;
	struct irc_rule *rules;

	long long tpt_uid;
	long long tpt_gid;
	char *tpt;

	enum log_type log_type;
	int log_level;
	char *log_template;
	char *log_file;
};

IRC_ATTR_PRINTF(2, 3)
_Noreturn static inline void
conf_fatal(struct conf *conf, const char *fmt, ...)
{
	va_list ap;

	fprintf(stderr, "%s:%zu:%zu: ", conf->path, conf->line, conf->column);

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	fprintf(stderr, "\n");
	exit(1);
}


IRC_ATTR_PRINTF(3, 4)
static inline void
conf_debug(const struct conf *conf, const char *origin, const char *fmt, ...)
{
#ifdef CONF_DEBUG
	va_list ap;

	printf("[%-10s] ", origin);
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
	printf("\n");
#else
	(void)conf;
	(void)origin;
	(void)fmt;
#endif
}

static long long
conf_resolve_uid(struct conf *conf, const char *value)
{
	const struct passwd *pwd;
	long long uid;

	if (irc_util_stoi(value, &uid) == 0)
		return uid;

	if (!(pwd = getpwnam(value)))
		conf_fatal(conf, "invalid uid: %s", value);

	return pwd->pw_uid;
}

static long long
conf_resolve_gid(struct conf *conf, const char *value)
{
	const struct group *grp;
	long long gid;

	if (irc_util_stoi(value, &gid) == 0)
		return gid;

	if (!(grp = getgrnam(value)))
		conf_fatal(conf, "invalid gid: %s", value);

	return grp->gr_gid;
}

/* {{{ lex */

/*
 * Advance by one byte, adjusting conf->line and/or conf->column depending on
 * next character.
 */
static inline void
conf_lex_advance(struct conf *conf)
{
	assert(*conf->off);

	if (*conf->off++ == '\n') {
		conf->line += 1;
		conf->column = 1;
	} else
		conf->column += 1;
}

/*
 * Ignore everything considered as blanks (spaces, tabs, CRLF, etc).
 */
static inline void
conf_lex_bskip(struct conf *conf)
{
	while (*conf->off && isspace((unsigned char)*conf->off))
		conf_lex_advance(conf);
}

/*
 * Yield the given token.
 */
static inline void
conf_lex_yield(struct conf *conf, const struct token *token)
{
	nce_coro_push(&conf->parser, token, sizeof (*token));
}

/*
 * Similar to conf_lex_yield but for non-string tokens but in contrast to it it
 * also consume the spaces after the token for convenience.
 */
static inline void
conf_lex_yield0(struct conf *conf, enum token_type type)
{
	conf_debug(conf, "lex", "token %c", type);
	conf_lex_yield(conf, &(const struct token) {
		.type = type
	});
	conf_lex_bskip(conf);
}

/*
 * Analyze a quoted string.
 */
static void
conf_lex_qstring(struct conf *conf)
{
	char *start;

	conf_debug(conf, "lex", "quoted string");

	/* Save current pointer location. */
	start = ++conf->off;

	while (*conf->off && *conf->off != TOKEN_STRING)
		conf_lex_advance(conf);

	if (*conf->off != TOKEN_STRING)
		conf_fatal(conf, "unterminated string");

	/* Replace quote to '\0' to terminate the string. */
	*conf->off = '\0';

	/* Advance manually, we know that there are no spaces here. */
	conf->column += 1;
	conf->off += 1;

	conf_lex_yield(conf, &(const struct token) {
		.type = TOKEN_STRING,
		.data = start
	});

	/* Skip possible residual spaces. */
	conf_lex_bskip(conf);
}

static inline int
conf_lex_istoken(const struct conf *conf)
{
	return *conf->off == TOKEN_BLK_BEGIN ||
	       *conf->off == TOKEN_BLK_END   ||
	       *conf->off == TOKEN_COMMA     ||
	       *conf->off == TOKEN_COMMENT   ||
	       *conf->off == TOKEN_STRING;
}

static void
conf_lex_string(struct conf *conf)
{
	char *start, save;

	/* Save current pointer location. */
	start = conf->off;

	/* String ends at the next blank or token identifier. */
	while (*conf->off && !isspace(*conf->off) && !conf_lex_istoken(conf))
		conf_lex_advance(conf);

	/*
	 * We edit temporarily the string by terminating at the current offset
	 * and put the character back once resumed.
	 */
	save = *conf->off;
	*conf->off = '\0';

	conf_lex_yield(conf, &(const struct token) {
		.type = TOKEN_STRING,
		.data = start
	});

	*conf->off = save;

	/* Skip possible residual spaces. */
	conf_lex_bskip(conf);
}

/*
 * Advance offset until the next line.
 */
static inline void
conf_lex_comment(struct conf *conf)
{
	conf_lex_advance(conf);

	while (*conf->off && *conf->off != '\n')
		conf_lex_advance(conf);

	conf_lex_bskip(conf);
}

static void
conf_lex_entry(struct nce_coro *self)
{
	enum token_type token;
	struct conf *conf;

	conf = CONF(self, lex);

	while (*conf->off) {
		switch (*conf->off) {
		case TOKEN_BLK_BEGIN:
		case TOKEN_BLK_END:
		case TOKEN_COMMA:
			token = *conf->off;
			conf_lex_advance(conf);
			conf_lex_yield0(conf, token);
			break;
		case TOKEN_COMMENT:
			conf_lex_comment(conf);
			break;
		case TOKEN_STRING:
			/* Quoted string has different semantics. */
			conf_lex_qstring(conf);
			break;
		default:
			/* Now we expect this last case as a beginning of a string */
			assert(!isspace(*conf->off));
			conf_lex_string(conf);
			break;
		}
	}

	conf_lex_yield0(conf, TOKEN_EOF);
}

/* }}} */

/* {{{ parse */

/*
 * Pull next incoming token and return its type.
 *
 * On end of content, this function stops parser coroutine.
 */
static inline enum token_type
conf_next(struct conf *conf, struct token *token)
{
	nce_coro_pull(&conf->parser, token, sizeof (*token));

	if (token->type == TOKEN_EOF)
		nce_coro_off();

	return token->type;
}

/*
 * Pull next incoming typed token or keep it in the queue if doesn't match.
 */
static inline int
conf_next_is(struct conf *conf, struct token *token, enum token_type type)
{
	nce_coro_pull(&conf->parser, token, sizeof (*token));

	if (token->type != type) {
		nce_coro_queue(&conf->parser, token, sizeof (*token));
		return 0;
	}

	return 1;
}

/*
 * Get next token and if it's a string that equals `what` it is removed from the
 * incoming queue.
 */
static inline int
conf_string_is(struct conf *conf, const char *what)
{
	struct token token = {};
	int rc = 0;

	if (conf_next_is(conf, &token, TOKEN_STRING)) {
		if (CONF_EQ(token.data, what))
			rc = 1;
		else
			nce_coro_queue(&conf->parser, &token, sizeof (token));
	}

	return rc;
}

/*
 * Require next token to be a string.
 */
static char *
conf_string(struct conf *conf)
{
	struct token token = {};

	if (conf_next(conf, &token) != TOKEN_STRING)
		conf_fatal(conf, "string expected");

	return token.data;
}

/*
 * Similar to conf_string but dynamically allocate the string token.
 */
static inline char *
conf_string_new(struct conf *conf)
{
	return irc_util_strdup(conf_string(conf));
}

/*
 * Require next token to be an integer.
 */
static long long
conf_int(struct conf *conf)
{
	const char *token;
	long long rc;

	token = conf_string(conf);

	if (irc_util_stoi(token, &rc) != 0)
		conf_fatal(conf, "number expected");

	return rc;
}

/*
 * Require the next token to be a string that equals to `keyword`.
 */
static inline void
conf_keyword(struct conf *conf, const char *keyword)
{
	if (!conf_string_is(conf, keyword))
		conf_fatal(conf, "keyword '%s' expected", keyword);
}

/*
 * Require a beginning block '{'.
 */
static inline void
conf_begin(struct conf *conf)
{
	struct token token = {};

	if (conf_next(conf, &token) != TOKEN_BLK_BEGIN)
		conf_fatal(conf, "expected '%c' block start", TOKEN_BLK_BEGIN);
}

/*
 * Returns non-zero if next token is a beginning of block '{' and pull it from
 * the queue, kept otherwise.
 */
static inline int
conf_begin_is(struct conf *conf)
{
	struct token token = {};

	return conf_next_is(conf, &token, TOKEN_BLK_BEGIN);
}

/*
 * Require a terminating block '}'.
 */
static inline void
conf_end(struct conf *conf)
{
	struct token token = {};

	if (conf_next(conf, &token) != TOKEN_BLK_END)
		conf_fatal(conf, "expected '%c' block end", TOKEN_BLK_END);
}

/* }}} */

/* {{{ log(s) */

/*
 * Logging section.
 *
 * log[s] [verbose] [template fmt] to (console|syslog|file path)
 */
static void
conf_parse_log(struct conf *conf)
{
	/* verbose|quiet */
	if (conf_string_is(conf, "verbose"))
		conf->log_level = 1;
	else if (conf_string_is(conf, "quiet"))
		conf->log_level = 0;

	/* template fmt */
	if (conf_string_is(conf, "template"))
		conf->log_template = conf_string_new(conf);

	/* Now 'to' keyword is to be expected. */
	conf_keyword(conf, "to");

	/* console|syslog|file path */
	if (conf_string_is(conf, "console")) {
		conf->log_type = LOG_TYPE_CONSOLE;
	} else if (conf_string_is(conf, "syslog")) {
		conf->log_type = LOG_TYPE_SYSLOG;
	} else if (conf_string_is(conf, "file")) {
		conf->log_type = LOG_TYPE_FILE;
		conf->log_file = conf_string_new(conf);
	} else {
		conf_fatal(conf, "invalid log sink");
	}
}

/* }}} */

/* {{{ transport */

/*
 * Transport section.
 *
 * transport [with uid value gid value] to path
 */
static void
conf_parse_transport(struct conf *conf)
{
	if (conf->tpt)
		conf_fatal(conf, "transport already defined");

	if (conf_string_is(conf, "with")) {
		conf_keyword(conf, "uid");
		conf->tpt_uid = conf_resolve_uid(conf, conf_string(conf));
		conf_keyword(conf, "gid");
		conf->tpt_gid = conf_resolve_gid(conf, conf_string(conf));
	} else
		conf->tpt_uid = conf->tpt_gid = -1;

	conf_keyword(conf, "to");
	conf->tpt = conf_string_new(conf);
}

/* }}} */

/* {{{ hook */

/*
 * Hook section.
 *
 * hook name to path
 */
static void
conf_parse_hook(struct conf *conf)
{
	struct irc_hook *hook;
	char *name, *path;

	name = conf_string_new(conf);

	LL_FOREACH(conf->hooks, hook)
		if (CONF_EQ(hook->name, name))
			conf_fatal(conf, "hook '%s' already exists", name);

	conf_keyword(conf, "to");
	path = conf_string_new(conf);

	hook = irc_hook_new(name, path);
	LL_APPEND(conf->hooks, hook);
}

/* }}} */

/* {{{ server */

static inline void
conf_parse_server_hostname(struct conf *conf, struct irc_server *server)
{
	irc_server_set_hostname(server, conf_string(conf));
}

static inline void
conf_parse_server_port(struct conf *conf, struct irc_server *server)
{
	long long port;

	port = conf_int(conf);

	if (port <= 0 || port >= UINT16_MAX)
		conf_fatal(conf, "invalid port range '%lld'", port);

	irc_server_set_port(server, port);
}

static inline void
conf_parse_server_prefix(struct conf *conf, struct irc_server *server)
{
	irc_server_set_prefix(server, conf_string(conf));
}

static inline void
conf_parse_server_ident(struct conf *conf, struct irc_server *server)
{
	irc_server_set_nickname(server, conf_string(conf));
	irc_server_set_username(server, conf_string(conf));
	irc_server_set_realname(server, conf_string(conf));
}

static inline void
conf_parse_server_password(struct conf *conf, struct irc_server *server)
{
	irc_server_set_password(server, conf_string(conf));
}

static void
conf_parse_server_join(struct conf *conf, struct irc_server *server)
{
	char *channel, *password;

	if (conf_string_is(conf, "with")) {
		conf_keyword(conf, "password");
		channel = conf_string_new(conf);
		password = conf_string_new(conf);
	} else {
		channel = conf_string_new(conf);
		password = NULL;
	}

	irc_server_join(server, channel, password);
}

static void
conf_parse_server_ctcp(struct conf *conf, struct irc_server *server)
{
	struct token token = {};
	char *key, *value;

	conf_begin(conf);

	while (conf_next_is(conf, &token, TOKEN_STRING)) {
		key = irc_util_strdup(token.data);
		value = conf_string_new(conf);
		irc_server_set_ctcp(server, key, value);
		free(key);
		free(value);
	}

	conf_end(conf);
}

static inline void
conf_parse_server_ssl(struct conf *conf, struct irc_server *server)
{
	conf_debug(conf, "server", "using SSL");
	irc_server_set_flags(server, server->flags | IRC_SERVER_FLAGS_SSL);
}

static void
conf_parse_server_options(struct conf *conf, struct irc_server *server)
{
	struct token token;

	while (conf_next(conf, &token) == TOKEN_STRING) {
		if (CONF_EQ(token.data, "AUTO-REJOIN")) {
			conf_debug(conf, "server", "set 'auto-rejoin'");
			irc_server_set_flags(server, server->flags | IRC_SERVER_FLAGS_AUTO_REJOIN);
		} else if (CONF_EQ(token.data, "JOIN-INVITE")) {
			conf_debug(conf, "server", "set 'join-invite'");
			irc_server_set_flags(server, server->flags | IRC_SERVER_FLAGS_JOIN_INVITE);
		} else
			conf_fatal(conf, "invalid server option '%s'", token.data);
	}

	if (token.type != TOKEN_BLK_END)
		conf_fatal(conf, "unterminated server options list");
}

static void
conf_parse_server(struct conf *conf)
{
	struct irc_server *server;
	const char *name;
	struct token token;

	name = conf_string(conf);

	LL_FOREACH(conf->servers, server)
		if (CONF_EQ(server->name, name))
			conf_fatal(conf, "server '%s' already exists", name);

	server = irc_server_new(name);
	conf_begin(conf);

	while (conf_next_is(conf, &token, TOKEN_STRING)) {
		if (CONF_EQ(token.data, "hostname"))
			conf_parse_server_hostname(conf, server);
		else if (CONF_EQ(token.data, "port"))
			conf_parse_server_port(conf, server);
		else if (CONF_EQ(token.data, "prefix"))
			conf_parse_server_prefix(conf, server);
		else if (CONF_EQ(token.data, "ident"))
			conf_parse_server_ident(conf, server);
		else if (CONF_EQ(token.data, "password"))
			conf_parse_server_password(conf, server);
		else if (CONF_EQ(token.data, "join"))
			conf_parse_server_join(conf, server);
		else if (CONF_EQ(token.data, "ctcp"))
			conf_parse_server_ctcp(conf, server);
		else if (CONF_EQ(token.data, "ssl"))
			conf_parse_server_ssl(conf, server);
		else if (CONF_EQ(token.data, "options"))
			conf_parse_server_options(conf, server);
	}

	conf_end(conf);

	if (!server->hostname)
		conf_fatal(conf, "no hostname set");
	if (!server->port)
		conf_fatal(conf, "no port set");
	if (!server->nickname || !server->username || !server->realname)
		conf_fatal(conf, "no ident set");

	LL_APPEND(conf->servers, server);
}

/* }}} */

/* {{{ rule */

static void
conf_parse_rule_criteria(struct conf *conf,
                         struct irc_rule *rule,
                         const char *criteria)
{
	void (*adder)(struct irc_rule *, const char *) = NULL;
	struct token token;

	if (CONF_EQ(criteria, "servers"))
		adder = irc_rule_add_server;
	else if (CONF_EQ(criteria, "channels"))
		adder = irc_rule_add_channel;
	else if (CONF_EQ(criteria, "origins"))
		adder = irc_rule_add_origin;
	else if (CONF_EQ(criteria, "plugins"))
		adder = irc_rule_add_plugin;
	else if (CONF_EQ(criteria, "events"))
		adder = irc_rule_add_event;
	else
		conf_fatal(conf, "invalid rule criteria '%s'", criteria);

	conf_begin(conf);

	while (conf_next(conf, &token) == TOKEN_STRING) {
		conf_debug(conf, "rule", "add '%s'", token.data);
		adder(rule, token.data);
	}

	if (token.type != TOKEN_BLK_END)
		conf_fatal(conf, "unterminated criteria list");
}

static void
conf_parse_rule(struct conf *conf)
{
	enum irc_rule_action action;
	struct irc_rule *rule;
	struct token token;

	if (conf_string_is(conf, "accept")) {
		conf_debug(conf, "rule", "type accept");
		action = IRC_RULE_ACCEPT;
	} else if (conf_string_is(conf, "drop")) {
		conf_debug(conf, "rule", "type drop");
		action = IRC_RULE_DROP;
	} else
		conf_fatal(conf, "invalid action");

	rule = irc_rule_new(action);

	if (conf_next_is(conf, &token, TOKEN_BLK_BEGIN)) {
		while (conf_next(conf, &token) == TOKEN_STRING) {
			conf_debug(conf, "rule", "parsing '%s' rule criteria", token.data);
			conf_parse_rule_criteria(conf, rule, token.data);
		}

		if (token.type != TOKEN_BLK_END)
			conf_fatal(conf, "unterminated rule block");
	}

	DL_APPEND(conf->rules, rule);
}

/* }}} */

/* {{{ plugin */

static void
conf_parse_plugin_options(struct conf *conf,
                          struct irc_plugin *plg,
                          const char *what)
{
	void (*setter)(struct irc_plugin *, const char *, const char *) = NULL;
	struct token token;
	char *key, *value;

	if (CONF_EQ(what, "config"))
		setter = irc_plugin_set_option;
	else if (CONF_EQ(what, "path") || CONF_EQ(what, "paths"))
		setter = irc_plugin_set_path;
	else if (CONF_EQ(what, "template"))
		setter = irc_plugin_set_template;
	else
		conf_fatal(conf, "invalid directive '%s' in plugin block", what);

	conf_begin(conf);

	while (conf_next_is(conf, &token, TOKEN_STRING)) {
		key = irc_util_strdup(token.data);
		value = conf_string_new(conf);

		conf_debug(conf, "plugin", "%s -> %s", key, value);
		setter(plg, key, value);

		free(key);
		free(value);
	}

	conf_end(conf);
}

/*
 * Plugin section
 *
 * plugin name [to path] [{
 *     config {
 *         key1 value1
 *         ...
 *     }
 *     template {
 *         key1 value1
 *         ...
 *     }
 *     path {
 *         key1 value1
 *         ...
 *     }
 * }
 */
static void
conf_parse_plugin(struct conf *conf)
{
	struct irc_plugin *plg;
	char *location = NULL;
	char *name = NULL;
	struct token token;

	name = conf_string_new(conf);

	LL_FOREACH(conf->plugins, plg)
		if (CONF_EQ(plg->name, name))
			conf_fatal(conf, "plugin '%s' already exists", name);

	/* Either `to path` should follow or the option block */
	if (conf_string_is(conf, "to"))
		location = conf_string_new(conf);

	if (irc_bot_plugin_get(name))
		conf_fatal(conf, "plugin '%s' already loaded", name);
	if (!(plg = irc_bot_plugin_search(name, location)))
		conf_fatal(conf, "plugin '%s' not found", name);

	if (conf_begin_is(conf)) {
		while (conf_next_is(conf, &token, TOKEN_STRING)) {
			conf_debug(conf, "plugin", "parsing '%s'", token.data);
			conf_parse_plugin_options(conf, plg, token.data);
		}

		conf_end(conf);
	}

	free(name);
	free(location);

	LL_APPEND(conf->plugins, plg);
}

/* }}} */

static void
conf_parser_entry(struct nce_coro *self)
{
	struct conf *conf;
	const char *topic;

	conf = CONF(self, parser);
	conf_debug(conf, "parser", "start");

	while ((topic = conf_string(conf))) {
		if (CONF_EQ(topic, "logs")) {
			fprintf(stderr, "section 'logs' has been renamed to 'log'\n");
			conf_parse_log(conf);
		} else if (CONF_EQ(topic, "log")) {
			conf_parse_log(conf);
		} else if (CONF_EQ(topic, "transport")) {
			conf_parse_transport(conf);
		} else if (CONF_EQ(topic, "hook")) {
			conf_parse_hook(conf);
		} else if (CONF_EQ(topic, "server")) {
			conf_parse_server(conf);
		} else if (CONF_EQ(topic, "rule")) {
			conf_parse_rule(conf);
		} else if (CONF_EQ(topic, "plugin")) {
			conf_parse_plugin(conf);
		}
	}

	conf_debug(conf, "parser", "end");
}

static void
conf_apply_log(const struct conf *conf)
{
	switch (conf->log_type) {
	case LOG_TYPE_CONSOLE:
		conf_debug(conf, "log", "verbose (%d) into console", conf->log_level);
		irc_log_to_console();
		break;
	case LOG_TYPE_FILE:
		conf_debug(conf, "log", "verbose (%d) into file %s", conf->log_level, conf->log_file);
		irc_log_to_file(conf->log_file);
		break;
	case LOG_TYPE_SYSLOG:
		conf_debug(conf, "log", "verbose (%d) into console", conf->log_level);
		irc_log_to_syslog();
		break;
	}

	irc_log_set_verbose(conf->log_level);
	irc_log_set_template(conf->log_template);
}

static void
conf_apply_rules(struct conf *conf)
{
	struct irc_rule *rule, *tmp;

	DL_FOREACH_SAFE(conf->rules, rule, tmp) {
		DL_DELETE(conf->rules, rule);
		irc_bot_rule_insert(rule, -1);
	}
}

static void
conf_apply_servers(struct conf *conf)
{
	struct irc_server *server, *tmp;

	LL_FOREACH_SAFE(conf->servers, server, tmp) {
		LL_DELETE(conf->servers, server);
		irc_bot_server_add(server);
	}
}

static void
conf_apply_plugins(struct conf *conf)
{
	struct irc_plugin *plugin, *tmp;

	LL_FOREACH_SAFE(conf->plugins, plugin, tmp) {
		LL_DELETE(conf->plugins, plugin);
		irc_bot_plugin_add(plugin);
	}
}

static void
conf_apply_hooks(struct conf *conf)
{
	struct irc_hook *hook, *tmp;

	LL_FOREACH_SAFE(conf->hooks, hook, tmp) {
		LL_DELETE(conf->hooks, hook);
		irc_log_info("hook %s: registered", hook->name);
		irc_bot_hook_add(hook);
	}
}

static void
conf_apply_transport(struct conf *conf)
{
	int rc;

	if (!conf->tpt)
		return;

	if (conf->tpt_uid != -1 && conf->tpt_gid != -1)
		conf_debug(conf, "transport", "binding on '%s' with %lld:%lld",
		    conf->tpt, conf->tpt_uid, conf->tpt_gid);
	else
		conf_debug(conf, "transport", "binding on '%s'", conf->tpt);

	rc = transport_start(conf->tpt, conf->tpt_uid, conf->tpt_gid);

	if (rc < 0)
		irc_util_die("abort: %s: %s\n", conf->tpt, strerror(-rc));
}

void
conf_open(const char *path)
{
	assert(path);

	struct conf conf = {};
	struct stat st;
	ssize_t nr;
	int fd;

	conf.path = path;
	conf.line = 1;
	conf.column = 1;

	if ((fd = open(path, O_RDONLY)) < 0)
		irc_util_die("open: %s", path);
	if (fstat(fd, &st) < 0)
		err(1, "stat: %s", path);

	conf.text = conf.off = calloc(1, st.st_size + 1);

	if ((nr = read(fd, conf.text, st.st_size)) != st.st_size)
		err(1, "read");

	close(fd);

	conf.parser.entry = conf_parser_entry;
	nce_coro_spawn(&conf.parser);

	conf.lex.entry = conf_lex_entry;
	nce_coro_spawn(&conf.lex);

	for (;;) {
		if (nce_coro_resumable(&conf.parser))
			nce_coro_resume(&conf.parser);
		else
			break;

		if (nce_coro_resumable(&conf.lex))
			nce_coro_resume(&conf.lex);
		else
			break;
	}

	nce_coro_destroy(&conf.lex);
	nce_coro_destroy(&conf.parser);

	conf_apply_log(&conf);
	conf_apply_rules(&conf);
	conf_apply_servers(&conf);
	conf_apply_plugins(&conf);
	conf_apply_hooks(&conf);
	conf_apply_transport(&conf);

	free(conf.log_file);
	free(conf.log_template);
	free(conf.tpt);
	free(conf.text);
}
