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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>

#include "config.h"

#include <utlist.h>

#include <nce/nce.h>

#include "channel.h"
#include "conn.h"
#include "irccd.h"
#include "log.h"
#include "server.h"
#include "util.h"

#define DEBUG(...) LOG(irc_log_debug, __VA_ARGS__)
#define INFO(...)  LOG(irc_log_info, __VA_ARGS__)
#define WARN(...)  LOG(irc_log_warn, __VA_ARGS__)

#define LOG(Fn, ...)                                                            \
do {                                                                            \
        char line[256] = {};                                                    \
                                                                                \
        snprintf(line, sizeof (line), __VA_ARGS__);                             \
                                                                                \
        Fn("server %s: %s", server->name, line);                                \
} while (0)

/*
 * Object wrapping the conn object and its associated coroutine.
 */
struct irc_server_coro {
	struct conn conn;
	struct nce_coro consumer;
};

/*
 * Tell if the nickname targets the bot itself.
 */
static inline int
irc_server_is_self(const struct irc_server *server, const char *nickname)
{
	return strncasecmp(server->nickname, nickname, strlen(server->nickname)) == 0;
}

/*
 * Parse the IRC prefixes protocol in the form `(abcde)!@#$%`.
 */
static void
irc_server_modes_parse(struct irc_server *server, const char *value)
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
irc_server_modes_find(struct irc_server *server, int mode)
{
	for (size_t i = 0; i < server->prefixesz; ++i)
		if (server->prefixes[i].mode == mode)
			return i;

	return 0;
}

static struct irc_channel *
irc_server_channels_add(struct irc_server *server,
                        const char *name,
                        const char *password,
                        enum irc_channel_flags flags)
{
	struct irc_channel *ch;

	ch = (struct irc_channel *)irc_server_channels_find(server, name);

	if (ch)
		ch->flags |= flags;
	else {
		ch = irc_channel_new(name, password, flags);
		LL_PREPEND(server->channels, ch);
	}

	return ch;
}

/*
 * Remove the channel from the server.
 *
 * Explicitly allow ch to be NULL for convenience.
 */
static inline void
irc_server_channels_remove(struct irc_server *server, struct irc_channel *ch)
{
	if (ch) {
		LL_DELETE(server->channels, ch);
		irc_channel_free(ch);
	}
}

/*
 * Clear the server channels, meaning the joining state get's reset and
 * nickname list emptied but they stay in the server to be joined later on
 */
static inline void
irc_server_channels_clear(struct irc_server *server)
{
	struct irc_channel *c;

	LL_FOREACH(server->channels, c)
		irc_channel_clear(c);
}

/*
 * Free all channels and clear the linked list.
 */
static inline void
irc_server_channels_free(struct irc_server *server)
{
	struct irc_channel *c, *tmp;

	LL_FOREACH_SAFE(server->channels, c, tmp)
		irc_channel_free(c);

	server->channels = NULL;
}

static void
irc_server_clear(struct irc_server *server)
{
	server->chantypes   = irc_util_free(server->chantypes);
	server->charset     = irc_util_free(server->charset);
	server->casemapping = irc_util_free(server->casemapping);

	server->channel_max  = 0;
	server->nickname_max = 0;
	server->topic_max    = 0;
	server->away_max     = 0;
	server->kick_max     = 0;

	server->prefixes  = irc_util_free(server->prefixes);
	server->prefixesz = 0;
}

