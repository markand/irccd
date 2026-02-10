/*
 * conn.c -- private connection handler and helpers
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
#include <sys/types.h>
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

#include "config.h"

#ifdef IRCCD_WITH_SSL
#       include <tls.h>
#endif

#include "conn.h"
#include "log.h"
#include "server.h"
#include "util.h"

#define RECONNECT_DELAY 30.0    /* Seconds to wait before reconnecting. */
#define CONNECT_TIMEOUT 5.0     /* Seconds before marking a server as dead. */
#define PING_TIMEOUT    300.0   /* Seconds after assuming ping timeout. */

#define CONN(Ptr, Field) \
        (IRC_UTIL_CONTAINER_OF(Ptr, struct conn, Field))

#define DEBUG(...) LOG(irc_log_debug, __VA_ARGS__)
#define INFO(...)  LOG(irc_log_info, __VA_ARGS__)
#define WARN(...)  LOG(irc_log_warn, __VA_ARGS__)

#define LOG(Fn, ...)                                                            \
do {                                                                            \
        char line[256] = {};                                                    \
                                                                                \
        snprintf(line, sizeof (line), __VA_ARGS__);                             \
                                                                                \
        Fn("server %s: %s", conn->parent->name, line);                          \
} while (0)

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

/*
 * Stop the io_fd coroutine and yield forever until the reconnect timer fires
 * up.
 *
 * Because this function is called within the io_fd coroutine, it can't destroys
 * itself so we will just disable the watcher and let the state machine do the
 * cleanup.
 */
static inline void
conn_reschedule(struct conn *conn)
{
	/* Stop at least our socket watcher, finalizer will do the rest. */
	nce_io_stop(&conn->io_fd.io);

	/* Feed the watchdog timer quickly. */
	nce_timer_feed(&conn->timer.timer, EV_TIMER);
	nce_coro_idle();
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
		WARN("getaddrinfo: %s", gai_strerror(rc));
		conn_reschedule(conn);
	} else {
		conn->state = STATE_CONNECT;

		for (const struct addrinfo *ai = conn->ai_list; ai; ai = ai->ai_next)
			DEBUG("resolves to %s", conn_info(conn, ai));

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
		WARN("socket: %s", strerror(errno));
		return -1;
	}
	if ((flags = fcntl(conn->fd, F_GETFL)) < 0 || fcntl(conn->fd, F_SETFL, flags | O_NONBLOCK) < 0) {
		WARN("fcntl: %s", strerror(errno));
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
			WARN("connect: %s", strerror(errno));
			goto exit;
		}

		/*
		 * When connection is in progress the socket will be writable
		 * once connection is complete or error'ed.
		 */
		DEBUG("connect in progress");
		nce_io_reset(&conn->io_fd.io, conn->fd, EV_WRITE);
		nce_io_wait(&conn->io_fd.io);
		nce_io_stop(&conn->io_fd.io);

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
		DEBUG("connect succeeded");

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

#ifdef IRCCD_WITH_SSL
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
	nce_io_reset(&conn->io_fd.io, conn->fd, EV_READ);
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
			WARN("recv: %s", strerror(errno));
	} else if (nr == 0) {
		WARN("remote closed connection");
		nr = -1;
	}

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
			WARN("send: %s", strerror(errno));
	} else if ((size_t)ns < bufsz)
		*events |= EV_WRITE;

	return ns;
}

#ifdef IRCCD_WITH_SSL

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

/*
 * Wait for socket activity.
 */
static int
conn_wait(struct conn *conn)
{
	int events, revents, rc;

	revents = nce_io_wait(&conn->io_fd.io);
	events = EV_READ;

	if ((revents & EV_READ) && (rc = conn_recv(conn, &events)) < 0)
		return rc;
	if ((revents & EV_WRITE) && (rc = conn_send(conn, &events)) < 0)
		return rc;

	if (events != conn->io_fd.io.io.events)
		nce_io_reset(&conn->io_fd.io, conn->fd, events);

	return 0;
}

/*
 * Parse a raw IRC incoming message from the internal input buffer.
 */
