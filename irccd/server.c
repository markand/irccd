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

#if defined(IRCCD_HAVE_SSL)
#       include <openssl/err.h>
#endif

#include "event.h"
#include "log.h"
#include "server.h"
#include "util.h"

struct origin {
	char nickname[IRC_NICKNAME_MAX];
	char username[IRC_USERNAME_MAX];
	char host[IRC_HOST_MAX];
};

static bool
is_ctcp(const char *line)
{
	assert(line);

	const size_t length = strlen(line);

	if (length < 2)
		return false;

	return line[0] == 0x1 && line[length - 1] == 0x1;
}

static char *
ctcp(char *line)
{
	assert(line);

	/* Remove last \001. */
	line[strcspn(line, "\001")] = 0;

	if (strncmp(line, "ACTION ", 7) == 0)
		line += 7;

	return line;
}

static int
compare_chan(const void *d1, const void *d2)
{
	return strcmp(
		((const struct irc_server_channel *)d1)->name,
		((const struct irc_server_channel *)d2)->name
	);
}

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

static inline void
sort(struct irc_server *s)
{
	qsort(s->channels, s->channelsz, sizeof (*s->channels), compare_chan);
}

static struct irc_server_channel *
add_channel(struct irc_server *s, const char *name, const char *password, bool joined)
{
	struct irc_server_channel ch = {
		.joined = joined
	};

	strlcpy(ch.name, name, sizeof (ch.name));

	if (password)
		strlcpy(ch.password, password, sizeof (ch.password));

	s->channels = irc_util_reallocarray(s->channels, ++s->channelsz, sizeof (ch));

	memcpy(&s->channels[s->channelsz - 1], &ch, sizeof (ch));
	sort(s);

	return irc_server_find(s, name);
}

static void
remove_channel(struct irc_server *s, struct irc_server_channel *ch)
{
	/* Null channel name will be moved at the end. */
	memset(ch, 0, sizeof (*ch));

	s->channels = irc_util_reallocarray(s->channels, --s->channelsz, sizeof (*ch));
	sort(s);
}

