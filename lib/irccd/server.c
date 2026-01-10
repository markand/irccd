/*
 * server.c -- an IRC server
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

#define _GNU_SOURCE
#include <sys/socket.h>
#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>

#include <ev.h>

#include <utlist.h>

#include <coro/cio.h>
#include <coro/coro.h>
#include <coro/ctimer.h>

#include "config.h"

#if IRCCD_WITH_SSL == 1
#       include <tls.h>
#endif

#include "channel.h"
#include "event.h"
#include "irccd.h"
#include "limits.h"
#include "log.h"
#include "server.h"
#include "util.h"

#define IRC_ARGS_MAX 32         /* maximum number of arguments to parse */
#define IRC_MESSAGE_LEN 512     /* per IRC message length */
#define IRC_BUF_SIZE 65536      /* incoming and outgoing buffers */

#define RECONNECT_DELAY 30.0    /* Seconds to wait before reconnecting. */
#define CONNECT_TIMEOUT 5.0     /* Seconds before marking a server as dead. */
#define PING_TIMEOUT    300.0   /* Seconds after assuming ping timeout. */

#define CONN(Ptr, Field) \
        (IRC_UTIL_CONTAINER_OF(Ptr, struct conn, Field))

/*
 * Private abstraction to the server connection using either plain or SSL
 * transport.
 */
struct conn {
	/* Link to the parent server. */
	struct irc_server *parent;

	/*
	 * Pointer to current endpoint to try and pointer to the original
	 * list.
	 */
	struct addrinfo *ai;
	struct addrinfo *ai_list;

	/*
	 * Input & output buffer and their respective sizes, not NUL
	 * terminated.
	 *
	 * Since conn is allocated on the heap its fine to have a fixed size
	 * array here.
	 */
	char in[IRC_BUF_SIZE];
	size_t insz;
	char out[IRC_BUF_SIZE];
	size_t outsz;

	int fd;
	struct cio_coro io_fd;
	struct ctimer_coro timer;

	/* OpenBSD's nice libtls. */
#if IRCCD_WITH_SSL == 1
	struct tls *tls;
#endif

	enum {
		STATE_RESOLVE,
		STATE_CONNECT,
		STATE_IDENT,
		STATE_READY
	} state;

	/* Transport callbacks */
	ssize_t (*recv)(struct conn *, void *, size_t, int *);
	ssize_t (*send)(struct conn *, const void *, size_t, int *);
};

typedef void (*log_t)(const char *fmt, ...);

static void
irc_server_log(const struct irc_server *server, log_t logger, const char *fmt, ...)
{
	char line[256] = {};
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(line, sizeof (line), fmt, ap);
	va_end(ap);

	logger("server %s: %s", server->name, line);
}

#define DEBUG(...) \
	irc_server_log(server, irc_log_debug, __VA_ARGS__)
#define INFO(...) \
	irc_server_log(server, irc_log_info, __VA_ARGS__)
#define WARN(...) \
	irc_server_log(server, irc_log_warn, __VA_ARGS__)

/* {{{ misc */

/*
 * Tell if the nickname targets the bot itself.
 */
static inline int
server_is_self(const struct irc_server *s, const char *nickname)
{
	return strncasecmp(s->nickname, nickname, strlen(s->nickname)) == 0;
}

/*
 * Parse the IRC prefixes protocol in the form `(abcde)!@#$%`.
 */
static void
server_modes_parse(struct irc_server *server, const char *value)
{
	char modes[16] = {}, syms[16] = {};
	size_t modesz, symsz;

	/* Just make sure in case this line would come twice. */
	server->prefixes  = irc_util_free(server->prefixes);
	server->prefixesz = 0;

	if (sscanf(value, "(%15[^)])%15s", modes, syms) == 2) {
		modesz = strlen(modes);
		symsz  = strlen(syms);

		if (modesz != symsz) {
			WARN("broken support prefix string");
			return;
		}

		server->prefixes  = irc_util_calloc(modesz, sizeof (*server->prefixes));
		server->prefixesz = modesz;

		for (size_t i = 0; i < modesz; ++i) {
			server->prefixes[i].mode   = modes[i];
			server->prefixes[i].symbol = syms[i];
		}
	}
}

/*
 * Find a mode in the server prefixes list. Return the index of the mode.
 */
static inline int
modes_find(struct irc_server *s, int mode)
{
	for (size_t i = 0; i < s->prefixesz; ++i)
		if (s->prefixes[i].mode == mode)
			return i;

	return 0;
}

/* }}} */

/* {{{ msg */

/*
 * IRC message to be parsed.
 *
 * All fields are just pointer over msg.buf, no dynamic allocation involved.
 */
struct msg {
	char *prefix;
	char *cmd;
	char *args[IRC_ARGS_MAX];
	char buf[IRC_MESSAGE_LEN + 1];
};

static inline void
msg_scan(char **line, char **str)
{
	char *p;

	if ((p = strchr(*line, ' ')))
		*p = '\0';

	*str = *line;
	*line = p ? p + 1 : strchr(*line, '\0');
}

static int
msg_parse(struct msg *msg, const char *line, size_t linesz)
{
	char *ptr = msg->buf;
	size_t a;

	memset(msg, 0, sizeof (*msg));
	memcpy(msg->buf, line, linesz);

	/*
	 * IRC message is defined as following:
	 *
	 * [:prefix] command arg1 arg2 [:last-argument]
	 */
	if (*ptr == ':')
		msg_scan((++ptr, &ptr), &msg->prefix);     /* prefix */

	msg_scan(&ptr, &msg->cmd);                         /* command */

	/* And finally arguments. */
	for (a = 0; *ptr && a < IRC_UTIL_SIZE(msg->args); ++a) {
		if (*ptr == ':') {
			msg->args[a] = ptr + 1;
			ptr = strchr(ptr, '\0');
		} else
			msg_scan(&ptr, &msg->args[a]);
	}

	if (a >= IRC_UTIL_SIZE(msg->args))
		return -EMSGSIZE;
	if (msg->cmd == NULL)
		return -EBADMSG;

	return 0;
}

static inline int
msg_is_ctcp(const char *line)
{
	size_t length;

	if (!line)
		return 0;
	if ((length = strlen(line)) < 2)
		return 0;

	return line[0] == 0x1 && line[length - 1] == 0x1;
}