static int
conn_next(struct conn *conn, struct conn_msg *msg)
{
	size_t length;
	char *pos;

	if (!(pos = memmem(conn->in, conn->insz, "\r\n", 2)))
		return 0;

	length = pos - conn->in;

	if (length > 0 && length < IRCCD_MESSAGE_LEN)
		irc__conn_msg_parse(msg, conn->in, length);

	/* (Re)move the first message received. */
	memmove(conn->in, pos + 2, sizeof (conn->in) - (length + 2));
	conn->insz -= length + 2;

	return 1;
}

/*
 * Exchange until we discover a proper IRC server.
 */
static void
conn_ident(struct conn *conn)
{
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

	/* Wait until fully sent. */
	while (conn->outsz && (rc = conn_wait(conn)) == 0)
		continue;

	if (rc == 0)
		conn->state = STATE_READY;
	else
		conn_reschedule(conn);
}

/*
 * Loop until something wrong appears.
 */
static void
conn_ready(struct conn *conn)
{
	while (conn_wait(conn) == 0)
		nce_timer_again(&conn->timer.timer);

	/*
	 * On disconnect, we empty the IRC buffer and add a custom "000" event
	 * so that conn_next gets notified.
	 */
	WARN("connection lost");
	conn->insz = snprintf(conn->in, sizeof (conn->in), ":internal 000\r\n");

	/* Yield until reconnect. */
	conn_reschedule(conn);
}

/*
 * This io_fd coroutine does all the magic regarding connection, IRC
 * identification and handshaking.
 *
 * By itself it only fills and drain input/output buffer but does not generate
 * events which is left to the producer coroutine.
 */