static void
read_support_prefix(struct irc_server *s, const char *value)
{
	char modes[16 + 1] = { 0 };
	char tokens[16 + 1] = { 0 };

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
convert_connect(struct irc_server *s, struct irc_event *ev)
{
	s->state = IRC_SERVER_STATE_CONNECTED;

	ev->type = IRC_EVENT_CONNECT;
	ev->server = s;

	/* Now join all channels that were requested. */
	for (size_t i = 0; i < s->channelsz; ++i)
		irc_server_join(s, s->channels[i].name, s->channels[i].password);
}

static void
convert_support(struct irc_server *s, struct irc_event *ev)
{
	char key[64];
	char value[64];

	for (size_t i = 4; i < ev->argsz; ++i) {
		if (sscanf(ev->args[i], "%63[^=]=%63s", key, value) != 2)
			continue;

		if (strcmp(key, "PREFIX") == 0)
			read_support_prefix(s, value);
		if (strcmp(key, "CHANTYPES") == 0)
			read_support_chantypes(s, value);
	}
}

static void
convert_join(struct irc_server *s, struct irc_event *ev)
{
	const struct origin *origin = parse_origin(ev->args[0]);
	struct irc_server_channel *ch;

	ev->type = IRC_EVENT_JOIN;
	ev->server = s;
	ev->join.origin = ev->args[0];
	ev->join.channel = ev->args[2];

	/* Also add a channel if the bot joined. */
	if (strcmp(s->nickname, origin->nickname)) {
		if ((ch = irc_server_find(s, ev->args[2])))
			ch->joined = true;
		else
			add_channel(s, ev->args[2], NULL, true);
	}
}

static void
convert_kick(struct irc_server *s, struct irc_event *ev)
{
	ev->type = IRC_EVENT_KICK;
	ev->server = s;
	ev->kick.origin = ev->args[0];
	ev->kick.channel = ev->args[2];
	ev->kick.target = ev->args[3];
	ev->kick.reason = ev->args[4];

	/*
	 * If the bot was kicked itself mark the channel as not joined and
	 * rejoin it automatically if the option is set.
	 */
	if (strcmp(ev->args[3], s->nickname) == 0) {
		struct irc_server_channel *ch = irc_server_find(s, ev->args[2]);

		if (ch) {
			ch->joined = false;

			if (s->flags & IRC_SERVER_FLAGS_AUTO_REJOIN)
				irc_server_join(s, ch->name, ch->password);
		}
	}
}

static void
convert_mode(struct irc_server *s, struct irc_event *ev)
{
	(void)s;
	(void)ev;

	for (size_t i = 0; i < ev->argsz; ++i) {
		printf("MODE: %zu=%s\n", i, ev->args[i]);
	}

#if 0
	if (strcmp(m->args[0], s->nickname) == 0) {
		/* Own user modes. */
		strlcpy(s->usermodes, m->args[1], sizeof (s->usermodes);
	} else {
		/* TODO: channel modes. */
	}
#endif
}

static void
convert_part(struct irc_server *s, struct irc_event *ev)
{
	const struct origin *origin = parse_origin(ev->args[0]);
	struct irc_server_channel *ch = irc_server_find(s, ev->args[2]);

	ev->type = IRC_EVENT_PART;
	ev->server = s;
	ev->part.origin = ev->args[0];
	ev->part.channel = ev->args[2];
	ev->part.reason = ev->args[3];

	if (ch && strcmp(origin->nickname, s->nickname) == 0)
		remove_channel(s, ch);
}

static void
convert_msg(struct irc_server *s, struct irc_event *ev)
{
	ev->type = IRC_EVENT_MESSAGE;
	ev->server = s;
	ev->message.origin = ev->args[0];
	ev->message.channel = ev->args[2];
	ev->message.message = ev->args[3];

	/*
	 * Detect CTCP commands which are PRIVMSG with a special boundaries.
	 *
	 * Example:
	 * PRIVMSG jean :\001ACTION I'm eating\001.
	 */
	if (is_ctcp(ev->args[3])) {
		ev->type = IRC_EVENT_ME;
		ev->message.message = ctcp(ev->args[3] + 1);
	}
}

static void
convert_nick(struct irc_server *s, struct irc_event *ev)
{
	const struct origin *origin = parse_origin(ev->args[0]);

	/* Update nickname if it is myself. */
	if (strcmp(origin->nickname, s->nickname) == 0)
		strlcpy(s->nickname, ev->args[2], sizeof (s->nickname));
}

static void
convert_notice(struct irc_server *s, struct irc_event *ev)
{
	ev->type = IRC_EVENT_NOTICE;
	ev->server = s;
	ev->notice.origin = ev->args[0];
	ev->notice.channel = ev->args[2];
	ev->notice.message = ev->args[3];
}

static void
convert_topic(struct irc_server *s, struct irc_event *ev)
{
	ev->type = IRC_EVENT_TOPIC;
	ev->server = s;
	ev->topic.origin = ev->args[0];
	ev->topic.channel = ev->args[2];
	ev->topic.topic = ev->args[3];
}

static void
convert_ping(struct irc_server *s, struct irc_event *ev)
{
	irc_server_send(s, "PONG %s", ev->args[0]);
}

static void
convert_names(struct irc_server *s, struct irc_event *ev)
{
	(void)s;
	(void)ev;
#if 0
	struct irc_server_channel *chan;
	char *p, *n;

	if (m->argsz < 3 || !(chan = irc_server_find(s, m->args[2])))
		return;

	/*
	 * Message arguments are as following:
	 * 0------- 1 2------- 3--------------------
	 * yourself = #channel nick1 nick2 nick3 ...
	 */
	for (p = m->args[3]; p; p = n ? n + 1 : NULL) {
		if ((n = strpbrk(p, " ")))
			*n = 0;

		channel_add(chan, s, p);
	}
#endif
}

static const struct convert {
	const char *command;
	void (*convert)(struct irc_server *, struct irc_event *);
} converters[] = {
	/* Must be kept ordered. */
	{ "001",        convert_connect          },
	{ "005",        convert_support          },
	{ "353",        convert_names            },
	{ "JOIN",       convert_join             },
	{ "KICK",       convert_kick             },
	{ "MODE",       convert_mode             },
	{ "NICK",       convert_nick             },
	{ "NOTICE",     convert_notice           },
	{ "PART",       convert_part             },
	{ "PING",       convert_ping             },
	{ "PRIVMSG",    convert_msg              },
	{ "TOPIC",      convert_topic            }
};

static int
compare_converter(const void *d1, const void *d2)
{
	return strcmp(d1, ((const struct convert *)d2)->command);
}

static void
convert(struct irc_server *s, struct irc_event *ev)
{
	const struct convert *c = bsearch(ev->args[1], converters, IRC_UTIL_SIZE(converters),
	    sizeof (*c), &(compare_converter));

	if (c)
		c->convert(s, ev);
}

static inline bool
scan(struct irc_event *ev, const char **line)
{
	const char *p = *line;
	size_t i = 0;

	/* Copy argument. */
	while (i < IRC_MESSAGE_MAX && *p && !isspace(*p))
		ev->args[ev->argsz][i++] = *p++;

	/* Skip optional spaces. */
	while (*p && isspace(*p))
		++p;

	if (i >= IRC_MESSAGE_MAX)
		return false;

	*line = p;
	ev->argsz++;

	return true;
}

static void
parse(struct irc_server *s, struct irc_event *ev, const char *line)
{
	if (!*line || *line++ != ':')
		return;
	if (!scan(ev, &line))   /* Prefix */
		return;
	if (!scan(ev, &line))   /* Command */
		return;

	/* Arguments. */
	while (*line && ev->argsz < IRC_ARGS_MAX) {
		/* Last argument: read until end. */
		if (*line == ':') {
			strlcpy(ev->args[ev->argsz++], ++line, IRC_MESSAGE_MAX);
			break;
		}

		if (!scan(ev, &line))
			return;
	}

	convert(s, ev);
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

#if defined(IRCCD_HAVE_SSL)
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

	if ((ret = getaddrinfo(s->host, service, &hints, &s->ai)) != 0)
		irc_log_warn("server %s: %s", s->name, gai_strerror(ret));

	s->aip = s->ai;

	return true;
}

static void
auth(struct irc_server *s)
{
	s->state = IRC_SERVER_STATE_CONNECTED;

	irc_server_send(s, "NICK %s", s->nickname);
	irc_server_send(s, "USER %s %s %s :%s", s->username,
		s->username, s->username, s->realname);
	/* TODO: server password as well. */
}

#if defined(IRCCD_HAVE_SSL)

static void
secure_update(struct irc_server *s, int ret)
{
	(void)s;
	(void)ret;

	assert(s);

	int r;

	if (!(s->flags & SERVER_FL_SSL))
		return;

	switch ((r = SSL_get_error(s->ssl, ret))) {
	case SSL_ERROR_WANT_READ:
		s->ssl_state = SERVER_SSL_NEED_READ;
		break;
	case SSL_ERROR_WANT_WRITE:
		s->ssl_state = SERVER_SSL_NEED_WRITE;
		break;
	case SSL_ERROR_SSL:
		clear(s);
		break;
	default:
		s->ssl_state = SERVER_SSL_NONE;
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
#if defined(IRCCD_HAVE_SSL)
		int r;

		s->state = SERVER_ST_HANDSHAKING;

		if ((r = SSL_do_handshake(s->ssl)) > 0)
			auth(s);

		secure_update(s, r);
#endif
	}
}

static void
secure_connect(struct irc_server *s)
{
	assert(s);

	if (!(s->flags & IRC_SERVER_FLAGS_SSL))
		handshake(s);
	else {
#if defined(IRCCD_HAVE_SSL)
		int r;

		if (!s->ctx)
			s->ctx = SSL_CTX_new(TLS_method());
		if (!s->ssl) {
			s->ssl = SSL_new(s->ctx);
			SSL_set_fd(s->ssl, s->fd);
		}

		if ((r = SSL_connect(s->ssl)) > 0)
			ssl_handshake(s);

		secure_update(s, r);
#endif
	}
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

		s->fd = socket(s->aip->ai_family, s->aip->ai_socktype,
		    s->aip->ai_protocol);

		if (s->fd < 0) {
			irc_log_warn("server %s: %s", s->name, strerror(errno));
			continue;
		}

		/* TODO: is F_GETFL required before? */
		fcntl(s->fd, F_SETFL, O_NONBLOCK);

		/*
		 * With some luck, the connection completes immediately,
		 * otherwise we will need to wait until the socket is writable.
		 */
		if (connect(s->fd, s->aip->ai_addr, s->aip->ai_addrlen) == 0) {
			secure_connect(s);
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

static void
input(struct irc_server *s)
{
	char buf[IRC_MESSAGE_MAX] = {0};
	ssize_t nr = 0;

	if (s->flags & IRC_SERVER_FLAGS_SSL) {
#if defined(IRCCD_HAVE_SSL)
		nr = SSL_read(s->ssl, buf, sizeof (buf) - 1);
		secure_update(s, nr);
#endif
	} else {
		if ((nr = recv(s->fd, buf, sizeof (buf) - 1, 0)) < 0)
			clear(s);
	}

	if (nr > 0) {
		if (strlcat(s->in, buf, sizeof (s->in)) >= sizeof (s->in)) {
			irc_log_warn("server %s: input buffer too long", s->name);
			clear(s);
		}
	}
}

static void
output(struct irc_server *s)
{
	ssize_t ns = 0;

	if (s->flags & IRC_SERVER_FLAGS_SSL) {
#if defined(IRCCD_HAVE_SSL)
		ns = SSL_write(s->ssl, s->out.data, s->out.size);
		secure_update(s, ns);
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

#if defined(IRCCD_HAVE_SSL)
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
#if defined(IRCCD_HAVE_SSL)
	if (s->flags & IRC_SERVER_FLAGS_SSL && s->ssl_state) {
		switch (s->ssl_state) {
		case SERVER_SSL_NEED_READ:
			pfd->events |= POLLIN;
			break;
		case SERVER_SSL_NEED_WRITE:
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
#if defined(IRCCD_HAVE_SSL)
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
		secure_connect(s);
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
	if (pfd->revents & POLLERR || pfd->revents & POLLHUP)
		clear(s);
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
	if (io_table[s->state].flush)
		io_table[s->state].flush(s, pfd);
}

bool
irc_server_poll(struct irc_server *s, struct irc_event *ev)
{
	assert(s);
	assert(ev);

	char *pos;
	size_t length;

	if (!(pos = strstr(s->in, "\r\n")))
		return false;

	/* Turn end of the string at delimiter. */
	*pos = 0;
	length = pos - s->in;

	/* Clear event in case we don't understand this message. */
	memset(ev, 0, sizeof (*ev));
	ev->type = IRC_EVENT_UNKNOWN;

	if (length > 0)
		parse(s, ev, s->in);

	memmove(s->in, pos + 2, sizeof (s->in) - (length + 2));

	return true;
}

struct irc_server_channel *
irc_server_find(struct irc_server *s, const char *name)
{
	assert(s);
	assert(name);

	struct irc_server_channel key = {0};

	strlcpy(key.name, name, sizeof (key.name));

	return bsearch(&key, s->channels, s->channelsz, sizeof (key), compare_chan);
}

bool
irc_server_send(struct irc_server *s, const char *fmt, ...)
{
	assert(s);
	assert(fmt);

	char buf[sizeof (s->out)];
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
irc_server_join(struct irc_server *s, const char *name, const char *pass)
{
	assert(s);
	assert(name);

	struct irc_server_channel *ch;
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
irc_server_part(struct irc_server *s, const char *name, const char *reason)
{
	assert(s);
	assert(name);

	bool ret;

	if (reason && strlen(reason) > 0)
		ret = irc_server_send(s, "PART %s :%s", name, reason);
	else
		ret = irc_server_send(s, "PART %s", name);

	return ret;
}

bool
irc_server_topic(struct irc_server *s, const char *name, const char *topic)
{
	assert(s);
	assert(name);
	assert(topic);

	return irc_server_send(s, "TOPIC %s :%s", name, topic);
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

void
irc_server_finish(struct irc_server *s)
{
	assert(s);

	clear(s);
	free(s->channels);
	memset(s, 0, sizeof (*s));
}
