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

#include <assert.h>
#include <errno.h>
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
#include "event.h"
#include "log.h"
#include "server.h"
#include "util.h"

static inline void
clear_channels(struct irc_server *s, bool free)
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
	free(s->bufwhois.nickname);
	free(s->bufwhois.username);
	free(s->bufwhois.realname);
	free(s->bufwhois.hostname);
	free(s->bufwhois.channels);

	memset(&s->params, 0, sizeof (s->params));
	memset(&s->bufwhois, 0, sizeof (s->bufwhois));
}

static inline bool
is_self(const struct irc_server *s, const char *nick)
{
	return strncmp(s->ident.nickname, nick, strlen(s->ident.nickname)) == 0;
}

#if 0

struct origin {
	char nickname[IRC_NICKNAME_LEN];
	char username[IRC_USERNAME_LEN];
	char host[IRC_HOST_LEN];
};

static const struct origin *
parse_origin(const char *prefix)
{
	static struct origin origin;
	char fmt[128];

	memset(&origin, 0, sizeof (origin));
	snprintf(fmt, sizeof (fmt), "%%%zu[^!]!%%%zu[^@]@%%%zus",
	    sizeof (origin.nickname) - 1,
	    sizeof (origin.username) - 1,
	    sizeof (origin.host) - 1);
	sscanf(prefix, fmt, origin.nickname, origin.username, origin.host);

	return &origin;
}

#endif

static void
add_nick(const struct irc_server *s, struct irc_channel *ch, const char *nick)
{
	char mode = 0, prefix = 0;

	irc_server_strip(s, &nick, &mode, &prefix);
	irc_channel_add(ch, nick, mode, prefix);
}

static struct irc_channel *
add_channel(struct irc_server *s, const char *name, const char *password, bool joined)
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
	irc_channel_finish(ch);
	LIST_REMOVE(ch, link);
}

bool
is_ctcp(const char *line)
{
	size_t length;

	if (!line)
		return false;
	if ((length = strlen(line)) < 2)
		return false;

	return line[0] == 0x1 && line[length - 1] == 0x1;
}

char *
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
		char *tk = tokens;

		for (size_t i = 0; i < IRC_UTIL_SIZE(s->params.prefixes) && *pm && *tk; ++i) {
			s->params.prefixes[i].mode = *pm++;
			s->params.prefixes[i].token = *tk++;
		}
	}
}

static void
read_support_chantypes(struct irc_server *s, const char *value)
{
	strlcpy(s->params.chantypes, value, sizeof (s->params.chantypes));
}

static void
handle_connect(struct irc_server *s, struct irc_event *ev, struct irc_conn_msg *msg)
{
	(void)msg;

	struct irc_channel *ch;

	s->state = IRC_SERVER_STATE_CONNECTED;

	/* Now join all channels that were requested. */
	LIST_FOREACH(ch, &s->channels, link)
		irc_server_join(s, ch->name, ch->password);

	ev->type = IRC_EVENT_CONNECT;
}

static void
handle_disconnect(struct irc_server *s, struct irc_event *ev)
{
	s->state = IRC_SERVER_STATE_NONE;
	ev->type = IRC_EVENT_DISCONNECT;
	ev->server = s;
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

		if (strcmp(key, "PREFIX") == 0)
			read_support_prefix(s, value);
		if (strcmp(key, "CHANTYPES") == 0)
			read_support_chantypes(s, value);
	}
}

static void
handle_invite(struct irc_server *s, struct irc_event *ev, struct irc_conn_msg *msg)
{
	ev->type = IRC_EVENT_INVITE;
	ev->invite.origin = strdup(msg->args[0]);
	ev->invite.channel = strdup(msg->args[1]);
	ev->invite.target = strdup(msg->args[2]);

	if (is_self(s, ev->invite.target) && s->flags & IRC_SERVER_FLAGS_JOIN_INVITE)
		irc_server_join(s, ev->invite.channel, NULL);
}

static void
handle_join(struct irc_server *s, struct irc_event *ev, struct irc_conn_msg *msg)
{
	ev->type = IRC_EVENT_JOIN;
	ev->join.origin = strdup(msg->prefix);
	ev->join.channel = strdup(msg->args[0]);

	add_channel(s, ev->join.channel, NULL, true);
}

static void
handle_kick(struct irc_server *s, struct irc_event *ev, struct irc_conn_msg *msg)
{
	ev->type = IRC_EVENT_KICK;
	ev->kick.origin = strdup(msg->prefix);
	ev->kick.channel = strdup(msg->args[0]);
	ev->kick.target = strdup(msg->args[1]);
	ev->kick.reason = msg->args[2] ? strdup(msg->args[2]) : NULL;

	struct irc_channel *ch = add_channel(s, ev->kick.channel, NULL, true);

	/*
	 * If the bot was kicked itself mark the channel as not joined and
	 * rejoin it automatically if the option is set.
	 */
	if (is_self(s, ev->kick.target) == 0) {
		ch->joined = false;
		irc_channel_clear(ch);

		if (s->flags & IRC_SERVER_FLAGS_AUTO_REJOIN)
			irc_server_join(s, ch->name, ch->password);
	} else
		irc_channel_remove(ch, ev->kick.target);
}

