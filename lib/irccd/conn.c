/*
 * conn.c -- abstract IRC server connection
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

#include <config.h>

#include <sys/socket.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

#include "conn.h"
#include "irccd.h"
#include "server.h"
#include "util.h"

#define TIMEOUT_PING    300
#define TIMEOUT_CONNECT 10

static void start(struct irc__conn *, enum irc__conn_state);
static void stop(struct irc__conn *);
static void retry(struct irc__conn *);

static void do_connecting(struct irc__conn *);
static void do_ready(struct irc__conn *, int);
static void do_handshaking(struct irc__conn *);

static void
timer_cb(struct ev_loop *loop, struct ev_timer *self, int revents)
{
	(void)revents;

	struct irc__conn *conn;

	conn = IRC_CONTAINER_OF(self, struct irc__conn, timer_fd);

	ev_timer_stop(loop, self);

	switch (conn->state) {
	case IRC__CONN_STATE_CONNECTING:
	case IRC__CONN_STATE_HANDSHAKING:
		/*
		 * Timer expired while attempting to connect or to handshake
		 * SSL status. We try the next endpoint if possible.
		 */
		retry(conn);
		break;
	case IRC__CONN_STATE_READY:
		/*
		 * Unrecoverable status as the IRC server didn't sent any
		 * message in a period of time.
		 */
		stop(conn);
		errno = ETIMEDOUT;
		conn->on_disconnect(conn);
		break;
	default:
		break;
	}
}

static void
io_cb(struct ev_loop *loop, struct ev_io *self, int revents)
{
	(void)loop;

	struct irc__conn *conn;

	conn = IRC_CONTAINER_OF(self, struct irc__conn, io_fd);

	switch (conn->state) {
	case IRC__CONN_STATE_CONNECTING:
		do_connecting(conn);
		break;
	case IRC__CONN_STATE_HANDSHAKING:
		do_handshaking(conn);
		break;
	case IRC__CONN_STATE_READY:
		do_ready(conn, revents);
		break;
	default:
		break;
	}
}

static inline void
scan(char **line, char **str)
{
	char *p = strchr(*line, ' ');

	if (p)
		*p = '\0';

	*str = *line;
	*line = p ? p + 1 : strchr(*line, '\0');
}

static int
parse(struct irc__conn_msg *msg, const char *line)
{
	char *ptr = msg->buf;
	size_t a;

	memset(msg, 0, sizeof (*msg));
	irc_util_strlcpy(msg->buf, line, sizeof (msg->buf));

	/*
	 * IRC message is defined as following:
	 *
	 * [:prefix] command arg1 arg2 [:last-argument]
	 */
	if (*ptr == ':')
		scan((++ptr, &ptr), &msg->prefix);     /* prefix */

	scan(&ptr, &msg->cmd);                         /* command */

	/* And finally arguments. */
	for (a = 0; *ptr && a < IRC_UTIL_SIZE(msg->args); ++a) {
		if (*ptr == ':') {
			msg->args[a] = ptr + 1;
			ptr = strchr(ptr, '\0');
		} else
			scan(&ptr, &msg->args[a]);
	}

	if (a >= IRC_UTIL_SIZE(msg->args))
		return errno = EMSGSIZE, -1;
	if (msg->cmd == NULL)
		return errno = EBADMSG, -1;

	return 0;
}

static int
create(struct irc__conn *conn)
{
	struct addrinfo *ai = conn->aip;
	int cflags = 0;

	if ((conn->fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol)) < 0)
		return -1;
	if ((cflags = fcntl(conn->fd, F_GETFL)) < 0)
		return -1;
	if (fcntl(conn->fd, F_SETFL, cflags | O_NONBLOCK) < 0)
		return -1;

	return 0;
}

static inline void
set_events(struct irc__conn *conn, int flags)
{
	assert(ev_is_active(&conn->io_fd));

	if (flags != conn->io_fd.events) {
		ev_io_stop(irc_bot_loop(), &conn->io_fd);
		ev_io_modify(&conn->io_fd, flags);
		ev_io_start(irc_bot_loop(), &conn->io_fd);
	}
}

#if defined(IRCCD_WITH_SSL)

static inline int
set_events_ssl(struct irc__conn *conn, int ret)
{
}