static void
irc_server_free(struct irc_server *s)
{
	irc_server_disconnect(s);

	irc_server_clear(s);
	irc_server_channels_free(s);

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

static void
irc_server_handle_connect(struct irc_server *server, struct conn_msg *)
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
irc_server_handle_disconnect(struct irc_server *server, struct conn_msg *)
{
	struct irc_event ev = {};

	ev.type = IRC_EVENT_DISCONNECT;
	ev.server = server;

	irc_bot_dispatch(&ev);
}

static void
irc_server_handle_support(struct irc_server *server, struct conn_msg *msg)
{
	char key[64];
	char value[64];

	for (size_t i = 0; i < msg->argsz && msg->args[i]; ++i) {
		if (sscanf(msg->args[i], "%63[^=]=%63s", key, value) != 2)
			continue;

		if (strcmp(key, "PREFIX") == 0) {
			irc_server_modes_parse(server, value);
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
irc_server_handle_invite(struct irc_server *server, struct conn_msg *msg)
{
	struct irc_event ev = {};

	ev.type = IRC_EVENT_INVITE;
	ev.server = server;
	ev.invite.origin = irc_util_strdup(msg->prefix);
	ev.invite.channel = irc_util_strdup(msg->args[1]);

	if (server->flags & IRC_SERVER_FLAGS_JOIN_INVITE) {
		INFO("joining %s on invite", ev.invite.channel);
		irc_server_join(server, ev.invite.channel, NULL);
	}

	irc_bot_dispatch(&ev);
}

static void
irc_server_handle_join(struct irc_server *server, struct conn_msg *msg)
{
	struct irc_event ev = {};

	ev.type = IRC_EVENT_JOIN;
	ev.server = server;
	ev.join.origin = irc_util_strdup(msg->prefix);
	ev.join.channel = irc_util_strdup(msg->args[0]);

	irc_server_channels_add(server, ev.join.channel, NULL, IRC_CHANNEL_FLAGS_JOINED);

	if (irc_server_is_self(server, ev.join.origin))
		INFO("joined channel %s", ev.join.channel);

	irc_bot_dispatch(&ev);
}

static void
irc_server_handle_kick(struct irc_server *server, struct conn_msg *msg)
{
	struct irc_channel *ch;
	struct irc_event ev = {};

	ev.type = IRC_EVENT_KICK;
	ev.server = server;
	ev.kick.origin = irc_util_strdup(msg->prefix);
	ev.kick.channel = irc_util_strdup(msg->args[0]);
	ev.kick.target = irc_util_strdup(msg->args[1]);
	ev.kick.reason = msg->args[2] ? irc_util_strdup(msg->args[2]) : NULL;

	ch = irc_server_channels_add(server, ev.kick.channel, NULL, 1);

	/*
	 * If the bot was kicked itself mark the channel as not joined and
	 * rejoin it automatically if the option is set.
	 */
	if (irc_server_is_self(server, ev.kick.target)) {
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
irc_server_handle_mode(struct irc_server *server, struct conn_msg *msg)
{
	const struct irc_channel_user *u;
	int action = 0, mode;
	size_t nelem = 0, argindex = 2;
	struct irc_channel *ch;
	struct irc_event ev = {};

	ev.type = IRC_EVENT_MODE;
	ev.server = server;
	ev.mode.origin = irc_util_strdup(msg->prefix);
	ev.mode.channel = irc_util_strdup(msg->args[0]);
	ev.mode.mode = irc_util_strdup(msg->args[1]);

	/* Create a NULL-sentineled list of arguments. */
	for (size_t i = 2; i < msg->argsz && msg->args[i]; ++i) {
		ev.mode.args = irc_util_reallocarray(ev.mode.args, nelem + 1, sizeof (char *));
		ev.mode.args[nelem++] = irc_util_strdup(msg->args[i]);
	}

	/* Add the NULL sentinel. */
	ev.mode.args = irc_util_reallocarray(ev.mode.args, nelem + 1, sizeof (char *));
	ev.mode.args[nelem] = NULL;

	ch = (struct irc_channel *)irc_server_channels_find(server, ev.mode.channel);

	if (!ch)
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
		if ((mode = irc_server_modes_find(server, *p)) == 0) {
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
irc_server_handle_part(struct irc_server *server, struct conn_msg *msg)
{
	struct irc_channel *ch;
	struct irc_event ev = {};

	ev.type = IRC_EVENT_PART;
	ev.server = server;
	ev.part.origin = irc_util_strdup(msg->prefix);
	ev.part.channel = irc_util_strdup(msg->args[0]);
	ev.part.reason = msg->args[1] ? irc_util_strdup(msg->args[1]) : NULL;

	ch = (struct irc_channel *)irc_server_channels_find(server, ev.part.channel);

	if (irc_server_is_self(server, ev.part.origin)) {
		INFO("leaving channel %s", ev.part.channel);
		irc_server_channels_remove(server, ch);
	} else
		irc_channel_remove(ch, ev.part.origin);

	irc_bot_dispatch(&ev);
}

static void
irc_server_handle_msg(struct irc_server *server, struct conn_msg *msg)
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
	if (irc__conn_msg_is_ctcp(msg->args[1]) && (user = irc_util_user_split(msg->prefix))) {
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
			ev.message.message = irc_util_strdup(irc__conn_msg_ctcp(msg->args[1]));
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
irc_server_handle_nick(struct irc_server *server, struct conn_msg *msg)
{
	struct irc_event ev = {};

	ev.type = IRC_EVENT_NICK;
	ev.server = server;
	ev.nick.origin = irc_util_strdup(msg->prefix);
	ev.nick.nickname = irc_util_strdup(msg->args[0]);

	/* Update nickname if it is myself. */
	if (irc_server_is_self(server, ev.nick.origin)) {
		INFO("nick change %s -> %s", server->nickname, ev.nick.nickname);
		server->nickname = irc_util_strdupfree(server->nickname, ev.nick.nickname);
	}

	irc_bot_dispatch(&ev);
}

static void
irc_server_handle_notice(struct irc_server *server, struct conn_msg *msg)
{
	struct irc_event ev = {};

	ev.type = IRC_EVENT_NOTICE;
	ev.server = server;
	ev.notice.origin = irc_util_strdup(msg->prefix);
	ev.notice.channel = irc_util_strdup(msg->args[0]);
	ev.notice.notice = irc_util_strdup(msg->args[1]);

	irc_bot_dispatch(&ev);
}

static void
irc_server_handle_topic(struct irc_server *server, struct conn_msg *msg)
{
	struct irc_event ev = {};

	ev.type = IRC_EVENT_TOPIC;
	ev.server = server;
	ev.topic.origin = irc_util_strdup(msg->prefix);
	ev.topic.channel = irc_util_strdup(msg->args[0]);
	ev.topic.topic = irc_util_strdup(msg->args[1]);

	irc_bot_dispatch(&ev);
}

static void
irc_server_handle_ping(struct irc_server *server, struct conn_msg *msg)
{
	if (msg->argsz >= 1)
		irc_server_send(server, "PONG :%s", msg->args[0]);
}

static void
irc_server_handle_names(struct irc_server *server, struct conn_msg *msg)
{
	struct irc_channel *ch;
	char *p, *token;
	int modes = 0;

	ch = irc_server_channels_add(server, msg->args[2], NULL, 1);

	/* Track existing nicknames into the given channel. */
	for (p = msg->args[3]; (token = strtok_r(p, " ", &p)); ) {
		if (strlen(token) == 0)
			continue;

		modes = irc_server_strip(server, (const char **)&token);
		irc_channel_add(ch, token, modes);
	}
}

static void
irc_server_handle_endofnames(struct irc_server *server, struct conn_msg *msg)
{
	const struct irc_channel *ch;
	const struct irc_channel_user *u;
	struct irc_event ev = {};
	size_t i = 0;

	ev.type = IRC_EVENT_NAMES;
	ev.server = server;
	ev.names.channel = irc_util_strdup(msg->args[1]);

	ch = (struct irc_channel *)irc_server_channels_find(server, ev.names.channel);

	if (!ch)
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
irc_server_handle_nicknameinuse(struct irc_server *server, struct conn_msg *)
{
	WARN("nickname %s is already in use", server->nickname);
}

static void
irc_server_handle_error(struct irc_server *server, struct conn_msg *msg)
{
	if (msg->args[0])
		WARN("%s", msg->args[0]);
}

/*
 * 319
 */
static void
irc_server_handle_whoischannels(struct irc_server *server,
                                struct irc_event_whois *ev,
                                struct conn_msg *msg)
{
	char *token, *p;
	int modes;

	if (!msg->args[2])
		return;

	for (p = msg->args[2]; (token = strtok_r(p, " ", &p)); ) {
		modes = irc_server_strip(server, (const char **)&token);

		ev->channels = irc_util_reallocarray(ev->channels,
		    ev->channelsz + 1, sizeof (*ev->channels));
		ev->channels[ev->channelsz].name = irc_util_strdup(token);
		ev->channels[ev->channelsz++].modes = modes;
	}
}

static void
irc_server_handle_whoisuser(struct irc_server *server, struct conn_msg *msg)
{
	struct irc_event ev = {};

	ev.type = IRC_EVENT_WHOIS;
	ev.server = server;
	ev.whois.nickname = irc_util_strdup(msg->args[1]);
	ev.whois.username = irc_util_strdup(msg->args[2]);
	ev.whois.hostname = irc_util_strdup(msg->args[3]);
	ev.whois.realname = irc_util_strdup(msg->args[5]);

	/* Now yield until we get end of whois. */
	do {
		irc__conn_pull(&server->coroutine->conn, msg);

		if (strcmp(msg->cmd, "319") == 0)
			irc_server_handle_whoischannels(server, &ev.whois, msg);
	} while (strcmp(msg->cmd, "318") != 0);

	irc_bot_dispatch(&ev);
}

static const struct handler {
	const char *command;
	void (*handle)(struct irc_server *, struct conn_msg *);
} handlers[] = {
	/* Must be kept ordered. */
	{ "000",        irc_server_handle_disconnect       },
	{ "001",        irc_server_handle_connect          },
	{ "005",        irc_server_handle_support          },
	{ "311",        irc_server_handle_whoisuser        },
	{ "353",        irc_server_handle_names            },
	{ "366",        irc_server_handle_endofnames       },
	{ "433",        irc_server_handle_nicknameinuse    },
	{ "ERROR",      irc_server_handle_error            },
	{ "INVITE",     irc_server_handle_invite           },
	{ "JOIN",       irc_server_handle_join             },
	{ "KICK",       irc_server_handle_kick             },
	{ "MODE",       irc_server_handle_mode             },
	{ "NICK",       irc_server_handle_nick             },
	{ "NOTICE",     irc_server_handle_notice           },
	{ "PART",       irc_server_handle_part             },
	{ "PING",       irc_server_handle_ping             },
	{ "PRIVMSG",    irc_server_handle_msg              },
	{ "TOPIC",      irc_server_handle_topic            }
};

static int
irc_server_handler_cmp(const void *name, const void *data)
{
	const struct handler *handler = data;

	return strcmp(name, handler->command);
}

static void
irc_server_handle(struct irc_server *server, struct conn_msg *msg)
{
	const struct handler *h;

	h = bsearch(msg->cmd, handlers, IRC_UTIL_SIZE(handlers),
	    sizeof (struct handler), irc_server_handler_cmp);

	if (h)
		h->handle(server, msg);
}

static void
irc_server_consumer_entry(struct nce_coro *self)
{
	struct irc_server_coro *sco;
	struct conn_msg msg;

	sco = IRC_UTIL_CONTAINER_OF(self, struct irc_server_coro, consumer);

	for (;;) {
		irc__conn_pull(&sco->conn, &msg);
		irc_server_handle(sco->conn.parent, &msg);
		irc__conn_msg_finish(&msg);
	}
}

static struct irc_server_coro *
irc_server_coro_new(struct irc_server *server)
{
	struct irc_server_coro *sco;

	sco = irc_util_calloc(1, sizeof (*sco));

	/* Initialize connection first. */
	irc__conn_spawn(&sco->conn, server);

	/* And now our consumer coroutine. */
	sco->consumer.name = "server.coroutine";
	sco->consumer.entry = irc_server_consumer_entry;
	sco->consumer.priority = sco->conn.io_fd.coro.priority + 1;
	nce_coro_spawn(&sco->consumer);

	return sco;
}

static inline void
irc_server_coro_free(struct irc_server_coro *sco)
{
	if (!sco)
		return;

	irc__conn_destroy(&sco->conn);
	nce_coro_destroy(&sco->consumer);
	free(sco);
}

struct irc_server *
irc_server_new(const char *name)
{
	assert(name);

	struct irc_server *server;

	server = irc_util_calloc(1, sizeof (*server));

	/* Predefined sane defaults. */
	server->name         = irc_util_strdup(name);
	server->port         = IRC_SERVER_DEFAULT_PORT;
	server->prefix       = irc_util_strdup(IRC_SERVER_DEFAULT_PREFIX);
	server->ctcp_version = irc_util_strdup(IRC_SERVER_DEFAULT_CTCP_VERSION);
	server->ctcp_source  = irc_util_strdup(IRC_SERVER_DEFAULT_CTCP_SOURCE);

	return server;
}

void
irc_server_set_hostname(struct irc_server *server, const char *hostname)
{
	assert(server);
	assert(server->coroutine == NULL);

	server->hostname = irc_util_strdupfree(server->hostname, hostname);
}

void
irc_server_set_flags(struct irc_server *server, enum irc_server_flags flags)
{
	assert(server);

	server->flags = flags;
}

void
irc_server_set_port(struct irc_server *server, unsigned int port)
{
	assert(server);
	assert(server->coroutine == NULL);

	server->port = port;
}

void
irc_server_set_nickname(struct irc_server *server, const char *nickname)
{
	assert(server);
	assert(nickname);

	if (irc__conn_ready(&server->coroutine->conn))
		irc_server_send(server, "NICK %s", nickname);
	else
		server->nickname = irc_util_strdupfree(server->nickname, nickname);
}

void
irc_server_set_username(struct irc_server *server, const char *username)
{
	assert(server);
	assert(server->coroutine == NULL);
	assert(username);

	server->username = irc_util_strdupfree(server->username, username);
}

void
irc_server_set_realname(struct irc_server *server, const char *realname)
{
	assert(server);
	assert(server->coroutine == NULL);
	assert(realname);

	server->realname = irc_util_strdupfree(server->realname, realname);
}

void
irc_server_set_ctcp(struct irc_server *server, const char *key, const char *value)
{
	assert(server);
	assert(key);
	assert(value);

	if (strcasecmp(key, "version") == 0)
		server->ctcp_version = irc_util_strdupfree(server->ctcp_version, value);
	else if (strcasecmp(key, "source") == 0)
		server->ctcp_source = irc_util_strdupfree(server->ctcp_source, value);
	else
		WARN("invalid CTCP '%s'", key);
}

void
irc_server_set_prefix(struct irc_server *server, const char *prefix)
{
	assert(server);
	assert(prefix);

	server->prefix = irc_util_strdupfree(server->prefix, prefix);
}

void
irc_server_set_password(struct irc_server *server, const char *password)
{
	assert(server);
	assert(server->coroutine == NULL);

	server->password = irc_util_strdupfree(server->password, password);
}

void
irc_server_connect(struct irc_server *server)
{
	assert(server);
	assert(server->hostname);
	assert(server->port);
	assert(server->nickname);
	assert(server->username);
	assert(server->realname);

	if (server->coroutine) {
		INFO("already connected");
		return;
	}

#ifdef IRCCD_WITH_SSL
	if (server->flags & IRC_SERVER_FLAGS_SSL) {
		WARN("SSL requested but not available");
		return;
	}
#endif

	server->coroutine = irc_server_coro_new(server);
}

void
irc_server_disconnect(struct irc_server *server)
{
	assert(server);

	irc_server_coro_free(server->coroutine);
	server->coroutine = NULL;

	irc_server_channels_clear(server);
	irc_server_clear(server);
}

void
irc_server_reconnect(struct irc_server *server)
{
	assert(server);

	irc_server_disconnect(server);
	irc_server_connect(server);
}

const struct irc_channel *
irc_server_channels_find(struct irc_server *server, const char *name)
{
	assert(server);
	assert(name);

	struct irc_channel *ch;

	LL_FOREACH(server->channels, ch)
		if (strcasecmp(ch->name, name) == 0)
			return ch;

	return NULL;
}

int
irc_server_send(struct irc_server *server, const char *fmt, ...)
{
	assert(server);
	assert(fmt);

	va_list ap;
	int rc;

	va_start(ap, fmt);
	rc = irc_server_send_va(server, fmt, ap);
	va_end(ap);

	return rc;
}

int
irc_server_send_va(struct irc_server *server, const char *fmt, va_list ap)
{
	char buf[IRCCD_MESSAGE_LEN];

	if (!irc__conn_ready(&server->coroutine->conn))
		return -ENOTCONN;

	vsnprintf(buf, sizeof (buf), fmt, ap);

	return irc__conn_push(&server->coroutine->conn, buf, strlen(buf));
}

int
irc_server_invite(struct irc_server *server, const char *channel, const char *target)
{
	assert(server);
	assert(channel);
	assert(target);

	return irc_server_send(server, "INVITE %s %s", target, channel);
}

int
irc_server_join(struct irc_server *server, const char *name, const char *pass)
{
	assert(server);
	assert(name);

	struct irc_channel *ch;
	int ret = 1;

	/*
	 * Search if there is already a channel pending or joined. If the
	 * server is connected we send a join command otherwise we put it there
	 * and wait for connection.
	 */
	ch = (struct irc_channel *)irc_server_channels_find(server, name);

	if (!ch)
		ch = irc_server_channels_add(server, name, pass, IRC_CHANNEL_FLAGS_NONE);

	if (!(ch->flags & IRC_CHANNEL_FLAGS_JOINED) && irc__conn_ready(&server->coroutine->conn)) {
		if (pass)
			ret = irc_server_send(server, "JOIN %s %s", name, pass);
		else
			ret = irc_server_send(server, "JOIN %s", name);
	}

	return ret;
}

int
irc_server_kick(struct irc_server *server, const char *channel, const char *target, const char *reason)
{
	assert(server);
	assert(channel);
	assert(target);

	int ret;

	if (reason)
		ret = irc_server_send(server, "KICK %s %s :%s", channel, target, reason);
	else
		ret = irc_server_send(server, "KICK %s %s", channel, target);

	return ret;
}

int
irc_server_part(struct irc_server *server, const char *channel, const char *reason)
{
	assert(server);
	assert(channel);

	int ret;

	if (reason && strlen(reason) > 0)
		ret = irc_server_send(server, "PART %s :%s", channel, reason);
	else
		ret = irc_server_send(server, "PART %s", channel);

	return ret;
}

int
irc_server_topic(struct irc_server *server, const char *channel, const char *topic)
{
	assert(server);
	assert(channel);
	assert(topic);

	return irc_server_send(server, "TOPIC %s :%s", channel, topic);
}

int
irc_server_message(struct irc_server *server, const char *chan, const char *msg)
{
	assert(server);
	assert(chan);
	assert(msg);

	return irc_server_send(server, "PRIVMSG %s :%s", chan, msg);
}

int
irc_server_me(struct irc_server *server, const char *chan, const char *message)
{
	assert(server);
	assert(chan);
	assert(message);

	return irc_server_send(server, "PRIVMSG %s :\001ACTION %s\001", chan, message);
}

int
irc_server_mode(struct irc_server *server, const char *channel, const char *mode, const char *args)
{
	assert(server);
	assert(channel);
	assert(mode);

	args = args ? args : "";

	return irc_server_send(server, "MODE %s %s %s", channel, mode, args);
}

int
irc_server_names(struct irc_server *server, const char *channel)
{
	return irc_server_send(server, "NAMES %s", channel);
}

int
irc_server_notice(struct irc_server *server, const char *channel, const char *message)
{
	assert(server);
	assert(channel);
	assert(message);

	return irc_server_send(server, "NOTICE %s :%s", channel, message);
}

int
irc_server_whois(struct irc_server *server, const char *target)
{
	assert(server);
	assert(target);

	return irc_server_send(server, "WHOIS %s", target);
}

int
irc_server_strip(const struct irc_server *server, const char **what)
{
	assert(server);
	assert(*what);

	int modes = 0;

	for (size_t i = 0; i < server->prefixesz; ++i) {
		if (**what == server->prefixes[i].symbol) {
			modes |= 1 << i;
			*what += 1;
		}
	}

	return modes;
}

void
irc_server_incref(struct irc_server *server)
{
	assert(server);

	server->refc++;
}

void
irc_server_decref(struct irc_server *server)
{
	assert(server);
	assert(server->refc >= 1);

	if (--server->refc == 0)
		irc_server_free(server);
}
