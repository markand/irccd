/*
 * server.c -- an IRC server
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

#include <assert.h>
#include <errno.h>
#include <err.h>
#include <poll.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"

#if defined(IRCCD_WITH_SSL)
#       include <openssl/err.h>
#endif

#include "channel.h"
#include "conn.h"
#include "event.h"
#include "log.h"
#include "server.h"
#include "util.h"

#define DELAY   30      /* Seconds to wait before reconnecting. */
#define TIMEOUT 1800    /* Seconds before marking a server as dead. */

static inline void
clear_channels(struct irc_server *s, int free)
{
	struct irc_channel *c, *tmp;

	LIST_FOREACH_SAFE(c, &s->channels, link, tmp) {
		if (free)
			irc_channel_finish(c);
		else
			irc_channel_clear(c);
	}

	if (free)
		LIST_INIT(&s->channels);
}

static inline void
clear_server(struct irc_server *s)
{
	irc_conn_finish(s->conn);

	free(s->bufwhois.nickname);
	free(s->bufwhois.username);
	free(s->bufwhois.realname);
	free(s->bufwhois.hostname);
	free(s->bufwhois.channels);

	memset(&s->params, 0, sizeof (s->params));
	memset(&s->bufwhois, 0, sizeof (s->bufwhois));
}

static inline int
is_self(const struct irc_server *s, const char *nick)
{
	return strncmp(s->ident.nickname, nick, strlen(s->ident.nickname)) == 0;
}

static struct irc_channel *
add_channel(struct irc_server *s, const char *name, const char *password, int joined)
{
	struct irc_channel *ch;

	if ((ch = irc_server_find(s, name))) {
		ch->joined = joined;
		return ch;
	}

	ch = irc_util_calloc(1, sizeof (*ch));
	ch->joined = joined;
	strlcpy(ch->name, name, sizeof (ch->name));

	if (password)
		strlcpy(ch->password, password, sizeof (ch->password));

	LIST_INIT(&ch->users);
	LIST_INSERT_HEAD(&s->channels, ch, link);

	return ch;
}

static void
remove_channel(struct irc_channel *ch)
{
	LIST_REMOVE(ch, link);
	irc_channel_finish(ch);
}

static int
is_ctcp(const char *line)
{
	size_t length;

	if (!line)
		return 0;
	if ((length = strlen(line)) < 2)
		return 0;

	return line[0] == 0x1 && line[length - 1] == 0x1;
}

static char *
ctcp(char *line)
{
	/* Skip first \001. */
	if (*line == '\001')
		line++;

	/* Remove last \001. */
	line[strcspn(line, "\001")] = '\0';

	if (strncmp(line, "ACTION ", 7) == 0)
		line += 7;

	return line;
}

static inline int
find_mode(struct irc_server *s, int mode)
{
	for (size_t i = 0; i < IRC_UTIL_SIZE(s->params.prefixes); ++i)
		if (s->params.prefixes[i].mode == mode)
			return i;

	return 0;
}

static void
read_support_prefix(struct irc_server *s, const char *value)
{
	char modes[IRC_UTIL_SIZE(s->params.prefixes) + 1] = {0};
	char tokens[IRC_UTIL_SIZE(s->params.prefixes) + 1] = {0};
	char fmt[32] = {0};

	snprintf(fmt, sizeof (fmt), "(%%%zu[^)])%%%zus",
	    sizeof (modes) - 1, sizeof (tokens) - 1);

	if (sscanf(value, fmt, modes, tokens) == 2) {
		char *pm = modes;
		char *sm = tokens;

		for (size_t i = 0; i < IRC_UTIL_SIZE(s->params.prefixes) && *pm && *sm; ++i) {
			s->params.prefixes[i].mode = *pm++;
			s->params.prefixes[i].symbol = *sm++;
		}
	}
}

static void
read_support_chantypes(struct irc_server *s, const char *value)
{
	strlcpy(s->params.chantypes, value, sizeof (s->params.chantypes));
}

static void
fail(struct irc_server *s)
{
	clear_channels(s, 0);
	clear_server(s);

	if (s->flags & IRC_SERVER_FLAGS_AUTO_RECO) {
		irc_log_info("server %s: waiting %u seconds before reconnecting", s->name, DELAY);
		s->state = IRC_SERVER_STATE_WAITING;
	} else {
		s->state = IRC_SERVER_STATE_DISCONNECTED;
	}

	/* Time point when we lose signal from the server. */
	s->lost_tp = time(NULL);
}