static char *
msg_ctcp(char *line)
{
	/* Skip first \x01. */
	if (*line == '\x01')
		line++;

	/* Remove last \001. */
	line[strcspn(line, "\x01")] = '\0';

	if (strncmp(line, "ACTION ", 7) == 0)
		line += 7;

	return line;
}

/* }}} */

/* {{{ channel */

static struct irc_channel *
server_channels_find(struct irc_server *s, const char *name)
{
	struct irc_channel *ch;

	LL_FOREACH(s->channels, ch)
		if (strcasecmp(ch->name, name) == 0)
			return ch;

	return NULL;
}

static struct irc_channel *
server_channels_add(struct irc_server *s,
                    const char *name,
                    const char *password,
                    enum irc_channel_flags flags)
{
	struct irc_channel *ch;

	if ((ch = server_channels_find(s, name)))
		ch->flags |= flags;
	else {
		ch = irc_channel_new(name, password, flags);
		LL_PREPEND(s->channels, ch);
	}

	return ch;
}

/*
 * Remove the channel from the server.
 *
 * Explicitly allow ch to be NULL for convenience.
 */
static inline void
server_channels_remove(struct irc_server *s, struct irc_channel *ch)
{
	if (ch) {
		LL_DELETE(s->channels, ch);
		irc_channel_free(ch);
	}
}

/*
 * Clear the server channels, meaning the joining state get's reset and
 * nickname list emptied but they stay in the server to be joined later on
 */
static inline void
server_channels_clear(struct irc_server *s)
{
	struct irc_channel *c;

	LL_FOREACH(s->channels, c)
		irc_channel_clear(c);
}

/*
 * Free all channels and clear the linked list.
 */
static inline void
server_channels_free(struct irc_server *s)
{
	struct irc_channel *c, *tmp;

	LL_FOREACH_SAFE(s->channels, c, tmp)
		irc_channel_free(c);

	s->channels = NULL;
}

static void
server_clear(struct irc_server *s)
{
	s->chantypes   = irc_util_free(s->chantypes);
	s->charset     = irc_util_free(s->charset);
	s->casemapping = irc_util_free(s->casemapping);

	s->channel_max  = 0;
	s->nickname_max = 0;
	s->topic_max    = 0;
	s->away_max     = 0;
	s->kick_max     = 0;

	s->prefixes  = irc_util_free(s->prefixes);
	s->prefixesz = 0;

	s->bufwhois.nickname = irc_util_free(s->bufwhois.nickname);
	s->bufwhois.username = irc_util_free(s->bufwhois.username);
	s->bufwhois.realname = irc_util_free(s->bufwhois.realname);
	s->bufwhois.hostname = irc_util_free(s->bufwhois.hostname);
	s->bufwhois.channels = irc_util_free(s->bufwhois.channels);
}

static void
server_free(struct irc_server *s)
{
	irc_server_disconnect(s);

	server_clear(s);
	server_channels_free(s);

	free(s->name);
	free(s->hostname);
	free(s->prefix);
	free(s->nickname);
	free(s->username);
	free(s->realname);
	free(s->password);
	free(s->ctcp_version);
	free(s->ctcp_source);
	free(s->chantypes);
	free(s->charset);
	free(s->casemapping);
	free(s);
}

/* }}} */

/* {{{ handlers */

static void
handle_connect(struct irc_server *server, struct msg *)
{
	struct irc_channel *ch;
	struct irc_event ev = {};

	/* Now join all channels that were requested. */
	LL_FOREACH(server->channels, ch)
		irc_server_join(server, ch->name, ch->password);

	ev.type = IRC_EVENT_CONNECT;
	ev.server = server;

	INFO("connection complete");
	irc_bot_dispatch(&ev);
}

static void
handle_disconnect(struct irc_server *s)
{
	struct irc_event ev = {};

	ev.type = IRC_EVENT_DISCONNECT;
	ev.server = s;

	irc_bot_dispatch(&ev);
}

static void
handle_support(struct irc_server *server, struct msg *msg)
{
	char key[64];
	char value[64];

	for (size_t i = 0; i < IRC_UTIL_SIZE(msg->args) && msg->args[i]; ++i) {
		if (sscanf(msg->args[i], "%63[^=]=%63s", key, value) != 2)
			continue;

		if (strcmp(key, "PREFIX") == 0) {
			server_modes_parse(server, value);
			INFO("prefixes:           %s", value);
		} else if (strcmp(key, "CHANTYPES") == 0) {
			server->chantypes = irc_util_strdupfree(server->chantypes, value);
			INFO("channel types:      %s", value);
		} else if (strcmp(key, "CHANNELLEN") == 0) {
			server->channel_max = atoi(value);
			INFO("channel name limit: %u", server->channel_max);
		} else if (strcmp(key, "NICKLEN") == 0) {
			server->nickname_max = atoi(value);
			INFO("nickname limit:     %u", server->nickname_max);
		} else if (strcmp(key, "TOPICLEN") == 0) {
			server->topic_max = atoi(value);
			INFO("topic limit:        %u", server->topic_max);
		} else if (strcmp(key, "AWAYLEN") == 0) {
			server->away_max = atoi(value);
			INFO("away message limit: %u", server->away_max);
		} else if (strcmp(key, "KICKLEN") == 0) {
			server->kick_max = atoi(value);
			INFO("kick reason limit:  %u", server->kick_max);
		} else if (strcmp(key, "CHARSET") == 0) {
			server->charset = irc_util_strdupfree(server->charset, value);
			INFO("charset:            %s", server->charset);
		} else if (strcmp(key, "CASEMAPPING") == 0) {
			server->casemapping = irc_util_strdupfree(server->casemapping, value);
			INFO("case mapping:       %s", server->casemapping);
		}
	}
}

static void
handle_invite(struct irc_server *s, struct msg *msg)
{
	struct irc_event ev = {};

	ev.type = IRC_EVENT_INVITE;
	ev.server = s;
	ev.invite.origin = irc_util_strdup(msg->prefix);
	ev.invite.channel = irc_util_strdup(msg->args[1]);

	if (s->flags & IRC_SERVER_FLAGS_JOIN_INVITE) {
		irc_server_join(s, ev.invite.channel, NULL);
		irc_log_info("server %s: joining %s on invite", s->name, ev.invite.channel);
	}

	irc_bot_dispatch(&ev);
}

