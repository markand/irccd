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

#include <sys/types.h>
#include <sys/socket.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <poll.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "config.h"

#if defined(IRCCD_WITH_SSL)
#       include <openssl/err.h>
#endif

#include "channel.h"
#include "event.h"
#include "log.h"
#include "server.h"
#include "set.h"
#include "util.h"

struct origin {
	char nickname[IRC_NICKNAME_MAX];
	char username[IRC_USERNAME_MAX];
	char host[IRC_HOST_MAX];
};

struct message {
	char *prefix;
	char *cmd;
	char *args[IRC_ARGS_MAX];
	char buf[IRC_MESSAGE_MAX];
};

static inline void
scan(char **line, char **str)
{
	char *p = strchr(*line, ' ');

	if (p)
		*p = '\0';

	*str = *line;
	*line = p ? p + 1 : strchr(*line, '\0');
}

static bool
parse(struct message *msg, const char *line)
{
	char *ptr = msg->buf;
	size_t a;

	memset(msg, 0, sizeof (*msg));
	strlcpy(msg->buf, line, sizeof (msg->buf));

	/*
	 * IRC message is defined as following:
	 *
	 * [:prefix] command arg1 arg2 [:last-argument]
	 */
	if (*ptr == ':')
		scan((++ptr, &ptr), &msg->prefix);     /* prefix */

	scan(&ptr, &msg->cmd);                         /* command */

	/* And finally arguments. */
	for (a = 0; *ptr && a < IRC_ARGS_MAX; ++a) {
		if (*ptr == ':') {
			msg->args[a] = ptr + 1;
			ptr = strchr(ptr, '\0');
		} else
			scan(&ptr, &msg->args[a]);
	}

	if (a >= IRC_ARGS_MAX)
		return errno = EMSGSIZE, false;
	if (msg->cmd == NULL)
		return errno = EBADMSG, false;

	return true;
}

static inline char
sym(const struct irc_server *s, char mode)
{
	for (size_t i = 0; i < sizeof (s->prefixes); ++i)
		if (s->prefixes[i].mode == mode)
			return s->prefixes[i].token;

	return '?';
}

static inline bool
is_self(const struct irc_server *s, const char *nick)
{
	return strncmp(s->nickname, nick, strlen(s->nickname)) == 0;
}

static int
cmp_channel(const struct irc_channel *c1, const struct irc_channel *c2)
{
	return strcmp(c1->name, c2->name);
}

#if 0

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
	char mode = 0;

	for (size_t i = 0; i < IRC_UTIL_SIZE(s->prefixes); ++i) {
		if (nick[0] == s->prefixes[i].token) {
			mode = s->prefixes[i].mode;
			++nick;
			break;
		}
	}

	irc_channel_add(ch, nick, mode);
}

static struct irc_channel *
add_channel(struct irc_server *s, const char *name, const char *password, bool joined)
{
	struct irc_channel chnew = {0}, *ch;

	if ((ch = irc_server_find(s, name))) {
		ch->joined = joined;
		return ch;
	}

	strlcpy(chnew.name, name, sizeof (chnew.name));

	if (password)
		strlcpy(chnew.password, password, sizeof (chnew.password));

	chnew.joined = joined;

	IRC_SET_ALLOC_PUSH(&s->channels, &s->channelsz, &chnew, cmp_channel);

	return irc_server_find(s, name);
}

static void
remove_channel(struct irc_server *s, struct irc_channel *ch)
{
	irc_channel_clear(ch);
	IRC_SET_ALLOC_REMOVE(&s->channels, &s->channelsz, ch);
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
	char modes[16 + 1] = {0};
	char tokens[16 + 1] = {0};

	if (sscanf(value, "(%16[^)])%16s", modes, tokens) == 2) {
		char *pm = modes;
		char *tk = tokens;

		for (size_t i = 0; i < 16 && *pm && *tk; ++i) {
			s->prefixes[i].mode = *pm++;
			s->prefixes[i].token = *tk++;
		}
	}
}

static void
read_support_chantypes(struct irc_server *s, const char *value)
{
	strlcpy(s->chantypes, value, sizeof (s->chantypes));
}

static void
handle_connect(struct irc_server *s, struct irc_event *ev, struct message *msg)
{
	(void)msg;

	s->state = IRC_SERVER_STATE_CONNECTED;

	/* Now join all channels that were requested. */
	for (size_t i = 0; i < s->channelsz; ++i)
		irc_server_join(s, s->channels[i].name, s->channels[i].password);

	ev->type = IRC_EVENT_CONNECT;
}