static inline ssize_t
input_ssl(struct irc__conn *conn, char *dst, size_t dstsz)
{
	int nr;

	nr = SSL_read(conn->ssl, dst, dstsz);

	return set_events_ssl(conn, nr);
}

#endif

static inline ssize_t
input_clear(struct irc__conn *conn, char *buf, size_t bufsz)
{
	ssize_t nr;

	while ((nr = recv(conn->fd, buf, bufsz, 0)) < 0 && errno == EINTR)
		continue;

	if (nr < 0) {
		if (errno == EWOULDBLOCK || errno == EAGAIN)
			nr = 0;
	} else if (nr == 0) {
		errno = ECONNRESET;
		nr = -1;
	}

	return nr;
}

static int
input(struct irc__conn *conn)
{
	size_t len = strlen(conn->in);
	size_t cap = sizeof (conn->in) - len - 1;
	ssize_t nr = 0;

#if defined(IRCCD_WITH_SSL)
	if (conn->flags & IRC__CONN_SSL)
		nr = input_ssl(conn, conn->in + len, cap);
	else
#endif
		nr = input_clear(conn, conn->in + len, cap);

	if (nr > 0)
		conn->in[len + nr] = '\0';

	return nr;
}

#if defined(IRCCD_WITH_SSL)

static inline ssize_t
output_ssl(struct irc__conn *conn)
{
	int ns;

	ns = SSL_write(conn->ssl, conn->out, strlen(conn->out));

	return set_events_ssl(conn, ns);
}

#endif

static inline ssize_t
output_clear(struct irc__conn *conn)
{
	ssize_t ns;

	while ((ns = send(conn->fd, conn->out, strlen(conn->out), 0)) < 0 && errno != EINTR)
		continue;

	if (ns < 0 && (errno == EWOULDBLOCK || errno == EAGAIN))
		ns = 0;

	return ns;
}

static int
output(struct irc__conn *conn)
{
	ssize_t ns = 0;

	if (strlen(conn->out) == 0)
		return 0;

#if defined(IRCCD_WITH_SSL)
	if (conn->flags & IRC__CONN_SSL)
		ns = output_ssl(conn);
	else
#endif
		ns = output_clear(conn);

	if (ns > 0) {
		/* Optimize if everything was sent. */
		if ((size_t)ns >= sizeof (conn->out) - 1)
			conn->out[0] = '\0';
		else
			memmove(conn->out, conn->out + ns, sizeof (conn->out) - ns);
	}

	return ns;
}

static void
start(struct irc__conn *conn, enum irc__conn_state state)
{
	switch ((conn->state = state)) {
	case IRC__CONN_STATE_CONNECTING:
		assert(!ev_is_active(&conn->timer_fd));
		assert(!ev_is_active(&conn->io_fd));

		ev_timer_init(&conn->timer_fd, timer_cb, TIMEOUT_CONNECT, 0.0);
		ev_timer_start(irc_bot_loop(), &conn->timer_fd);

		/* Connect in progress, wait for writable condition. */
		ev_io_init(&conn->io_fd, io_cb, conn->fd, EV_WRITE);
		ev_io_start(irc_bot_loop(), &conn->io_fd);
		break;
	case IRC__CONN_STATE_HANDSHAKING:
		assert(ev_is_active(&conn->timer_fd));
		assert(ev_is_active(&conn->io_fd));

		/*
		 * OpenSSL handshake started, wait for conditions that will be
		 * set in do_handshaking.
		 */
		ev_timer_again(irc_bot_loop(), &conn->timer_fd);
		do_handshaking(conn);
		break;
	case IRC__CONN_STATE_READY:
		assert(ev_is_active(&conn->io_fd));

		ev_timer_stop(irc_bot_loop(), &conn->timer_fd);
		ev_timer_set(&conn->timer_fd, TIMEOUT_PING, TIMEOUT_PING);
		ev_timer_start(irc_bot_loop(), &conn->timer_fd);

		/* start with read condition. */
		set_events(conn, EV_READ);

		/* indicate user of readiness. */
		conn->on_connect(conn);
		break;
	default:
		break;
	}
}