static void
handle_connect(struct irc_server *s, struct irc_event *ev, struct irc_conn_msg *msg)
{
	(void)msg;

	struct irc_channel *ch;

	/* Now join all channels that were requested. */
	LIST_FOREACH(ch, &s->channels, link)
		irc_server_join(s, ch->name, ch->password);

	s->state = IRC_SERVER_STATE_CONNECTED;
	ev->type = IRC_EVENT_CONNECT;

	irc_log_info("server %s: connection complete", s->name);
}

static void
handle_disconnect(struct irc_server *s, struct irc_event *ev)
{
	ev->type = IRC_EVENT_DISCONNECT;
	ev->server = s;

	fail(s);
	irc_log_info("server %s: connection lost", s->name);
}

static void
handle_support(struct irc_server *s, struct irc_event *ev, struct irc_conn_msg *msg)
{
	(void)ev;

	char key[64];
	char value[64];

	for (size_t i = 0; i < IRC_UTIL_SIZE(msg->args) && msg->args[i]; ++i) {
		if (sscanf(msg->args[i], "%63[^=]=%63s", key, value) != 2)
			continue;

		if (strcmp(key, "PREFIX") == 0) {
			read_support_prefix(s, value);
			irc_log_info("server %s: prefixes:           %s",
			    s->name, value);
		} else if (strcmp(key, "CHANTYPES") == 0) {
			read_support_chantypes(s, value);
			irc_log_info("server %s: channel types:      %s",
			    s->name, value);
		} else if (strcmp(key, "CHANNELLEN") == 0) {
			s->params.chanlen = atoi(value);
			irc_log_info("server %s: channel name limit: %u",
			    s->name, s->params.chanlen);
		} else if (strcmp(key, "NICKLEN") == 0) {
			s->params.nicklen = atoi(value);
			irc_log_info("server %s: nickname limit:     %u",
			    s->name, s->params.nicklen);
		} else if (strcmp(key, "TOPICLEN") == 0) {
			s->params.topiclen = atoi(value);
			irc_log_info("server %s: topic limit:        %u",
			    s->name, s->params.topiclen);
		} else if (strcmp(key, "AWAYLEN") == 0) {
			s->params.awaylen = atoi(value);
			irc_log_info("server %s: away message limit: %u",
			    s->name, s->params.awaylen);
		} else if (strcmp(key, "KICKLEN") == 0) {
			s->params.kicklen = atoi(value);
			irc_log_info("server %s: kick reason limit:  %u",
			    s->name, s->params.kicklen);
		}
		else if (strcmp(key, "CHARSET") == 0) {
			strlcpy(s->params.charset, value, sizeof (s->params.charset));
			irc_log_info("server %s: charset:            %s",
			    s->name, s->params.charset);
		} else if (strcmp(key, "CASEMAPPING") == 0) {
			strlcpy(s->params.casemapping, value, sizeof (s->params.casemapping));
			irc_log_info("server %s: case mapping:       %s",
			    s->name, s->params.casemapping);
		}
	}
}

static void
handle_invite(struct irc_server *s, struct irc_event *ev, struct irc_conn_msg *msg)
{
	ev->type = IRC_EVENT_INVITE;
	ev->invite.origin = strdup(msg->prefix);
	ev->invite.channel = strdup(msg->args[1]);

	if (s->flags & IRC_SERVER_FLAGS_JOIN_INVITE) {
		irc_server_join(s, ev->invite.channel, NULL);
		irc_log_info("server %s: joining %s on invite", s->name, ev->invite.channel);
	}
}

static void
handle_join(struct irc_server *s, struct irc_event *ev, struct irc_conn_msg *msg)
{
	ev->type = IRC_EVENT_JOIN;
	ev->join.origin = strdup(msg->prefix);
	ev->join.channel = strdup(msg->args[0]);

	add_channel(s, ev->join.channel, NULL, 1);

	if (is_self(s, ev->join.origin))
		irc_log_info("server %s: joined channel %s", s->name, ev->join.channel);
}