static void
handle_join(struct irc_server *server, struct msg *msg)
{
	struct irc_event ev = {};

	ev.type = IRC_EVENT_JOIN;
	ev.server = server;
	ev.join.origin = irc_util_strdup(msg->prefix);
	ev.join.channel = irc_util_strdup(msg->args[0]);

	server_channels_add(server, ev.join.channel, NULL, IRC_CHANNEL_FLAGS_JOINED);

	if (server_is_self(server, ev.join.origin))
		INFO("joined channel %s", ev.join.channel);

	irc_bot_dispatch(&ev);
}

static void
handle_kick(struct irc_server *server, struct msg *msg)
{
	struct irc_channel *ch;
	struct irc_event ev = {};

	ev.type = IRC_EVENT_KICK;
	ev.server = server;
	ev.kick.origin = irc_util_strdup(msg->prefix);
	ev.kick.channel = irc_util_strdup(msg->args[0]);
	ev.kick.target = irc_util_strdup(msg->args[1]);
	ev.kick.reason = msg->args[2] ? irc_util_strdup(msg->args[2]) : NULL;

	ch = server_channels_add(server, ev.kick.channel, NULL, 1);

	/*
	 * If the bot was kicked itself mark the channel as not joined and
	 * rejoin it automatically if the option is set.
	 */
	if (server_is_self(server, ev.kick.target)) {
		ch->flags &= IRC_CHANNEL_FLAGS_JOINED;
		irc_channel_clear(ch);

		if (server->flags & IRC_SERVER_FLAGS_AUTO_REJOIN) {
			irc_server_join(server, ch->name, ch->password);
			INFO("auto-rejoining %s after kick", ch->name);
		}
	} else
		irc_channel_remove(ch, ev.kick.target);

	irc_bot_dispatch(&ev);
}

static void
handle_mode(struct irc_server *s, struct msg *msg)
{
	const struct irc_channel_user *u;
	int action = 0, mode;
	size_t nelem = 0, argindex = 2;
	struct irc_channel *ch;
	struct irc_event ev = {};

	ev.type = IRC_EVENT_MODE;
	ev.server = s;
	ev.mode.origin = irc_util_strdup(msg->prefix);
	ev.mode.channel = irc_util_strdup(msg->args[0]);
	ev.mode.mode = irc_util_strdup(msg->args[1]);

	/* Create a NULL-sentineled list of arguments. */
	for (size_t i = 2; i < IRC_ARGS_MAX && msg->args[i]; ++i) {
		ev.mode.args = irc_util_reallocarray(ev.mode.args, nelem + 1, sizeof (char *));
		ev.mode.args[nelem++] = irc_util_strdup(msg->args[i]);
	}

	/* Add the NULL sentinel. */
	ev.mode.args = irc_util_reallocarray(ev.mode.args, nelem + 1, sizeof (char *));
	ev.mode.args[nelem] = NULL;

	if (!(ch = server_channels_find(s, ev.mode.channel)))
		goto skip;

	for (const char *p = ev.mode.mode; *p; ++p) {
		/* Determine if we're adding or removing a mode. */
		if (*p == '+' || *p == '-') {
			action = *p;
			continue;
		}

		/* All these mode require an argument but we don't use. */
		switch (*p) {
		case 'b':
		case 'k':
		case 'l':
		case 'e':
		case 'I':
			++argindex;
			continue;
		}

		/* Find which mode this symbol is (e.g. o=@). */
		if ((mode = modes_find(s, *p)) == 0) {
			++argindex;
			continue;
		}
		if (!msg->args[argindex] || !(u = irc_channel_get(ch, msg->args[argindex]))) {
			++argindex;
			continue;
		}

		++argindex;

		if (action == '+')
			irc_channel_set(ch, u->nickname, u->modes | (1 << mode));
		else
			irc_channel_set(ch, u->nickname, u->modes & ~(1 << mode));
	}

skip:
	irc_bot_dispatch(&ev);
}

static void
handle_part(struct irc_server *server, struct msg *msg)
{
	struct irc_channel *ch;
	struct irc_event ev = {};

	ev.type = IRC_EVENT_PART;
	ev.server = server;
	ev.part.origin = irc_util_strdup(msg->prefix);
	ev.part.channel = irc_util_strdup(msg->args[0]);
	ev.part.reason = msg->args[1] ? irc_util_strdup(msg->args[1]) : NULL;

	ch = server_channels_find(server, ev.part.channel);

	if (server_is_self(server, ev.part.origin)) {
		server_channels_remove(server, ch);
		irc_log_info("leaving channel %s", ev.part.channel);
	} else
		irc_channel_remove(ch, ev.part.origin);

	irc_bot_dispatch(&ev);
}

static void
handle_msg(struct irc_server *server, struct msg *msg)
{
	time_t now = time(NULL);
	struct irc_user *user;
	struct irc_event ev = {};

	ev.server = server;

	/*
	 * Detect CTCP commands which are PRIVMSG with a special boundaries.
	 *
	 * Example:
	 * PRIVMSG jean :\001ACTION I'm eating\001
	 * PRIVMSG jean :\001VERSION\001
	 */
	if (msg_is_ctcp(msg->args[1]) && (user = irc_util_user_split(msg->prefix))) {
		if (strcmp(msg->args[1], "\x01""CLIENTINFO\x01") == 0)
			irc_server_notice(server, user->nickname,
			    "\x01""CLIENTINFO ACTION CLIENTINFO SOURCE TIME VERSION\x01");
		else if (strcmp(msg->args[1], "\x01""SOURCE\x01") == 0 && server->ctcp_source)
			irc_server_send(server, "NOTICE %s :\x01SOURCE %s\x01",
			    user->nickname, server->ctcp_source);
		else if (strcmp(msg->args[1], "\x01""TIME\x01") == 0) {
			irc_server_send(server, "NOTICE %s :\x01TIME %s\x01",
			    user->nickname, ctime(&now));
		} else if (strcmp(msg->args[1], "\x01VERSION\x01") == 0 && server->ctcp_version) {
			irc_server_send(server, "NOTICE %s :\x01VERSION %s\x01",
				user->nickname, server->ctcp_version);
		} else if (strncmp(msg->args[1], "\x01""ACTION", 7) == 0) {
			ev.type = IRC_EVENT_ME;
			ev.message.origin = irc_util_strdup(msg->prefix);
			ev.message.channel = irc_util_strdup(msg->args[0]);
			ev.message.message = irc_util_strdup(msg_ctcp(msg->args[1]));
		}

		irc_util_user_free(user);
	} else {
		ev.type = IRC_EVENT_MESSAGE;
		ev.message.origin = irc_util_strdup(msg->prefix);
		ev.message.channel = irc_util_strdup(msg->args[0]);
		ev.message.message = irc_util_strdup(msg->args[1]);
	}

	irc_bot_dispatch(&ev);
}