static void
handle_support(struct irc_server *s, struct irc_event *ev, struct message *msg)
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
handle_invite(struct irc_server *s, struct irc_event *ev, struct message *msg)
{
	ev->type = IRC_EVENT_INVITE;
	ev->invite.origin = strdup(msg->args[0]);
	ev->invite.channel = strdup(msg->args[1]);
	ev->invite.target = strdup(msg->args[2]);

	if (is_self(s, ev->invite.target) && s->flags & IRC_SERVER_FLAGS_JOIN_INVITE)
		irc_server_join(s, ev->invite.channel, NULL);
}

static void
handle_join(struct irc_server *s, struct irc_event *ev, struct message *msg)
{
	ev->type = IRC_EVENT_JOIN;
	ev->join.origin = strdup(msg->prefix);
	ev->join.channel = strdup(msg->args[0]);

	add_channel(s, ev->join.channel, NULL, true);
}

static void
handle_kick(struct irc_server *s, struct irc_event *ev, struct message *msg)
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
handle_mode(struct irc_server *s, struct irc_event *ev, struct message *msg)
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
handle_part(struct irc_server *s, struct irc_event *ev, struct message *msg)
{
	struct irc_channel *ch;

	ev->type = IRC_EVENT_PART;
	ev->part.origin = strdup(msg->prefix);
	ev->part.channel = strdup(msg->args[0]);
	ev->part.reason = msg->args[1] ? strdup(msg->args[1]) : NULL;

	ch = add_channel(s, ev->part.channel, NULL, true);

	if (is_self(s, ev->part.origin) == 0)
		remove_channel(s, ch);
	else
		irc_channel_remove(ch, ev->part.origin);
}

static void
handle_msg(struct irc_server *s, struct irc_event *ev, struct message *msg)
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
handle_nick(struct irc_server *s, struct irc_event *ev, struct message *msg)
{
	ev->type = IRC_EVENT_NICK;
	ev->nick.origin = strdup(msg->prefix);
	ev->nick.nickname = strdup(msg->args[0]);

	/* Update nickname if it is myself. */
	if (is_self(s, ev->nick.origin) == 0)
		strlcpy(s->nickname, ev->nick.nickname, sizeof (s->nickname));
}

static void
handle_notice(struct irc_server *s, struct irc_event *ev, struct message *msg)
{
	(void)s;

	ev->type = IRC_EVENT_NOTICE;
	ev->notice.origin = strdup(msg->prefix);
	ev->notice.channel = strdup(msg->args[0]);
	ev->notice.notice = strdup(msg->args[1]);
}

static void
handle_topic(struct irc_server *s, struct irc_event *ev, struct message *msg)
{
	(void)s;

	ev->type = IRC_EVENT_TOPIC;
	ev->topic.origin = strdup(msg->prefix);
	ev->topic.channel = strdup(msg->args[0]);
	ev->topic.topic = strdup(msg->args[1]);
}

static void
handle_ping(struct irc_server *s, struct irc_event *ev, struct message *msg)
{
	(void)s;
	(void)ev;
	(void)msg;

	//irc_server_send(s, "PONG %s", args[1]);
}

static void
handle_names(struct irc_server *s, struct irc_event *ev, struct message *msg)
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
handle_endofnames(struct irc_server *s, struct irc_event *ev, struct message *msg)
{
	(void)s;
	(void)ev;
	(void)msg;

	FILE *fp;
	const struct irc_channel *ch;
	size_t length;

	ev->type = IRC_EVENT_NAMES;
	ev->names.channel = strdup(msg->args[1]);

	/* Construct a string list for every user in the channel. */
	ch = irc_server_find(s, ev->names.channel);
	fp = open_memstream(&ev->names.names, &length);

	for (size_t i = 0; i < ch->usersz; ++i) {
		const struct irc_channel_user *u = &ch->users[i];

		if (u->mode)
			fprintf(fp, "%c", sym(s, u->mode));

		fprintf(fp, "%s", u->nickname);

		if (i + 1 < ch->usersz)
			fputc(' ', fp);
	}

	fclose(fp);
}

static void
handle_whoisuser(struct irc_server *s, struct irc_event *ev, struct message *msg)
{
	(void)s;
	(void)msg;

	s->bufwhois.nickname = strdup(msg->args[1]);
	s->bufwhois.username = strdup(msg->args[2]);
	s->bufwhois.hostname = strdup(msg->args[3]);
	s->bufwhois.realname = strdup(msg->args[5]);
}