static void
handle_kick(struct irc_server *s, struct irc_event *ev, struct irc_conn_msg *msg)
{
	ev->type = IRC_EVENT_KICK;
	ev->kick.origin = strdup(msg->prefix);
	ev->kick.channel = strdup(msg->args[0]);
	ev->kick.target = strdup(msg->args[1]);
	ev->kick.reason = msg->args[2] ? strdup(msg->args[2]) : NULL;

	struct irc_channel *ch = add_channel(s, ev->kick.channel, NULL, 1);

	/*
	 * If the bot was kicked itself mark the channel as not joined and
	 * rejoin it automatically if the option is set.
	 */
	if (is_self(s, ev->kick.target)) {
		ch->joined = 0;
		irc_channel_clear(ch);

		if (s->flags & IRC_SERVER_FLAGS_AUTO_REJOIN) {
			irc_server_join(s, ch->name, ch->password);
			irc_log_info("server %s: rejoining %s after kick", s->name, ch->name);
		}
	} else
		irc_channel_remove(ch, ev->kick.target);
}

static void
handle_mode(struct irc_server *s, struct irc_event *ev, struct irc_conn_msg *msg)
{
	(void)s;
	(void)ev;
	(void)msg;

	int action = 0, mode;
	size_t nelem = 0, argindex = 2;
	struct irc_channel *ch;
	struct irc_channel_user *u;

	ev->type = IRC_EVENT_MODE;
	ev->mode.origin = irc_util_strdup(msg->prefix);
	ev->mode.channel = irc_util_strdup(msg->args[0]);
	ev->mode.mode = irc_util_strdup(msg->args[1]);

	/* Create a NULL-sentineled list of arguments. */
	for (size_t i = 2; i < IRC_ARGS_MAX && msg->args[i]; ++i) {
		ev->mode.args = irc_util_reallocarray(ev->mode.args, nelem + 1, sizeof (char *));
		ev->mode.args[nelem++] = irc_util_strdup(msg->args[i]);
	}

	/* Add the NULL sentinel. */
	ev->mode.args = irc_util_reallocarray(ev->mode.args, nelem + 1, sizeof (char *));
	ev->mode.args[nelem] = NULL;

	if (!(ch = irc_server_find(s, ev->mode.channel)))
		return;

	for (const char *p = ev->mode.mode; *p; ++p) {
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
		if ((mode = find_mode(s, *p)) == 0) {
			++argindex;
			continue;
		}
		if (!msg->args[argindex] || !(u = irc_channel_find(ch, msg->args[argindex]))) {
			++argindex;
			continue;
		}

		++argindex;

		if (action == '+')
			u->modes |= (1 << mode);
		else
			u->modes &= ~(1 << mode);
	}
}

static void
handle_part(struct irc_server *s, struct irc_event *ev, struct irc_conn_msg *msg)
{
	struct irc_channel *ch;

	ev->type = IRC_EVENT_PART;
	ev->part.origin = strdup(msg->prefix);
	ev->part.channel = strdup(msg->args[0]);
	ev->part.reason = msg->args[1] ? strdup(msg->args[1]) : NULL;

	ch = add_channel(s, ev->part.channel, NULL, 1);

	if (is_self(s, ev->part.origin)) {
		remove_channel(ch);
		irc_log_info("server %s: leaving channel %s", s->name, ev->part.channel);
	} else
		irc_channel_remove(ch, ev->part.origin);
}

static void
handle_msg(struct irc_server *s, struct irc_event *ev, struct irc_conn_msg *msg)
{
	(void)s;

	ev->message.origin = strdup(msg->prefix);
	ev->message.channel = strdup(msg->args[0]);

	/*
	 * Detect CTCP commands which are PRIVMSG with a special boundaries.
	 *
	 * Example:
	 * PRIVMSG jean :\001ACTION I'm eating\001.
	 */
	if (is_ctcp(msg->args[1])) {
		ev->type = IRC_EVENT_ME;
		ev->message.message = strdup(ctcp(msg->args[1]));
	} else {
		ev->type = IRC_EVENT_MESSAGE;
		ev->message.message = strdup(msg->args[1]);
	}
}

static void
handle_nick(struct irc_server *s, struct irc_event *ev, struct irc_conn_msg *msg)
{
	ev->type = IRC_EVENT_NICK;
	ev->nick.origin = strdup(msg->prefix);
	ev->nick.nickname = strdup(msg->args[0]);

	/* Update nickname if it is myself. */
	if (is_self(s, ev->nick.origin) == 0) {
		irc_log_info("server %s: nick change %s -> %s", s->name,
		    s->ident.nickname, ev->nick.nickname);
		strlcpy(s->ident.nickname, ev->nick.nickname, sizeof (s->ident.nickname));
	}
}