static void
handle_nick(struct irc_server *server, struct msg *msg)
{
	struct irc_event ev = {};

	ev.type = IRC_EVENT_NICK;
	ev.server = server;
	ev.nick.origin = irc_util_strdup(msg->prefix);
	ev.nick.nickname = irc_util_strdup(msg->args[0]);

	/* Update nickname if it is myself. */
	if (server_is_self(server, ev.nick.origin)) {
		INFO("nick change %s -> %s", server->nickname, ev.nick.nickname);
		server->nickname = irc_util_strdupfree(server->nickname, ev.nick.nickname);
	}

	irc_bot_dispatch(&ev);
}

static void
handle_notice(struct irc_server *s, struct msg *msg)
{
	struct irc_event ev = {};

	ev.type = IRC_EVENT_NOTICE;
	ev.server = s;
	ev.notice.origin = irc_util_strdup(msg->prefix);
	ev.notice.channel = irc_util_strdup(msg->args[0]);
	ev.notice.notice = irc_util_strdup(msg->args[1]);

	irc_bot_dispatch(&ev);
}

static void
handle_topic(struct irc_server *s, struct msg *msg)
{
	struct irc_event ev = {};

	ev.type = IRC_EVENT_TOPIC;
	ev.server = s;
	ev.topic.origin = irc_util_strdup(msg->prefix);
	ev.topic.channel = irc_util_strdup(msg->args[0]);
	ev.topic.topic = irc_util_strdup(msg->args[1]);

	irc_bot_dispatch(&ev);
}

static void
handle_ping(struct irc_server *s, struct msg *msg)
{
	struct conn *conn = s->conn;

	ctimer_again(&conn->timer.timer);

	if (msg->args[0])
		irc_server_send(s, "PONG :%s", msg->args[0]);
}

static void
handle_names(struct irc_server *s, struct msg *msg)
{
	struct irc_channel *ch;
	char *p, *token;
	int modes = 0;

	ch = server_channels_add(s, msg->args[2], NULL, 1);

	/* Track existing nicknames into the given channel. */
	for (p = msg->args[3]; (token = strtok_r(p, " ", &p)); ) {
		if (strlen(token) == 0)
			continue;

		modes = irc_server_strip(s, (const char **)&token);
		irc_channel_add(ch, token, modes);
	}
}

static void
handle_endofnames(struct irc_server *s, struct msg *msg)
{
	const struct irc_channel *ch;
	const struct irc_channel_user *u;
	struct irc_event ev = {};
	size_t i = 0;

	ev.type = IRC_EVENT_NAMES;
	ev.server = s;
	ev.names.channel = irc_util_strdup(msg->args[1]);

	if (!(ch = server_channels_find(s, ev.names.channel)))
		return;

	ev.names.usersz = irc_channel_count(ch);
	ev.names.users  = irc_util_calloc(ev.names.usersz, sizeof (*ev.names.users));

	LL_FOREACH(ch->users, u) {
		ev.names.users[i].nickname = irc_util_strdup(u->nickname);
		ev.names.users[i].modes = u->modes;
	}

	irc_bot_dispatch(&ev);
}

static void
handle_nicknameinuse(struct irc_server *server, struct msg *)
{
	WARN("nickname %s is already in use", server->nickname);
}

static void
handle_error(struct irc_server *server, struct msg *msg)
{
	if (msg->args[0])
		WARN("%s", msg->args[0]);
}

static void
handle_whoisuser(struct irc_server *server, struct msg *msg)
{
	server->bufwhois.nickname = irc_util_strdup(msg->args[1]);
	server->bufwhois.username = irc_util_strdup(msg->args[2]);
	server->bufwhois.hostname = irc_util_strdup(msg->args[3]);
	server->bufwhois.realname = irc_util_strdup(msg->args[5]);
}

static void
handle_whoischannels(struct irc_server *server, struct msg *msg)
{
	char *token, *p;
	int modes;

	if (!msg->args[2])
		return;

	for (p = msg->args[2]; (token = strtok_r(p, " ", &p)); ) {
		modes = irc_server_strip(server, (const char **)&token);
		server->bufwhois.channels = irc_util_reallocarray(
		    server->bufwhois.channels,
		    server->bufwhois.channelsz + 1,
		    sizeof (*server->bufwhois.channels)
		);

		server->bufwhois.channels[server->bufwhois.channelsz].name = irc_util_strdup(token);
		server->bufwhois.channels[server->bufwhois.channelsz++].modes = modes;
	}
}

/*
 * TODO: refactor this to avoid using a temporary buffer.
 */
static void
handle_endofwhois(struct irc_server *server, struct msg *)
{
	struct irc_event ev = {};

	ev.server = server;
	ev.type = IRC_EVENT_WHOIS;
	ev.whois = server->bufwhois;

	irc_bot_dispatch(&ev);

	/* Get rid of buffered whois. */
	memset(&server->bufwhois, 0, sizeof (server->bufwhois));
}

static const struct handler {
	const char *command;
	void (*handle)(struct irc_server *, struct msg *);
} handlers[] = {
	/* Must be kept ordered. */
	{ "001",        handle_connect          },
	{ "005",        handle_support          },
	{ "311",        handle_whoisuser        },
	{ "318",        handle_endofwhois       },
	{ "319",        handle_whoischannels    },
	{ "353",        handle_names            },
	{ "366",        handle_endofnames       },
	{ "433",        handle_nicknameinuse    },
	{ "ERROR",      handle_error            },
	{ "INVITE",     handle_invite           },
	{ "JOIN",       handle_join             },
	{ "KICK",       handle_kick             },
	{ "MODE",       handle_mode             },
	{ "NICK",       handle_nick             },
	{ "NOTICE",     handle_notice           },
	{ "PART",       handle_part             },
	{ "PING",       handle_ping             },
	{ "PRIVMSG",    handle_msg              },
	{ "TOPIC",      handle_topic            }
};