static void
conn_io_entry(struct nce_coro *self)
{
	struct conn *conn = CONN(self, io_fd.coro);

	conn->state = STATE_RESOLVE;

	for (;;) {
		/* Rearm timer */
		nce_timer_again(&conn->timer.timer);

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
conn_io_finalizer(struct nce_coro *self)
{
	struct conn *conn = CONN(self, io_fd.coro);

	if (conn->ai_list) {
		freeaddrinfo(conn->ai_list);
		conn->ai_list = NULL;
		conn->ai = NULL;
	}

#ifdef IRCCD_WITH_SSL
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
	conn->io_fd.coro.name = "conn.io_fd";
	conn->io_fd.coro.flags = NCE_INACTIVE;
	conn->io_fd.coro.priority = 0;
	conn->io_fd.coro.entry = conn_io_entry;
	conn->io_fd.coro.finalizer = conn_io_finalizer;
	nce_io_coro_spawn(&conn->io_fd, 0, 0);
}

static void
conn_timer_resurrect(struct conn *conn)
{
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

	nce_coro_destroy(&conn->io_fd.coro);

	/* Now reschedule ourself to reconnect later. */
	nce_timer_stop(&conn->timer.timer);
	nce_timer_set(&conn->timer.timer, RECONNECT_DELAY, 0.0);
	nce_timer_start(&conn->timer.timer);
	nce_timer_wait(&conn->timer.timer);
	nce_timer_stop(&conn->timer.timer);

	conn_io_spawn(conn);
}

/*
 * This timer coroutine is triggered when no activity appeared on the socket
 * before or after an IRC server has been identified.
 *
 * So each time activity occurs, the timer must be fed again to avoid
 * unneeded disconnection.
 */
static void
conn_timer_entry(struct nce_coro *self)
{
	struct conn *conn = CONN(self, timer.coro);

	while (nce_timer_wait(&conn->timer.timer))
		conn_timer_resurrect(conn);
}

static void
conn_timer_spawn(struct conn *conn)
{
	conn->timer.coro.name  = "conn.timer";
	conn->io_fd.coro.priority = 1;
	conn->timer.coro.entry = conn_timer_entry;
	nce_timer_coro_spawn(&conn->timer, 60.0, 60.0);
}

/*
 * This producer coroutine pull message from the io_fd input buffer stream and
 * pushes them into its own storage.
 *
 * Caller may retrieve those messages using ::irc__conn_pull.
 */
static void
conn_producer_entry(struct nce_coro *self)
{
	struct conn *conn;
	struct conn_msg msg;

	conn = CONN(self, producer);

	for (;;) {
		while (conn_next(conn, &msg) == 0)
			nce_coro_yield();

		nce_coro_push(&conn->producer, &msg, sizeof (msg));
	}
}

static void
conn_producer_spawn(struct conn *conn)
{
	conn->producer.name = "conn.producer";
	conn->producer.entry = conn_producer_entry;
	conn->producer.priority = 2;
	nce_coro_spawn(&conn->producer);
}

int
irc__conn_ready(const struct conn *conn)
{
	return conn && (conn->state == STATE_READY || conn->state == STATE_IDENT);
}

void
irc__conn_spawn(struct conn *conn, struct irc_server *server)
{
	assert(conn);
	assert(server);

	conn->parent = server;

#ifdef IRCCD_WITH_SSL
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

	conn_timer_spawn(conn);
	conn_io_spawn(conn);
	conn_producer_spawn(conn);
}

int
irc__conn_push(struct conn *conn, const char *data, size_t datasz)
{
	if (datasz + 2 >= sizeof (conn->out) - conn->outsz)
		return -ENOBUFS;

	memcpy(&conn->out[conn->outsz], data, datasz);
	conn->outsz += datasz;
	memcpy(&conn->out[conn->outsz], "\r\n", 2);
	conn->outsz += 2;

	nce_io_reset(&conn->io_fd.io, conn->fd, EV_READ | EV_WRITE);

	return 0;
}

void
irc__conn_pull(struct conn *conn, struct conn_msg *msg)
{
	assert(conn);
	assert(msg);

	nce_coro_pull(&conn->producer, msg, sizeof (*msg));
}

void
irc__conn_destroy(struct conn *conn)
{
	assert(conn);

	nce_coro_destroy(&conn->producer);
	nce_coro_destroy(&conn->io_fd.coro);
	nce_coro_destroy(&conn->timer.coro);
}

static inline void
irc__conn_msg_scan(char **line, char **str)
{
	char *p;

	if ((p = strchr(*line, ' ')))
		*p = '\0';

	*str = *line;
	*line = p ? p + 1 : strchr(*line, '\0');
}

int
irc__conn_msg_parse(struct conn_msg *msg, const char *line, size_t linesz)
{
	assert(msg);
	assert(line);

	char *ptr;

	memset(msg, 0, sizeof (*msg));

	ptr = msg->buf = strndup(line, linesz);

	/*
	 * IRC message is defined as following:
	 *
	 * [:prefix] command arg1 arg2 [:last-argument]
	 */
	if (*ptr == ':')
		irc__conn_msg_scan((++ptr, &ptr), &msg->prefix);

	irc__conn_msg_scan(&ptr, &msg->cmd);

	/* And finally arguments. */
	while (*ptr) {
		msg->args = irc_util_reallocarray(msg->args, msg->argsz + 1, sizeof (char *));
		msg->args[msg->argsz] = NULL;

		if (*ptr == ':') {
			msg->args[msg->argsz] = ptr + 1;
			ptr = strchr(ptr, '\0');
		} else
			irc__conn_msg_scan(&ptr, &msg->args[msg->argsz]);

		msg->argsz++;
	}

	if (msg->cmd == NULL)
		return -EBADMSG;

	return 0;
}

int
irc__conn_msg_is_ctcp(const char *line)
{
	assert(line);

	size_t length;

	if (!line)
		return 0;
	if ((length = strlen(line)) < 2)
		return 0;

	return line[0] == 0x1 && line[length - 1] == 0x1;
}

char *
irc__conn_msg_ctcp(char *line)
{
	assert(line);

	/* Skip first \x01. */
	if (*line == '\x01')
		line++;

	/* Remove last \001. */
	line[strcspn(line, "\x01")] = '\0';

	if (strncmp(line, "ACTION ", 7) == 0)
		line += 7;

	return line;
}

void
irc__conn_msg_finish(struct conn_msg *msg)
{
	assert(msg);

	free(msg->buf);
}