static void
handle_notice(struct irc_server *s, struct irc_event *ev, struct irc_conn_msg *msg)
{
	(void)s;

	ev->type = IRC_EVENT_NOTICE;
	ev->notice.origin = strdup(msg->prefix);
	ev->notice.channel = strdup(msg->args[0]);
	ev->notice.notice = strdup(msg->args[1]);
}

static void
handle_topic(struct irc_server *s, struct irc_event *ev, struct irc_conn_msg *msg)
{
	(void)s;

	ev->type = IRC_EVENT_TOPIC;
	ev->topic.origin = strdup(msg->prefix);
	ev->topic.channel = strdup(msg->args[0]);
	ev->topic.topic = strdup(msg->args[1]);
}

static void
handle_ping(struct irc_server *s, struct irc_event *ev, struct irc_conn_msg *msg)
{
	(void)ev;
	(void)msg;

	irc_server_send(s, "PONG %s", msg->args[1]);
}

static void
handle_names(struct irc_server *s, struct irc_event *ev, struct irc_conn_msg *msg)
{
	(void)ev;

	struct irc_channel *ch;
	char *p, *token;
	int modes = 0;

	ch = add_channel(s, msg->args[2], NULL, 1);

	/* Track existing nicknames into the given channel. */
	for (p = msg->args[3]; (token = strtok_r(p, " ", &p)); ) {
		if (strlen(token) == 0)
			continue;

		modes = irc_server_strip(s, (const char **)&token);
		irc_channel_add(ch, token, modes);
	}
}

static void
handle_endofnames(struct irc_server *s, struct irc_event *ev, struct irc_conn_msg *msg)
{
	FILE *fp;
	size_t length;
	const struct irc_channel *ch;
	const struct irc_channel_user *u;

	ev->type = IRC_EVENT_NAMES;
	ev->names.channel = irc_util_strdup(msg->args[1]);

	/* Construct a string list for every user in the channel. */
	ch = irc_server_find(s, ev->names.channel);

	if (!(fp = open_memstream(&ev->names.names, &length)))
		err(1, "open_memstream");

	LIST_FOREACH(u, &ch->users, link) {
		for (size_t i = 0; i < IRC_UTIL_SIZE(s->params.prefixes); ++i)
			if (u->modes & (1 << i))
				fprintf(fp, "%c", s->params.prefixes[i].symbol);

		fprintf(fp, "%s", u->nickname);

		if (LIST_NEXT(u, link))
			fputc(' ', fp);
	}

	fclose(fp);
}

static void
handle_nicknameinuse(struct irc_server *s, struct irc_event *ev, struct irc_conn_msg *msg)
{
	(void)msg;
	(void)ev;

	irc_log_warn("server %s: nickname %s is already in use", s->name, s->ident.nickname);
	fail(s);
}

static void
handle_error(struct irc_server *s, struct irc_event *ev, struct irc_conn_msg *msg)
{
	ev->type = IRC_EVENT_DISCONNECT;

	if (msg->args[0])
		irc_log_warn("server %s: %s", s->name, msg->args[0]);

	fail(s);
}

static void
handle_whoisuser(struct irc_server *s, struct irc_event *ev, struct irc_conn_msg *msg)
{
	(void)s;
	(void)ev;
	(void)msg;

	s->bufwhois.nickname = strdup(msg->args[1]);
	s->bufwhois.username = strdup(msg->args[2]);
	s->bufwhois.hostname = strdup(msg->args[3]);
	s->bufwhois.realname = strdup(msg->args[5]);
}

static void
handle_whoischannels(struct irc_server *s, struct irc_event *ev, struct irc_conn_msg *msg)
{
	(void)ev;

	char *token, *p;
	int modes;

	if (!msg->args[2])
		return;

	for (p = msg->args[2]; (token = strtok_r(p, " ", &p)); ) {
		modes = irc_server_strip(s, (const char **)&token);
		s->bufwhois.channels = irc_util_reallocarray(
		    s->bufwhois.channels,
		    s->bufwhois.channelsz + 1,
		    sizeof (*s->bufwhois.channels)
		);

		s->bufwhois.channels[s->bufwhois.channelsz].name = irc_util_strdup(token);
		s->bufwhois.channels[s->bufwhois.channelsz++].modes = modes;
	}
}