static int
handler_cmp(const void *name, const void *data)
{
	const struct handler *handler = data;

	return strcmp(name, handler->command);
}

static void
handler(struct irc_server *s, struct msg *msg)
{
	const struct handler *h;

	h = bsearch(msg->cmd, handlers, IRC_UTIL_SIZE(handlers),
	    sizeof (struct handler), handler_cmp);

	if (h)
		h->handle(s, msg);
}

/* }}} */

/*
 * Return a human friendly description of an remote address.
 */
static const char *
conn_info(const struct conn *conn, const struct addrinfo *ai)
{
	static char info[80];
	char ip[INET6_ADDRSTRLEN + 1] = {};
	const void *addr = NULL;
	const char *family;

	if (ai->ai_family == AF_INET) {
		family = "IPv4";
		addr = &((const struct sockaddr_in *)ai->ai_addr)->sin_addr;
	} else if (ai->ai_family == AF_INET6) {
		family = "IPv6";
		addr = &((const struct sockaddr_in6 *)ai->ai_addr)->sin6_addr;
	}

	if (addr) {
		inet_ntop(ai->ai_family, addr, ip, sizeof (ip));
		snprintf(info, sizeof (info), "%s %s port %u", family, ip, conn->parent->port);
	} else
		snprintf(info, sizeof (info), "unknown %d family type", ai->ai_family);

	return info;
}

static inline void
conn_reschedule(struct conn *conn)
{
	/* First, notify everyone about disconnection. */
	handle_disconnect(conn->parent);

	/* Stop at least our socket watcher, finalizer will do the rest. */
	cio_stop(&conn->io_fd.io);

	/* Feed the watchdog timer quickly. */
	ctimer_feed(&conn->timer.timer, EV_TIMER);
	coro_idle();
}

/*
 * Attempt to resolve the server IRC hostname into a broken down list of
 * addrinfo in the conn->ai_list field.
 */
static void
conn_resolve(struct conn *conn)
{
	struct addrinfo hints = {};
	char service[16] = {};
	int rc;

	/* Prevent use of IPv4/IPv6 if only one is specified. */
	if (conn->parent->flags & IRC_SERVER_FLAGS_NO_IPV4)
		hints.ai_family = AF_INET6;
	else if (conn->parent->flags & IRC_SERVER_FLAGS_NO_IPV6)
		hints.ai_family = AF_INET;
	else
		hints.ai_family = AF_UNSPEC;

	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags    = AI_NUMERICSERV;

	snprintf(service, sizeof (service), "%u", conn->parent->port);

	/*
	 * If this function fail there is nothing we can do except going
	 * directly to the reconnect later step.
	 */
	if ((rc = getaddrinfo(conn->parent->hostname, service, &hints, &conn->ai_list)) != 0) {
		irc_log_warn("server %s: getaddrinfo: %s", conn->parent->name, gai_strerror(rc));
		conn_reschedule(conn);
	} else {
		conn->state = STATE_CONNECT;

		for (const struct addrinfo *ai = conn->ai_list; ai; ai = ai->ai_next)
			irc_log_debug("server %s: resolves to %s",
			    conn->parent->name, conn_info(conn, ai));

		/* Start with first one. */
		conn->ai = conn->ai_list;
	}
}

/*
 * Create a non-blocking socket.
 */
static int
conn_socket(struct conn *conn)
{
	const struct addrinfo *ai = conn->ai;
	int flags;

	if ((conn->fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol)) < 0) {
		irc_log_warn("server %s: socket: %s", conn->parent->name, strerror(errno));
		return -1;
	}
	if ((flags = fcntl(conn->fd, F_GETFL)) < 0 || fcntl(conn->fd, F_SETFL, flags | O_NONBLOCK) < 0) {
		irc_log_warn("server %s: fcntl: %s", conn->parent->name, strerror(errno));
		close(conn->fd);
		conn->fd = -1;
		return -1;
	}

	return 0;
}

/*
 * Attempt to connect to the current conn->ai endpoint asynchronously.
 */
static int
conn_dial(struct conn *conn)
{
	int rc = -1;
	socklen_t len;

	if (connect(conn->fd, conn->ai->ai_addr, conn->ai->ai_addrlen) < 0) {
		if (errno != EINPROGRESS && errno != EAGAIN) {
			irc_log_warn("server %s: connect: %s", conn->parent->name, strerror(errno));
			goto exit;
		}

		/*
		 * When connection is in progress the socket will be writable
		 * once connection is complete or error'ed.
		 */
		irc_log_debug("server %s: connect in progress", conn->parent->name);
		cio_reset(&conn->io_fd.io, conn->fd, EV_WRITE);
		cio_wait(&conn->io_fd.io);
		cio_stop(&conn->io_fd.io);

		/*
		 * Determine if the non blocking connect(2) call succeeded, otherwise
		 * we re-try again a connect to the next endpoint.
		 */
		len = sizeof (rc);
		getsockopt(conn->fd, SOL_SOCKET, SO_ERROR, &rc, &len);
	} else {
		/* Immediate success (unlikely) */
		rc = 0;
	}

exit:
	if (rc < 0) {
		close(conn->fd);
		conn->fd = -1;
	} else
		irc_log_debug("server %s: connect succeeded", conn->parent->name);

	return rc;
}

/*
 * Create the socket and connect to every endpoint resolved in the server list.
 */
static void
conn_connect(struct conn *conn)
{
	struct irc_server *server = conn->parent;
	int rc = 0;

	for (;;) {
		if (conn_socket(conn) < 0 || conn_dial(conn) < 0)
			rc = -1;

		/* Success. */
		if (rc == 0)
			break;
		if ((conn->ai = conn->ai->ai_next) == NULL) {
			WARN("no more endpoint available");
			break;
		}
	}

	if (rc < 0)
		conn_reschedule(conn);

#if IRCCD_WITH_SSL == 1
	/*
	 * Now go with SSL if needed. It does not require that much aside a new
	 * handle. All handshake process will be done transparently.
	 */
	if (server->flags & IRC_SERVER_FLAGS_SSL) {
		conn->tls = tls_client();

		if (tls_connect_socket(conn->tls, conn->fd, server->hostname) < 0) {
			WARN("TLS error: %s", tls_error(conn->tls));
			conn_reschedule(conn);
		}
	}
#endif

	conn->state = STATE_IDENT;
}