static void
handle_mode(struct irc_server *s, struct irc_event *ev, struct irc_conn_msg *msg)
{
	(void)s;
	(void)ev;
	(void)msg;

	ev->type = IRC_EVENT_MODE;
	ev->mode.origin = strdup(msg->prefix);
	ev->mode.channel = strdup(msg->args[0]);
	ev->mode.mode = strdup(msg->args[1]);
	ev->mode.limit = msg->args[2] ? strdup(msg->args[2]) : NULL;
	ev->mode.user = msg->args[3] ? strdup(msg->args[3]) : NULL;
	ev->mode.mask = msg->args[4] ? strdup(msg->args[4]) : NULL;

	/* TODO: update nickname modes. */
}

static void
handle_part(struct irc_server *s, struct irc_event *ev, struct irc_conn_msg *msg)
{
	struct irc_channel *ch;

	ev->type = IRC_EVENT_PART;
	ev->part.origin = strdup(msg->prefix);
	ev->part.channel = strdup(msg->args[0]);
	ev->part.reason = msg->args[1] ? strdup(msg->args[1]) : NULL;

	ch = add_channel(s, ev->part.channel, NULL, true);

	if (is_self(s, ev->part.origin) == 0)
		remove_channel(ch);
	else
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
	if (is_self(s, ev->nick.origin) == 0)
		strlcpy(s->ident.nickname, ev->nick.nickname, sizeof (s->ident.nickname));
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
	(void)s;
	(void)ev;
	(void)msg;

	irc_server_send(s, "PONG %s", msg->args[1]);
}

static void
handle_names(struct irc_server *s, struct irc_event *ev, struct irc_conn_msg *msg)
{
	(void)s;
	(void)ev;
	(void)msg;

	struct irc_channel *ch;
	char *p, *token;

	ch = add_channel(s, msg->args[2], NULL, true);

	/* Track existing nicknames into the given channel. */
	for (p = msg->args[3]; (token = strtok_r(p, " ", &p)); )
		if (strlen(token) > 0)
			add_nick(s, ch, token);
}

static void
handle_endofnames(struct irc_server *s, struct irc_event *ev, struct irc_conn_msg *msg)
{
	(void)s;
	(void)ev;
	(void)msg;

	FILE *fp;
	size_t length;
	const struct irc_channel *ch;
	const struct irc_channel_user *u;

	ev->type = IRC_EVENT_NAMES;
	ev->names.channel = strdup(msg->args[1]);

	/* Construct a string list for every user in the channel. */
	ch = irc_server_find(s, ev->names.channel);
	fp = open_memstream(&ev->names.names, &length);

	LIST_FOREACH(u, &ch->users, link) {
		if (u->symbol)
			fprintf(fp, "%c", u->symbol);

		fprintf(fp, "%s", u->nickname);

		if (LIST_NEXT(u, link))
			fputc(' ', fp);
	}

	fclose(fp);
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

	size_t curlen, reqlen;

	curlen = s->bufwhois.channels ? strlen(s->bufwhois.channels) : 0;
	reqlen = strlen(msg->args[2]);

	/*
	 * If there is already something, add a space at the end of the current
	 * buffer.
	 */
	if (curlen > 0)
		reqlen++;

	/* Now, don't forget */
	s->bufwhois.channels = irc_util_realloc(s->bufwhois.channels, reqlen + 1);

	if (curlen > 0) {
		strcat(s->bufwhois.channels, " ");
		strcat(s->bufwhois.channels, msg->args[2]);
	} else
		strcpy(s->bufwhois.channels, msg->args[2]);
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

	if (s->ident.password[0])
		irc_server_send(s, "PASS %s", s->ident.password);

	irc_server_send(s, "USER %s %s %s :%s", s->ident.username,
	    s->ident.username, s->ident.username, s->ident.realname);
	irc_server_send(s, "NICK %s", s->ident.nickname);
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

	s = irc_util_calloc(1, sizeof (*s));

	/* Connection. */
	s->conn.port = port;
	strlcpy(s->conn.hostname, hostname, sizeof (s->conn.hostname));

	/* Identity. */
	strlcpy(s->ident.nickname, nickname, sizeof (s->ident.nickname));
	strlcpy(s->ident.username, username, sizeof (s->ident.username));
	strlcpy(s->ident.realname, realname, sizeof (s->ident.realname));

	/* Server itslf. */
	strlcpy(s->name, name, sizeof (s->name));
	LIST_INIT(&s->channels);

	return s;
}

void
irc_server_connect(struct irc_server *s)
{
	assert(s);

	if (irc_conn_connect(&s->conn))
		s->state = IRC_SERVER_STATE_CONNECTING;
	else
		s->state = IRC_SERVER_STATE_DISCONNECTED;
}

void
irc_server_disconnect(struct irc_server *s)
{
	assert(s);

	s->state = IRC_SERVER_STATE_DISCONNECTED;

	irc_conn_disconnect(&s->conn);

	clear_channels(s, false);
	clear_server(s);
}

