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

#include <irccd/hook.h>
#include <irccd/irccd.h>
#include <irccd/log.h>
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

union token {
	enum token_type type;

	struct {
		enum token_type type;
		char *at;
	} str;
};

struct conf {
	const char *path;
	char *text;
	char *off;
	size_t line;
	size_t column;
	struct nce_coro lex;
	struct nce_coro parser;
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
conf_debug(const struct conf *, const char *origin, const char *fmt, ...)
{
#if 1
	va_list ap;

	printf("[%-10s] ", origin);
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
	printf("\n");
#endif
}

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
conf_lex_yield(struct conf *conf, const union token *token)
{
	//conf_debug(conf, "lex", "yielding");
	nce_coro_push(&conf->parser, token, sizeof (*token));
	//conf_debug(conf, "lex", "resumed");
}

/*
 * Similar to conf_lex_yield but for non-string tokens but in contrast to it it
 * also consume the spaces after the token for convenience.
 */
static inline void
conf_lex_yield0(struct conf *conf, enum token_type type)
{
	conf_debug(conf, "lex", "token %c", type);
	conf_lex_yield(conf, &(const union token) {
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

	conf_lex_yield(conf, &(const union token) {
		.str = {
			.type = TOKEN_STRING,
			.at   = start
		}
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

	//conf_debug(conf, "lex", "simple string");

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

	//conf_debug(conf, "lex", "simple string content '%s'", start);
	conf_lex_yield(conf, &(const union token) {
		.str = {
			.type = TOKEN_STRING,
			.at   = start
		}
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
	conf_debug(conf, "lex", "comment");
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
	conf_debug(conf, "lex", "start");

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

	conf_debug(conf, "lex", "end");
	conf_lex_yield0(conf, TOKEN_EOF);
}

/*
 * Pull next incoming token and return its type.
 */
static inline enum token_type
conf_next(struct conf *conf, union token *token)
{
	//conf_debug(conf, "parser", "pulling");
	nce_coro_pull(&conf->parser, token, sizeof (*token));
	//conf_debug(conf, "parser", "pulled 0x%0x", token->type);

	if (token->type == TOKEN_EOF)
		nce_coro_off();

	return token->type;
}

/*
 * Get the next token and expect to be a string.
 */
static char *
conf_string(struct conf *conf)
{
	union token token = {};

	if (conf_next(conf, &token) != TOKEN_STRING)
		conf_fatal(conf, "string expected");

	return token.str.at;
}

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
 * Similar to conf_string but dynamically allocate the string token.
 */
static inline char *
conf_string_new(struct conf *conf)
{
	return irc_util_strdup(conf_string(conf));
}

/*
 * Get the next (or current) string and expect it to equal `keyword`.
 *
 * If value is provided, it is compared to it. Otherwise the function calls
 * conf_string to retrieve the next one.
 */
static void
conf_keyword(struct conf *conf, const char *value, const char *keyword)
{
	union token token = {};

	if (!value) {
		/* TODO: token description. */
		if (conf_next(conf, &token) != TOKEN_STRING)
			conf_fatal(conf, "unexpected token, expecting keyword '%s'", keyword);

		value = token.str.at;
	}

	if (strcmp(value, keyword) != 0)
		conf_fatal(conf, "unexpected string '%s', expected keyword '%s'", value, keyword);
}

/*
 * Expect a beginning block '{'.
 */
static inline void
conf_begin(struct conf *conf)
{
	union token token = {};

	if (conf_next(conf, &token) != TOKEN_BLK_BEGIN)
		conf_fatal(conf, "expected '%c' block start", TOKEN_BLK_BEGIN);
}

#if 0

/*
 * Expect a terminating block '}'.
 */
static inline void
conf_end(struct conf *conf)
{
	union token token = {};

	if (conf_next(conf, &token) != TOKEN_BLK_END)
		conf_fatal(conf, "expected '%c' block end", TOKEN_BLK_END);
}

#endif

#if 0
static inline const char *
conf_list(struct conf *conf)
{
	union token token = {};

	for (;;) {
		switch (conf_next(conf, &token)) {
		case TOKEN_STRING:
			return token.str.at;
		case TOKEN_COMMA:
			break;
		default:
			conf_fatal(conf, "unexpected token '%c' in list",
			break;
		}
	}
	while (conf_next(conf, &token) == TOKEN_COMMA)
		continue;
}
#endif

/*
 * Logging section.
 *
 * log[s] [verbose] [template fmt] to (console|syslog|file path)
 */
static void
conf_parse_log(struct conf *conf)
{
	const char *token;

	token = conf_string(conf);

	if (CONF_EQ(token, "verbose")) {
		irc_log_set_verbose(1);
		token = conf_string(conf);
	} else if (CONF_EQ(token, "quiet"))
		token = conf_string(conf);

	if (CONF_EQ(token, "template")) {
		token = conf_string(conf);
		conf_debug(conf, "log", "using template format '%s'", token);
		irc_log_set_template(token);
		token = conf_string(conf);
	}

	/* Now 'to' keyword is to be expected. */
	conf_keyword(conf, token, "to");

	/* And now destination. */
	token = conf_string(conf);

	if (CONF_EQ(token, "console")) {
		conf_debug(conf, "log", "log into console");
		irc_log_to_console();
	} else if (CONF_EQ(token, "syslog")) {
		conf_debug(conf, "log", "log into syslog");
		irc_log_to_syslog();
	} else if (CONF_EQ(token, "file")) {
		token = conf_string(conf);
		conf_debug(conf, "log", "log into file '%s'", token);
		irc_log_to_file(token);
	}
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

/*
 * Transport section.
 *
 * transport [with uid value gid value] to path
 */
static void
conf_parse_transport(struct conf *conf)
{
	long long uid, gid;
	const char *token;
	int rc;

	token = conf_string(conf);

	if (CONF_EQ(token, "with")) {
		conf_keyword(conf, NULL, "uid");
		uid = conf_resolve_uid(conf, conf_string(conf));

		conf_keyword(conf, NULL, "gid");
		gid = conf_resolve_gid(conf, conf_string(conf));
	} else
		uid = gid = -1;

	conf_keyword(conf, token, "to");
	token = conf_string(conf);

	if (uid != -1 && gid != -1) {
		conf_debug(conf, "transport", "binding on '%s' with %lld:%lld", token, uid, gid);
		rc = transport_bindp(token, uid, gid);
	} else {
		conf_debug(conf, "transport", "binding on '%s'", token);
		rc = transport_bind(token);
	}

	if (rc < 0)
		irc_util_die("abort: %s: %s\n", token, strerror(-rc));
}

static void
conf_parse_hook(struct conf *conf)
{
	char *name, *path;

	name = conf_string_new(conf);
	conf_keyword(conf, NULL, "to");
	path = conf_string_new(conf);

	irc_bot_hook_add(irc_hook_new(name, path));
	conf_debug(conf, "hook", "added '%s' -> '%s'", name, path);

	free(name);
	free(path);
}














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



/*
 * In server.
 *
 * join "channel"
 * join with password "secret" "channel"
 */
static void
conf_parse_server_join(struct conf *conf, struct irc_server *server)
{
	const char *token;
	char *channel, *password;

	token = conf_string(conf);

	if (CONF_EQ(token, "with")) {
		conf_keyword(conf, NULL, "password");
		channel = conf_string_new(conf);
		password = conf_string_new(conf);
	} else {
		channel = irc_util_strdup(token);
		password = NULL;
	}

	irc_server_join(server, channel, password);
}

static void
conf_parse_server_ctcp(struct conf *conf, struct irc_server *server)
{
	union token token = {};
	char *key, *value;

	conf_begin(conf);

	while (conf_next(conf, &token) == TOKEN_STRING) {
		key = irc_util_strdup(token.str.at);
		value = conf_string_new(conf);
		irc_server_set_ctcp(server, key, value);
		free(key);
		free(value);
	}

	if (token.type != TOKEN_BLK_END)
		conf_fatal(conf, "unterminated ctcp block");
}

static void
conf_parse_server(struct conf *conf)
{
	struct irc_server *server;
	union token token;
	const char *str;

	server = irc_server_new(conf_string(conf));
	conf_begin(conf);

	while (conf_next(conf, &token) == TOKEN_STRING) {
		str = token.str.at;

		if (CONF_EQ(str, "hostname"))
			conf_parse_server_hostname(conf, server);
		else if (CONF_EQ(str, "port"))
			conf_parse_server_port(conf, server);
		else if (CONF_EQ(str, "prefix"))
			conf_parse_server_prefix(conf, server);
		else if (CONF_EQ(str, "ident"))
			conf_parse_server_ident(conf, server);
		else if (CONF_EQ(str, "password"))
			conf_parse_server_password(conf, server);
		else if (CONF_EQ(str, "join"))
			conf_parse_server_join(conf, server);
		else if (CONF_EQ(str, "ctcp"))
			conf_parse_server_ctcp(conf, server);
	}

	if (token.type != TOKEN_BLK_END)
		conf_fatal(conf, "unterminated server section");

	if (!server->hostname)
		conf_fatal(conf, "no hostname set");
	if (!server->port)
		conf_fatal(conf, "no port set");
	if (!server->nickname || !server->username || !server->realname)
		conf_fatal(conf, "ni ident set");

	irc_bot_server_add(server);
}

static void
conf_parser_entry(struct nce_coro *self)
{
	struct conf *conf;
	const char *topic;

	conf = CONF(self, parser);

	conf_debug(conf, "parser", "start");

	while ((topic = conf_string(conf))) {
		if (CONF_EQ(topic, "logs") || CONF_EQ(topic, "log"))
			conf_parse_log(conf);
		else if (CONF_EQ(topic, "transport"))
			conf_parse_transport(conf);
		else if (CONF_EQ(topic, "hook"))
			conf_parse_hook(conf);
		else if (CONF_EQ(topic, "server"))
			conf_parse_server(conf);
	}

	conf_debug(conf, "parser", "end");
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
		if (!nce_coro_resumable(&conf.parser))
			break;
		nce_coro_resume(&conf.parser);

		if (!nce_coro_resumable(&conf.lex))
			break;

		nce_coro_resume(&conf.lex);
	}

	nce_coro_destroy(&conf.lex);
	nce_coro_destroy(&conf.parser);

	free(conf.text);
}