static ssize_t
conn_tcp_recv(struct conn *conn, void *buf, size_t bufsz, int *events)
{
	ssize_t nr;

	while ((nr = recv(conn->fd, buf, bufsz, MSG_NOSIGNAL)) < 0 && errno == EINTR)
		continue;

	if (nr < 0) {
		/*
		 * Called with no events, we allow it and simply return no data
		 * for now.
		 */
		if (errno == EWOULDBLOCK || errno == EAGAIN) {
			*events = EV_READ;
			nr = 0;
		} else
			irc_log_warn("server %s: recv: %s",
			    conn->parent->name, strerror(errno));
	} else if (nr == 0) {
		irc_log_warn("server %s: remote closed connection", conn->parent->name);
		nr = -1;
	} else if (conn->insz < sizeof (conn->in))
		*events |= EV_READ;

	return nr;
}

static ssize_t
conn_tcp_send(struct conn *conn, const void *buf, size_t bufsz, int *events)
{
	ssize_t ns;

	while ((ns = send(conn->fd, buf, bufsz, MSG_NOSIGNAL)) < 0 && errno != EINTR)
		continue;

	/*
	 * Similar to conn_plain_recv, accept a call with not able to send for
	 * now.
	 */
	if (ns < 0) {
		if (errno == EWOULDBLOCK || errno == EAGAIN) {
			*events |= EV_WRITE;
			ns = 0;
		} else
			irc_log_warn("server %s: send: %s",
			    conn->parent->name, strerror(errno));
	} else if ((size_t)ns < bufsz)
		*events |= EV_WRITE;

	return ns;
}

#if IRCCD_WITH_SSL == 1

static inline ssize_t
conn_tls_want(const struct conn *conn, ssize_t rc, int *events)
{
	if (rc < 0) {
		switch (rc) {
		case TLS_WANT_POLLIN:
			rc = 0;
			*events = EV_READ;
			break;
		case TLS_WANT_POLLOUT:
			rc = 0;
			*events = EV_WRITE;
			break;
		default:
			break;
		}
	} else {
		if (conn->insz < sizeof (conn->in))
			*events |= EV_READ;
		if (conn->outsz)
			*events |= EV_WRITE;
	}

	return rc;
}

static ssize_t
conn_tls_recv(struct conn *conn, void *buf, size_t bufsz, int *events)
{
	ssize_t rc;

	rc = tls_read(conn->tls, buf, bufsz);

	return conn_tls_want(conn, rc, events);
}

static ssize_t
conn_tls_send(struct conn *conn, const void *buf, size_t bufsz, int *events)
{
	ssize_t rc;

	rc = tls_write(conn->tls, buf, bufsz);

	return conn_tls_want(conn, rc, events);
}

#endif

static int
conn_recv(struct conn *conn, int *events)
{
	ssize_t nr;
	size_t limit;

	limit = sizeof (conn->in) - conn->insz;

	if ((nr = conn->recv(conn, &conn->in[conn->insz], limit, events)) > 0)
		conn->insz += nr;

	return nr;
}

/*
 * Attempt to send the output buffer from the connection. This function should
 * usually not be called if the output buffer is empty.
 */
static int
conn_send(struct conn *conn, int *events)
{
	ssize_t ns;

	if (conn->outsz == 0)
		return 0;

	ns = conn->send(conn, conn->out, conn->outsz, events);

	if (ns > 0) {
		if ((size_t)ns >= conn->outsz) {
			memset(conn->out, 0, sizeof (conn->out));
			conn->outsz = 0;
		} else {
			memmove(conn->out, conn->out + ns, sizeof (conn->out) - ns);
			conn->outsz -= ns;
		}
	}

	return ns;
}

static int
conn_push(struct conn *conn, const char *data, size_t datasz)
{
	if (datasz + 2 >= sizeof (conn->out) - conn->outsz)
		return -ENOBUFS;

	memcpy(&conn->out[conn->outsz], data, datasz);
	conn->outsz += datasz;
	memcpy(&conn->out[conn->outsz], "\r\n", 2);
	conn->outsz += 2;

	cio_reset(&conn->io_fd.io, conn->fd, EV_READ | EV_WRITE);

	return 0;
}

/*
 * Wait for socket activity.
 */
static int
conn_wait(struct conn *conn)
{
	int events, revents, rc;

	revents = cio_wait(&conn->io_fd.io);
	events = 0;

	if ((revents & EV_READ) && (rc = conn_recv(conn, &events)) < 0)
		return rc;
	if ((revents & EV_WRITE) && (rc = conn_send(conn, &events)) < 0)
		return rc;

	if (events != conn->io_fd.io.io.events)
		cio_reset(&conn->io_fd.io, conn->fd, events);

	return 0;
}

/*
 * Yield until the next message from the IRC server comes.
 */
static int
conn_next(struct conn *conn, struct msg *msg)
{
	char *pos;
	size_t length;
	int rc;

	while (!(pos = memmem(conn->in, conn->insz, "\r\n", 2))) {
		if ((rc = conn_wait(conn)) < 0)
			return rc;
	}

	length = pos - conn->in;

	if (length > 0 && length < IRC_MESSAGE_LEN)
		msg_parse(msg, conn->in, length);

	/* (Re)move the first message received. */
	memmove(conn->in, pos + 2, sizeof (conn->in) - (length + 2));
	conn->insz -= length + 2;

	return 0;
}

/*
 * Exchange until we discover a proper IRC server.
 */
static void
conn_ident(struct conn *conn)
{
	struct msg msg;
	int rc = 0;

	/*
	 * Use multi-prefix extension to keep track of all combined "modes" in
	 * a channel.
	 *
	 * https://ircv3.net/specs/extensions/multi-prefix-3.1.html
	 */
	irc_server_send(conn->parent, "CAP REQ :multi-prefix");

	if (conn->parent->password)
		irc_server_send(conn->parent, "PASS %s", conn->parent->password);

	irc_server_send(conn->parent, "NICK %s", conn->parent->nickname);
	irc_server_send(conn->parent, "USER %s %s %s :%s",
	                conn->parent->username,
	                conn->parent->username,
	                conn->parent->username,
	                conn->parent->realname);
	irc_server_send(conn->parent, "CAP END");

	while (conn->state == STATE_IDENT) {
		if ((rc = conn_next(conn, &msg)) < 0)
			break;

		/* 001 will be our connection status indicator. */
		if (strcmp(msg.cmd, "001") == 0) {
			conn->state = STATE_READY;

			/*
			 * Now that we are ready we can tell the watchdog that
			 * it must inspect larger timeouts.
			 */
			ctimer_stop(&conn->timer.timer);
			ctimer_set(&conn->timer.timer, PING_TIMEOUT, PING_TIMEOUT);
			ctimer_start(&conn->timer.timer);
		}

		handler(conn->parent, &msg);
	}

	if (rc < 0)
		conn_reschedule(conn);
}