static void
handle_endofwhois(struct irc_server *s, struct irc_event *ev, struct irc_conn_msg *msg)
{
	(void)msg;

	ev->type = IRC_EVENT_WHOIS;
	ev->whois = s->bufwhois;

	memset(&s->bufwhois, 0, sizeof (s->bufwhois));
}

static const struct handler {
	const char *command;
	void (*handle)(struct irc_server *, struct irc_event *, struct irc_conn_msg *);
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
cmp_handler(const char *name, const struct handler *handler)
{
	return strcmp(name, handler->command);
}

static inline struct handler *
find_handler(const char *name)
{
	return bsearch(name, handlers, IRC_UTIL_SIZE(handlers), sizeof (struct handler),
	    (irc_cmp)(cmp_handler));
}

static void
handle(struct irc_server *s, struct irc_event *ev, struct irc_conn_msg *msg)
{
	const struct handler *h;

	/* Update last message time to detect non-notified disconnection. */
	s->last_tp = time(NULL);

	if (!(h = find_handler(msg->cmd)))
		return;

	memset(ev, 0, sizeof (*ev));

	ev->server = s;
	h->handle(s, ev, msg);
}

static void
auth(struct irc_server *s)
{
	s->state = IRC_SERVER_STATE_CONNECTED;

	/*
	 * Use multi-prefix extension to keep track of all combined "modes" in
	 * a channel.
	 *
	 * https://ircv3.net/specs/extensions/multi-prefix-3.1.html
	 */
	irc_server_send(s, "CAP REQ :multi-prefix");

	if (s->ident.password[0])
		irc_server_send(s, "PASS %s", s->ident.password);

	irc_server_send(s, "NICK %s", s->ident.nickname);
	irc_server_send(s, "USER %s %s %s :%s", s->ident.username,
	    s->ident.username, s->ident.username, s->ident.realname);
	irc_server_send(s, "CAP END");
}

struct irc_server *
irc_server_new(const char *name,
               const char *nickname,
               const char *username,
               const char *realname,
               const char *hostname,
               unsigned int port)
{
	assert(name);
	assert(nickname);
	assert(username);
	assert(realname);
	assert(hostname);

	struct irc_server *s;

	/* Connection. */
	s = irc_util_calloc(1, sizeof (*s));

	/* Hide implementation to get rid of OpenSSL headers in public API. */
	s->conn = irc_util_calloc(1, sizeof (*s->conn));
	s->conn->port = port;
	strlcpy(s->conn->hostname, hostname, sizeof (s->conn->hostname));

	/* Identity. */
	strlcpy(s->ident.nickname, nickname, sizeof (s->ident.nickname));
	strlcpy(s->ident.username, username, sizeof (s->ident.username));
	strlcpy(s->ident.realname, realname, sizeof (s->ident.realname));

	/* Server itself. */
	strlcpy(s->name, name, sizeof (s->name));
	LIST_INIT(&s->channels);

	/* Default options. */
	strlcpy(s->prefix, "!", sizeof (s->prefix));

	return s;
}

void
irc_server_connect(struct irc_server *s)
{
	assert(s);

	if (s->flags & IRC_SERVER_FLAGS_SSL)
		s->conn->flags |= IRC_CONN_SSL;

	s->conn->sv = s;

	if (irc_conn_connect(s->conn) < 0)
		fail(s);
	else
		s->state = IRC_SERVER_STATE_CONNECTING;

	/*
	 * Assume the last time we received a message was now, so that
	 * irc_server_flush don't think the server is already dead while we
	 * didn't have any answer from it.
	 */
	s->last_tp = time(NULL);
}

void
irc_server_disconnect(struct irc_server *s)
{
	assert(s);

	s->state = IRC_SERVER_STATE_DISCONNECTED;

	clear_channels(s, 0);
	clear_server(s);
}

void
irc_server_prepare(const struct irc_server *s, struct pollfd *pfd)
{
	assert(s);
	assert(pfd);

	irc_conn_prepare(s->conn, pfd);
}

void
irc_server_flush(struct irc_server *s, const struct pollfd *pfd)
{
	assert(s);
	assert(pfd);

	switch (s->state) {
	case IRC_SERVER_STATE_WAITING:
		if (difftime(time(NULL), s->lost_tp) >= DELAY)
			irc_server_connect(s);
		break;
	case IRC_SERVER_STATE_CONNECTED:
		if (difftime(time(NULL), s->last_tp) >= TIMEOUT) {
			irc_log_warn("server %s: no message in more than %u seconds", s->name, TIMEOUT);
			fail(s);
		} else if (irc_conn_flush(s->conn, pfd) < 0) {
			irc_log_warn("server %s: %s", s->name, strerror(errno));
			return fail(s);
		}
		break;
	case IRC_SERVER_STATE_CONNECTING:
		/*
		 * Now the conn object is ready which means the server has
		 * to authenticate.
		 */
		auth(s);
		break;
	default:
		break;
	}
}

int
irc_server_poll(struct irc_server *s, struct irc_event *ev)
{
	assert(s);
	assert(ev);

	struct irc_conn_msg msg = {0};

	/*
	 * When the server gets disconnected, the state changes to
	 * IRC_SERVER_STATE_DISCONNECTED which notifies the caller with the
	 * appropriate event. Then to avoid returning this same event each time
	 * this function is called again, we immediately change the state to
	 * something else.
	 */
	if (s->state == IRC_SERVER_STATE_DISCONNECTED) {
		handle_disconnect(s, ev);
		s->state = IRC_SERVER_STATE_NONE;
		return 1;
	}

	if (irc_conn_poll(s->conn, &msg))
		return handle(s, ev, &msg), 1;

	return 0;
}

struct irc_channel *
irc_server_find(struct irc_server *s, const char *name)
{
	assert(s);
	assert(name);

	struct irc_channel *ch;

	LIST_FOREACH(ch, &s->channels, link)
		if (strcmp(ch->name, name) == 0)
			return ch;

	return NULL;
}

int
irc_server_send(struct irc_server *s, const char *fmt, ...)
{
	assert(s);
	assert(fmt);

	char buf[IRC_BUF_LEN];
	va_list ap;

	if (s->state != IRC_SERVER_STATE_CONNECTED) {
		errno = ENOTCONN;
		return -1;
	}

	va_start(ap, fmt);
	vsnprintf(buf, sizeof (buf), fmt, ap);
	va_end(ap);

	return irc_conn_send(s->conn, buf);
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

	struct irc_channel *ch;
	int ret = 1;

	/*
	 * Search if there is already a channel pending or joined. If the
	 * server is connected we send a join command otherwise we put it there
	 * and wait for connection.
	 */
	if (!(ch = irc_server_find(s, name)))
		ch = add_channel(s, name, pass, 0);

	if (!ch->joined && s->state == IRC_SERVER_STATE_CONNECTED) {
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
irc_server_mode(struct irc_server *s,
                const char *channel,
                const char *mode,
                const char *limit,
                const char *user,
                const char *mask)
{
	assert(s);
	assert(channel);
	assert(mode);

	return irc_server_send(s, "MODE %s %s %s %s %s", channel, mode,
	    limit ? limit : "",
	    user ? user : "",
	    mask ? mask : "");
}

int
irc_server_names(struct irc_server *s, const char *channel)
{
	return irc_server_send(s, "NAMES %s", channel);
}

int
irc_server_nick(struct irc_server *s, const char *nick)
{
	assert(s);
	assert(nick);

	if (s->state <= IRC_SERVER_STATE_DISCONNECTED) {
		strlcpy(s->ident.nickname, nick, sizeof (s->ident.nickname));
		return 1;
	}

	return irc_server_send(s, "NICK %s", nick);
}

int
irc_server_notice(struct irc_server *s, const char *channel, const char *message)
{
	assert(s);
	assert(channel);
	assert(message);

	return irc_server_send(s, "NOTICE %s: %s", channel, message);
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

	for (size_t i = 0; i < IRC_UTIL_SIZE(s->params.prefixes); ++i) {
		if (**what == s->params.prefixes[i].symbol) {
			modes |= 1 << i;
			*what += 1;
		}
	}

	return modes;
}

void
irc_server_split(const char *prefix, struct irc_server_user *user)
{
	assert(prefix);
	assert(user);

	char fmt[128];

	memset(user, 0, sizeof (*user));
	snprintf(fmt, sizeof (fmt), "%%%zu[^!]!%%%zu[^@]@%%%zus",
	    sizeof (user->nickname) - 1,
	    sizeof (user->username) - 1,
	    sizeof (user->host) - 1);
	sscanf(prefix, fmt, user->nickname, user->username, user->host);
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

	if (--s->refc == 0) {
		clear_channels(s, 1);
		free(s);
		free(s->conn);
	}
}