static void
handle_whoischannels(struct irc_server *s, struct irc_event *ev, struct message *msg)
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
handle_endofwhois(struct irc_server *s, struct irc_event *ev, struct message *msg)
{
	(void)msg;

	ev->type = IRC_EVENT_WHOIS;
	ev->whois = s->bufwhois;

	memset(&s->bufwhois, 0, sizeof (s->bufwhois));
}

static const struct handler {
	const char *command;
	void (*handle)(struct irc_server *, struct irc_event *, struct message *);
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
handle(struct irc_server *s, struct irc_event *ev, struct message *msg)
{
	const struct handler *h;

	if (!(h = find_handler(msg->cmd)))
		return;

	memset(ev, 0, sizeof (*ev));

	ev->server = s;
	h->handle(s, ev, msg);
}

static void
clear(struct irc_server *s)
{
	s->state = IRC_SERVER_STATE_DISCONNECTED;

	if (s->fd != 0) {
		close(s->fd);
		s->fd = 0;
	}

	if (s->ai) {
		freeaddrinfo(s->ai);
		s->ai = NULL;
		s->aip = NULL;
	}

#if defined(IRCCD_WITH_SSL)
	if (s->ssl) {
		SSL_free(s->ssl);
		s->ssl = NULL;
	}
	if (s->ctx) {
		SSL_CTX_free(s->ctx);
		s->ctx = NULL;
	}
#endif
}

static bool
lookup(struct irc_server *s)
{
	struct addrinfo hints = {
		.ai_socktype = SOCK_STREAM,
	};
	char service[16];
	int ret;

	snprintf(service, sizeof (service), "%hu", s->port);

	if ((ret = getaddrinfo(s->hostname, service, &hints, &s->ai)) != 0)
		irc_log_warn("server %s: %s", s->name, gai_strerror(ret));

	s->aip = s->ai;

	return true;
}

static void
auth(struct irc_server *s)
{
	s->state = IRC_SERVER_STATE_CONNECTED;

	if (s->password[0])
		irc_server_send(s, "PASS %s", s->password);

	irc_server_send(s, "NICK %s", s->nickname);
	irc_server_send(s, "USER %s %s %s :%s", s->username,
		s->username, s->username, s->realname);
}

#if defined(IRCCD_WITH_SSL)

static void
update(struct irc_server *s, int ret)
{
	(void)s;
	(void)ret;

	assert(s);

	if (!(s->flags & IRC_SERVER_FLAGS_SSL))
		return;

	switch (SSL_get_error(s->ssl, ret)) {
	case SSL_ERROR_WANT_READ:
		s->ssl_state = IRC_SERVER_SSL_NEED_READ;
		break;
	case SSL_ERROR_WANT_WRITE:
		s->ssl_state = IRC_SERVER_SSL_NEED_WRITE;
		break;
	case SSL_ERROR_SSL:
		clear(s);
		break;
	default:
		s->ssl_state = IRC_SERVER_SSL_NONE;
		break;
	}
}

#endif

static void
handshake(struct irc_server *s)
{
	assert(s);

	if (!(s->flags & IRC_SERVER_FLAGS_SSL))
		auth(s);
	else {
#if defined(IRCCD_WITH_SSL)
		int r;

		s->state = IRC_SERVER_STATE_HANDSHAKING;

		if ((r = SSL_do_handshake(s->ssl)) > 0)
			auth(s);

		update(s, r);
#endif
	}
}

static void
try_connect(struct irc_server *s)
{
	assert(s);

	if (!(s->flags & IRC_SERVER_FLAGS_SSL))
		handshake(s);
	else {
#if defined(IRCCD_WITH_SSL)
		if (!s->ctx)
			s->ctx = SSL_CTX_new(TLS_method());
		if (!s->ssl) {
			s->ssl = SSL_new(s->ctx);
			SSL_set_fd(s->ssl, s->fd);
		}

		SSL_set_connect_state(s->ssl);
		handshake(s);
#endif
	}
}

static bool
set_nonblock(struct irc_server *s)
{
	int cflags = 0;

	if ((cflags = fcntl(s->fd, F_GETFL)) < 0 ||
	    fcntl(s->fd, F_SETFL, cflags | O_NONBLOCK) < 0)
		return false;

	return true;
}

static bool
create(struct irc_server *s)
{
	s->fd = socket(s->aip->ai_family, s->aip->ai_socktype,
	    s->aip->ai_protocol);

	return set_nonblock(s);
}

static void
dial(struct irc_server *s)
{
	/* No more address available. */
	if (s->aip == NULL) {
		clear(s);
		return;
	}

	for (; s->aip; s->aip = s->aip->ai_next) {
		/* We may need to close a socket that was open earlier. */
		if (s->fd != 0)
			close(s->fd);

		if (!create(s)) {
			irc_log_warn("server %s: %s", s->name, strerror(errno));
			continue;
		}

		/*
		 * With some luck, the connection completes immediately,
		 * otherwise we will need to wait until the socket is writable.
		 */
		if (connect(s->fd, s->aip->ai_addr, s->aip->ai_addrlen) == 0) {
			try_connect(s);
			break;
		}

		/* Connect failed, check why. */
		switch (errno) {
		case EINPROGRESS:
		case EAGAIN:
			/* Let the writable state to determine. */
			return;
		default:
			irc_log_warn("server %s: %s", s->name, strerror(errno));
			break;
		}
	}
}

static size_t
input_ssl(struct irc_server *s, char *dst, size_t dstsz)
{
	int nr;

	if ((nr = SSL_read(s->ssl, dst, dstsz)) <= 0) {
		update(s, nr);
		return 0;
	}

	s->ssl_state = IRC_SERVER_SSL_NONE;

	return nr;
}

static size_t
input_clear(struct irc_server *s, char *buf, size_t bufsz)
{
	ssize_t nr;

	if ((nr = recv(s->fd, buf, bufsz, 0)) <= 0) {
		clear(s);
		return 0;
	}

	return nr;
}

static void
input(struct irc_server *s)
{
	size_t len = strlen(s->in);
	size_t cap = sizeof (s->in) - len - 1;
	size_t nr = 0;

	if (s->flags & IRC_SERVER_FLAGS_SSL) {
#if defined(IRCCD_WITH_SSL)
		nr = input_ssl(s, s->in + len, cap);
#endif
	} else
		nr = input_clear(s, s->in + len, cap);

	if (nr > 0)
		s->in[len + nr] = '\0';
}

static void
output(struct irc_server *s)
{
	ssize_t ns = 0;

	if (s->flags & IRC_SERVER_FLAGS_SSL) {
#if defined(IRCCD_WITH_SSL)
		ns = SSL_write(s->ssl, s->out, strlen(s->out));
		update(s, ns);
#endif
	} else if ((ns = send(s->fd, s->out, strlen(s->out), 0)) <= 0)
		clear(s);

	if (ns > 0) {
		/* Optimize if everything was sent. */
		if ((size_t)ns >= sizeof (s->out))
			s->out[0] = '\0';
		else
			memmove(s->out, s->out + ns, sizeof (s->out) - ns);
	}
}

static void
prepare_connecting(const struct irc_server *s, struct pollfd *pfd)
{
	(void)s;

#if defined(IRCCD_WITH_SSL)
	if (s->flags & IRC_SERVER_FLAGS_SSL && s->ssl && s->ctx) {
		switch (s->ssl_state) {
		case IRC_SERVER_SSL_NEED_READ:
			pfd->events |= POLLIN;
			break;
		case IRC_SERVER_SSL_NEED_WRITE:
			pfd->events |= POLLOUT;
			break;
		default:
			break;
		}
	} else
#endif
		pfd->events |= POLLOUT;
}

static void
prepare_ready(const struct irc_server *s, struct pollfd *pfd)
{
#if defined(IRCCD_WITH_SSL)
	if (s->flags & IRC_SERVER_FLAGS_SSL && s->ssl_state) {
		switch (s->ssl_state) {
		case IRC_SERVER_SSL_NEED_READ:
			pfd->events |= POLLIN;
			break;
		case IRC_SERVER_SSL_NEED_WRITE:
			pfd->events |= POLLOUT;
			break;
		default:
			break;
		}
	} else {
#endif
		pfd->events |= POLLIN;

		if (s->out[0])
			pfd->events |= POLLOUT;
#if defined(IRCCD_WITH_SSL)
	}
#endif
}

static void
flush_connecting(struct irc_server *s, const struct pollfd *pfd)
{
	(void)pfd;

	int res, err = -1;
	socklen_t len = sizeof (int);

	if ((res = getsockopt(s->fd, SOL_SOCKET, SO_ERROR, &err, &len)) < 0 || err) {
		irc_log_warn("server %s: %s", s->name, strerror(res ? err : errno));
		dial(s);
	} else
		try_connect(s);
}

static void
flush_handshaking(struct irc_server *s, const struct pollfd *pfd)
{
	(void)pfd;

	handshake(s);
}

static void
flush_ready(struct irc_server *s, const struct pollfd *pfd)
{
	if (pfd->revents & POLLERR || pfd->revents & POLLHUP) {
		clear(s);
		return;
	}

	if (pfd->revents & POLLIN)
		input(s);
	if (pfd->revents & POLLOUT)
		output(s);
}

static const struct {
	void (*prepare)(const struct irc_server *, struct pollfd *);
	void (*flush)(struct irc_server *, const struct pollfd *);
} io_table[] = {
	[IRC_SERVER_STATE_CONNECTING] = {
		prepare_connecting,
		flush_connecting
	},
	[IRC_SERVER_STATE_HANDSHAKING] = {
		prepare_ready,
		flush_handshaking
	},
	[IRC_SERVER_STATE_CONNECTED] = {
		prepare_ready,
		flush_ready
	},
};

void
irc_server_connect(struct irc_server *s)
{
	assert(s);

	s->state = IRC_SERVER_STATE_CONNECTING;

	if (!lookup(s))
		clear(s);
	else
		dial(s);
}

void
irc_server_disconnect(struct irc_server *s)
{
	assert(s);

	clear(s);
}

void
irc_server_prepare(const struct irc_server *s, struct pollfd *pfd)
{
	pfd->fd = s->fd;
	pfd->events = 0;

	if (io_table[s->state].prepare)
		io_table[s->state].prepare(s, pfd);
}

void
irc_server_flush(struct irc_server *s, const struct pollfd *pfd)
{
	if (pfd->fd != s->fd)
		return;

	if (io_table[s->state].flush)
		io_table[s->state].flush(s, pfd);
}

bool
irc_server_poll(struct irc_server *s, struct irc_event *ev)
{
	assert(s);
	assert(ev);

	struct message msg;
	char *pos;
	size_t length;

	if (!(pos = strstr(s->in, "\r\n")))
		return false;

	/* Turn end of the string at delimiter. */
	*pos = 0;
	length = pos - s->in;

	if (length > 0 && parse(&msg, s->in))
		handle(s, ev, &msg);

	memmove(s->in, pos + 2, sizeof (s->in) - (length + 2));

	return true;
}

struct irc_channel *
irc_server_find(struct irc_server *s, const char *name)
{
	assert(s);
	assert(name);

	struct irc_channel key = {0};

	strlcpy(key.name, name, sizeof (key.name));

	return IRC_SET_FIND(s->channels, s->channelsz, &key, cmp_channel);
}

bool
irc_server_send(struct irc_server *s, const char *fmt, ...)
{
	assert(s);
	assert(fmt);

	char buf[IRC_BUF_MAX];
	va_list ap;
	size_t len, avail, required;

	va_start(ap, fmt);
	required = vsnprintf(buf, sizeof (buf), fmt, ap);
	va_end(ap);

	len = strlen(s->out);
	avail = sizeof (s->out) - len;

	/* Don't forget \r\n. */
	if (required + 2 >= avail)
		return false;

	strlcat(s->out, buf, sizeof (s->out));
	strlcat(s->out, "\r\n", sizeof (s->out));

	return true;
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

	if (s->state == IRC_SERVER_STATE_DISCONNECTED) {
		strlcpy(s->nickname, nick, sizeof (s->nickname));
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

#if 0
	/* Cleanup previous result. */
	free(s->whois.channels);
	memset(&s->whois, 0, sizeof (s->whois));
#endif

	return irc_server_send(s, "WHOIS %s", target);
}

struct irc_server_namemode
irc_server_strip(const struct irc_server *s, const char *item)
{
	assert(s);
	assert(item);

	struct irc_server_namemode ret = {0};

	for (size_t i = 0; i < IRC_UTIL_SIZE(s->prefixes); ++i) {
		if (item[0] == s->prefixes[i].token) {
			ret.mode = s->prefixes[i].mode;
			++item;
			break;
		}
	}

	ret.name = (char *)item;

	return ret;
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
		clear(s);
		//free(s->whois.channels);
		free(s->channels);
		free(s);
	}
}