static int
dequeue(struct irc__conn *conn, struct irc__conn_msg *msg)
{
	char *pos;
	size_t length;

	if (!(pos = strstr(conn->in, "\r\n")))
		return 0;

	/* Turn end of the string at delimiter. */
	*pos = 0;
	length = pos - conn->in;

	if (length > 0)
		parse(msg, conn->in);

	/* (Re)move the first message received. */
	memmove(conn->in, pos + 2, sizeof (conn->in) - (length + 2));

	return 1;
}


/*
 * Check if the connection completed.
 *
 * This callback is invoked when connect() has started but not completed
 * immediately. The condition must be writable.
 */
static void
do_connecting(struct irc__conn *conn)
{
	int res, err = -1;
	socklen_t len = sizeof (int);

	/*
	 * Determine if the non blocking connect(2) call succeeded, otherwise
	 * we re-try again a connect to the next endpoint.
	 */
	if ((res = getsockopt(conn->fd, SOL_SOCKET, SO_ERROR, &err, &len)) < 0 || err)
		retry(conn);
	else
		start(conn, IRC__CONN_STATE_HANDSHAKING);
}

/*
 * Attempt to connect to current conn->aip asynchronously otherwise the socket
 * event callback will try the next conn->aip value unless we have reached the
 * limit.
 */
static void
dial(struct irc__conn *conn)
{
	for (; conn->aip; conn->aip = conn->aip->ai_next) {
		/* Attempt to initialize a socket with the current conn->aip. */
		if (create(conn) < 0)
			continue;

		/*
		 * With some luck, the connection completes immediately,
		 * otherwise we will need to wait until the socket is writable.
		 */
		if (connect(conn->fd, conn->aip->ai_addr, conn->aip->ai_addrlen) == 0) {
			start(conn, IRC__CONN_STATE_HANDSHAKING);
			break;
		}

		if (errno == EINPROGRESS || errno == EAGAIN) {
			/* Connect "succeeds" but isn't complete yet. */
			start(conn, IRC__CONN_STATE_CONNECTING);
			break;
		}
	}

	/* No more address available. */
	if (conn->aip == NULL) {
		errno = EHOSTUNREACH;
		conn->on_disconnect(conn);
	}
}

static int
lookup(struct irc__conn *conn)
{
	struct addrinfo hints = {
		.ai_socktype = SOCK_STREAM,
		.ai_flags = AI_NUMERICSERV
	};
	char service[16];
	int rc;

	snprintf(service, sizeof (service), "%hu", conn->port);

	if ((rc = getaddrinfo(conn->hostname, service, &hints, &conn->ai)) != 0) {
		errno = EHOSTUNREACH;
		rc = -1;
	} else
		conn->aip = conn->ai;

	return rc;
}

struct irc__conn *
irc__conn_new(void)
{
	struct irc__conn *conn;

	conn = irc_util_calloc(1, sizeof (*conn));
	conn->fd = -1;

	return conn;
}

void
irc__conn_connect(struct irc__conn *conn)
{
	assert(conn);

	/* Start with no state as getaddrinfo could even fail. */
	conn->state = IRC__CONN_STATE_NONE;

#if !defined(IRCCD_WITH_SSL)
	if (conn->flags & IRC_CONN_SSL) {
		irc_log_warn("server %s: SSL requested but not available", conn->sv->name);
		errno = EINVAL;
		return -1;
	}
#endif

	/* Fetch DNS information. */
	if (lookup(conn) < 0)
		conn->on_disconnect(conn);
	else
		dial(conn);
}

void
irc__conn_disconnect(struct irc__conn *conn)
{
	assert(conn);

	stop(conn);

	if (conn->aip) {
		freeaddrinfo(conn->aip);
		conn->aip = NULL;
	}
}

int
irc__conn_push(struct irc__conn *conn, const char *data)
{
	assert(conn);
	assert(data);

	if (irc_util_strlcat(conn->out, data, sizeof (conn->out)) >= sizeof (conn->out)) {
		errno = EMSGSIZE;
		return -1;
	}
	if (irc_util_strlcat(conn->out, "\r\n", sizeof (conn->out)) >= sizeof (conn->out)) {
		errno = EMSGSIZE;
		return -1;
	}

	if (conn->out[0])
		set_events(conn, EV_READ | EV_WRITE);

	return 0;
}

void
irc__conn_free(struct irc__conn *conn)
{
	assert(conn);
	assert(conn->fd != -1);

	if (conn->aip != NULL)
		freeaddrinfo(conn->aip);

	free(conn);
}