/*
 * Loop until something wrong appears.
 */
static void
conn_ready(struct conn *conn)
{
	struct msg msg;
	int rc;

	while ((rc = conn_next(conn, &msg)) == 0)
		handler(conn->parent, &msg);

	if (rc < 0)
		conn_reschedule(conn);
}

static void
conn_io_entry(struct cio *self)
{
	struct conn *conn = CONN(self, io_fd.io);

	conn->state = STATE_RESOLVE;

	for (;;) {
		/* Rearm timer */
		ctimer_again(&conn->timer.timer);

		switch (conn->state) {
		case STATE_RESOLVE:
			conn_resolve(conn);
			break;
		case STATE_CONNECT:
			conn_connect(conn);
			break;
		case STATE_IDENT:
			conn_ident(conn);
			break;
		case STATE_READY:
			conn_ready(conn);
			break;
		default:
			/* Oops forgot to implement a state. */
			fprintf(stderr, "state not implemented: %d", conn->state);
			abort();
			break;
		}
	}
}

static void
conn_io_finalizer(struct cio *self)
{
	struct conn *conn = CONN(self, io_fd.io);

	if (conn->ai_list) {
		freeaddrinfo(conn->ai_list);
		conn->ai_list = NULL;
		conn->ai = NULL;
	}

#if IRCCD_WITH_SSL
	if (conn->tls) {
		tls_close(conn->tls);
		tls_free(conn->tls);
		conn->tls = NULL;
	}
#endif

	if (conn->fd != -1) {
		close(conn->fd);
		conn->fd = -1;
	}
}

static void
conn_io_spawn(struct conn *conn)
{
	cio_coro_spawn(&conn->io_fd, &(const struct cio_coro_def) {
		.name      = "irc_server.io",
		.flags     = CORO_ATTACHED | CORO_INACTIVE,
		.entry     = conn_io_entry,
		.finalizer = conn_io_finalizer
	});
}

static void
conn_timer_resurrect(struct conn *conn)
{
	struct irc_server *server = conn->parent;

	switch (conn->state) {
	case STATE_RESOLVE:
	case STATE_CONNECT:
		WARN("timeout while connecting");
		break;
	case STATE_IDENT:
		WARN("error during IRC protocol");
		break;
	case STATE_READY:
		WARN("ping timeout");
		break;
	default:
		break;
	}

	coro_finish(&conn->io_fd.coro);

	/* Now reschedule ourself to reconnect later. */
	ctimer_stop(&conn->timer.timer);
	ctimer_set(&conn->timer.timer, 2.0, 0.0);
	ctimer_start(&conn->timer.timer);
	ctimer_wait(&conn->timer.timer);
	ctimer_stop(&conn->timer.timer);

	conn_io_spawn(conn);
}

static void
conn_timer_entry(struct ctimer *self)
{
	struct conn *conn = CONN(self, timer.timer);

	while (ctimer_wait(self))
		conn_timer_resurrect(conn);
}

static void
conn_timer_spawn(struct conn *conn)
{
	ctimer_coro_spawn(&conn->timer, &(const struct ctimer_coro_def) {
		.name   = "irc_server.timer",
		.flags  = CORO_ATTACHED,
		.entry  = conn_timer_entry,
		.after  = 60.0,
		.repeat = 60.0
	});
}

static void
conn_spawn(struct irc_server *server)
{
	struct conn *conn;

	conn = irc_util_calloc(1, sizeof (*conn));
	conn->parent = server;

#if IRCCD_WITH_SSL == 1
	if (server->flags & IRC_SERVER_FLAGS_SSL) {
		conn->recv = conn_tls_recv;
		conn->send = conn_tls_send;
	} else {
		conn->recv = conn_tcp_recv;
		conn->send = conn_tcp_send;
	}
#else
	conn->recv = conn_tcp_recv;
	conn->send = conn_tcp_send;
#endif

	server->conn = conn;

	conn_timer_spawn(conn);
	conn_io_spawn(conn);
}

struct irc_server *
irc_server_new(const char *name)
{
	assert(name);

	struct irc_server *s;

	s = irc_util_calloc(1, sizeof (*s));

	/* Predefined sane defaults. */
	s->name         = irc_util_strdup(name);
	s->port         = IRC_SERVER_DEFAULT_PORT;
	s->prefix       = irc_util_strdup(IRC_SERVER_DEFAULT_PREFIX);
	s->ctcp_version = irc_util_strdup(IRC_SERVER_DEFAULT_CTCP_VERSION);
	s->ctcp_source  = irc_util_strdup(IRC_SERVER_DEFAULT_CTCP_SOURCE);

	return s;
}

void
irc_server_set_ident(struct irc_server *s,
                     const char *nickname,
                     const char *username,
                     const char *realname)
{
	assert(s);
	assert(nickname);
	assert(username);
	assert(realname);

	/* must not be connected */
	assert(!s->conn);

	s->nickname = irc_util_strdupfree(s->nickname, nickname);
	s->username = irc_util_strdupfree(s->username, username);
	s->realname = irc_util_strdupfree(s->realname, realname);
}

void
irc_server_set_params(struct irc_server *s,
                      const char *hostname,
                      unsigned int port,
                      enum irc_server_flags flags)
{
	assert(s);
	assert(hostname);

	/* must not be connected */
	assert(!s->conn);

	s->hostname = irc_util_strdupfree(s->hostname, hostname);
	s->port     = port;
	s->flags    = flags;
}

void
irc_server_set_ctcp(struct irc_server *s,
                    const char *version,
                    const char *source)
{
	assert(s);

	s->ctcp_version = irc_util_strdupfree(s->ctcp_version, version);
	s->ctcp_source  = irc_util_strdupfree(s->ctcp_source, source);
}