void
irc_server_prepare(const struct irc_server *s, struct pollfd *pfd)
{
	assert(s);
	assert(pfd);

	irc_conn_prepare(&s->conn, pfd);
}

void
irc_server_flush(struct irc_server *s, const struct pollfd *pfd)
{
	assert(s);
	assert(pfd);

	if (!irc_conn_flush(&s->conn, pfd))
		return irc_server_disconnect(s);
	if (s->conn.state != IRC_CONN_STATE_READY)
		return;

	switch (s->state) {
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

bool
irc_server_poll(struct irc_server *s, struct irc_event *ev)
{
	assert(s);
	assert(ev);

	struct irc_conn_msg msg = {0};

	if (irc_conn_poll(&s->conn, &msg))
		return handle(s, ev, &msg), true;
	if (s->state == IRC_SERVER_STATE_DISCONNECTED)
		return handle_disconnect(s, ev), true;

	return false;
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

bool
irc_server_send(struct irc_server *s, const char *fmt, ...)
{
	assert(s);
	assert(fmt);

	char buf[IRC_BUF_LEN];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof (buf), fmt, ap);
	va_end(ap);

	return irc_conn_send(&s->conn, buf);
}

bool
irc_server_invite(struct irc_server *s, const char *channel, const char *target)
{
	assert(s);
	assert(channel);
	assert(target);

	return irc_server_send(s, "INVITE %s %s", target, channel);
}

bool
irc_server_join(struct irc_server *s, const char *name, const char *pass)
{
	assert(s);
	assert(name);

	struct irc_channel *ch;
	bool ret = true;

	/*
	 * Search if there is already a channel pending or joined. If the
	 * server is connected we send a join command otherwise we put it there
	 * and wait for connection.
	 */
	if (!(ch = irc_server_find(s, name)))
		ch = add_channel(s, name, pass, false);

	if (!ch->joined && s->state == IRC_SERVER_STATE_CONNECTED) {
		if (pass)
			ret = irc_server_send(s, "JOIN %s %s", name, pass);
		else
			ret = irc_server_send(s, "JOIN %s", name);
	}

	return ret;
}

bool
irc_server_kick(struct irc_server *s, const char *channel, const char *target, const char *reason)
{
	assert(s);
	assert(channel);
	assert(target);

	bool ret;

	if (reason)
		ret = irc_server_send(s, "KICK %s %s :%s", channel, target, reason);
	else
		ret = irc_server_send(s, "KICK %s %s", channel, target);

	return ret;
}

bool
irc_server_part(struct irc_server *s, const char *channel, const char *reason)
{
	assert(s);
	assert(channel);

	bool ret;

	if (reason && strlen(reason) > 0)
		ret = irc_server_send(s, "PART %s :%s", channel, reason);
	else
		ret = irc_server_send(s, "PART %s", channel);

	return ret;
}

bool
irc_server_topic(struct irc_server *s, const char *channel, const char *topic)
{
	assert(s);
	assert(channel);
	assert(topic);

	return irc_server_send(s, "TOPIC %s :%s", channel, topic);
}

bool
irc_server_message(struct irc_server *s, const char *chan, const char *msg)
{
	assert(s);
	assert(chan);
	assert(msg);

	return irc_server_send(s, "PRIVMSG %s :%s", chan, msg);
}

bool
irc_server_me(struct irc_server *s, const char *chan, const char *message)
{
	assert(s);
	assert(chan);
	assert(message);

	return irc_server_send(s, "PRIVMSG %s :\001ACTION %s\001", chan, message);
}

bool
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

bool
irc_server_names(struct irc_server *s, const char *channel)
{
	return irc_server_send(s, "NAMES %s", channel);
}

bool
irc_server_nick(struct irc_server *s, const char *nick)
{
	assert(s);
	assert(nick);

	if (s->state <= IRC_SERVER_STATE_DISCONNECTED) {
		strlcpy(s->ident.nickname, nick, sizeof (s->ident.nickname));
		return true;
	}

	return irc_server_send(s, "NICK %s", nick);
}

bool
irc_server_notice(struct irc_server *s, const char *channel, const char *message)
{
	assert(s);
	assert(channel);
	assert(message);

	return irc_server_send(s, "NOTICE %s: %s", channel, message);
}

bool
irc_server_whois(struct irc_server *s, const char *target)
{
	assert(s);
	assert(target);

	return irc_server_send(s, "WHOIS %s", target);
}

void
irc_server_strip(const struct irc_server *s, const char **nick, char *mode, char *prefix)
{
	assert(s);
	assert(*nick);

	if (mode)
		*mode = 0;
	if (prefix)
		*mode = 0;

	for (size_t i = 0; i < IRC_UTIL_SIZE(s->params.prefixes); ++i) {
		if (**nick == s->params.prefixes[i].token) {
			*mode = s->params.prefixes[i].mode;
			*prefix = s->params.prefixes[i].token;
			*nick += 1;
			break;
		}
	}
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
		clear_channels(s, true);
		free(s);
	}
}