void
irc_server_set_prefix(struct irc_server *s, const char *prefix)
{
	assert(s);
	assert(prefix);

	s->prefix = irc_util_strdupfree(s->prefix, prefix);
}

void
irc_server_set_password(struct irc_server *s, const char *password)
{
	assert(s);

	/* must not be connected */
	assert(!s->conn);

	s->password = irc_util_strdupfree(s->password, password);
}

void
irc_server_connect(struct irc_server *s)
{
	assert(s);
	assert(s->hostname);
	assert(s->nickname);
	assert(s->username);
	assert(s->realname);

	if (s->conn) {
		irc_log_info("server %s: already connected", s->name);
		return;
	}

#if !IRCCD_WITH_SSL == 1
	if (s->flags & IRC_SERVER_FLAGS_SSL) {
		irc_log_warn("server %s: SSL requested but not available", s->name);
		return;
	}
#endif

	conn_spawn(s);
}

void
irc_server_disconnect(struct irc_server *s)
{
	assert(s);

	struct conn *conn = s->conn;

	if (!conn) {
		irc_log_info("server %s: already disconnected", s->name);
		return;
	}

	coro_finish(&conn->io_fd.coro);
	coro_finish(&conn->timer.coro);

	server_channels_clear(s);
	server_clear(s);
}

void
irc_server_reconnect(struct irc_server *s)
{
	assert(s);

	irc_server_disconnect(s);
	irc_server_connect(s);
}

const struct irc_channel *
irc_server_channels_find(struct irc_server *s, const char *name)
{
	assert(s);
	assert(name);

	return server_channels_find(s, name);
}

int
irc_server_send(struct irc_server *s, const char *fmt, ...)
{
	assert(s);
	assert(fmt);

	va_list ap;
	int rc;

	va_start(ap, fmt);
	rc = irc_server_send_va(s, fmt, ap);
	va_end(ap);

	return rc;
}

int
irc_server_send_va(struct irc_server *s, const char *fmt, va_list ap)
{
	char buf[IRC_MESSAGE_LEN];
	struct conn *conn = s->conn;

	if (!conn)
		return -ENOTCONN;

	vsnprintf(buf, sizeof (buf), fmt, ap);

	return conn_push(s->conn, buf, strlen(buf));
}

int
irc_server_invite(struct irc_server *s, const char *channel, const char *target)
{
	assert(s);
	assert(channel);
	assert(target);

	return irc_server_send(s, "INVITE %s %s", target, channel);
}

int
irc_server_join(struct irc_server *s, const char *name, const char *pass)
{
	assert(s);
	assert(name);

	struct conn *conn = s->conn;
	struct irc_channel *ch;
	int ret = 1;

	/*
	 * Search if there is already a channel pending or joined. If the
	 * server is connected we send a join command otherwise we put it there
	 * and wait for connection.
	 */
	if (!(ch = server_channels_find(s, name)))
		ch = server_channels_add(s, name, pass, IRC_CHANNEL_FLAGS_NONE);

	if (!(ch->flags & IRC_CHANNEL_FLAGS_JOINED) && (conn && conn->state == STATE_READY)) {
		if (pass)
			ret = irc_server_send(s, "JOIN %s %s", name, pass);
		else
			ret = irc_server_send(s, "JOIN %s", name);
	}

	return ret;
}

int
irc_server_kick(struct irc_server *s, const char *channel, const char *target, const char *reason)
{
	assert(s);
	assert(channel);
	assert(target);

	int ret;

	if (reason)
		ret = irc_server_send(s, "KICK %s %s :%s", channel, target, reason);
	else
		ret = irc_server_send(s, "KICK %s %s", channel, target);

	return ret;
}

int
irc_server_part(struct irc_server *s, const char *channel, const char *reason)
{
	assert(s);
	assert(channel);

	int ret;

	if (reason && strlen(reason) > 0)
		ret = irc_server_send(s, "PART %s :%s", channel, reason);
	else
		ret = irc_server_send(s, "PART %s", channel);

	return ret;
}

int
irc_server_topic(struct irc_server *s, const char *channel, const char *topic)
{
	assert(s);
	assert(channel);
	assert(topic);

	return irc_server_send(s, "TOPIC %s :%s", channel, topic);
}

int
irc_server_message(struct irc_server *s, const char *chan, const char *msg)
{
	assert(s);
	assert(chan);
	assert(msg);

	return irc_server_send(s, "PRIVMSG %s :%s", chan, msg);
}

int
irc_server_me(struct irc_server *s, const char *chan, const char *message)
{
	assert(s);
	assert(chan);
	assert(message);

	return irc_server_send(s, "PRIVMSG %s :\001ACTION %s\001", chan, message);
}

int
irc_server_mode(struct irc_server *s, const char *channel, const char *mode, const char *args)
{
	assert(s);
	assert(channel);
	assert(mode);

	args = args ? args : "";

	return irc_server_send(s, "MODE %s %s %s", channel, mode, args);
}

int
irc_server_names(struct irc_server *s, const char *channel)
{
	return irc_server_send(s, "NAMES %s", channel);
}

int
irc_server_nick(struct irc_server *s, const char *nickname)
{
	assert(s);
	assert(nickname);

	/*
	 * If not connected, just replace nickname immediately.
	 */
	if (!s->conn) {
		s->nickname = irc_util_strdupfree(s->nickname, nickname);
		return 0;
	}

	return irc_server_send(s, "NICK %s", nickname);
}

int
irc_server_notice(struct irc_server *s, const char *channel, const char *message)
{
	assert(s);
	assert(channel);
	assert(message);

	return irc_server_send(s, "NOTICE %s :%s", channel, message);
}

int
irc_server_whois(struct irc_server *s, const char *target)
{
	assert(s);
	assert(target);

	return irc_server_send(s, "WHOIS %s", target);
}

int
irc_server_strip(const struct irc_server *s, const char **what)
{
	assert(s);
	assert(*what);

	int modes = 0;

	for (size_t i = 0; i < s->prefixesz; ++i) {
		if (**what == s->prefixes[i].symbol) {
			modes |= 1 << i;
			*what += 1;
		}
	}

	return modes;
}

void
irc_server_incref(struct irc_server *s)
{
	assert(s);

	s->refc++;
}

void
irc_server_decref(struct irc_server *s)
{
	assert(s);
	assert(s->refc >= 1);

	if (--s->refc == 0)
		server_free(s);
}
