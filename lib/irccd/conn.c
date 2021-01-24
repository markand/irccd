/*
 * conn.c -- an IRC server channel
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

#include <sys/socket.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <poll.h>
#include <string.h>
#include <unistd.h>

#include "conn.h"
#include "util.h"

static void
cleanup(struct irc_conn *conn)
{
	if (conn->fd != 0)
		close(conn->fd);

#if defined(IRCCD_WITH_SSL)
	if (conn->ssl)
		SSL_free(conn->ssl);
	if (conn->ctx)
		SSL_CTX_free(conn->ctx);

	conn->ssl = NULL;
	conn->ctx = NULL;
#endif

	conn->fd = -1;
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

static bool
parse(struct irc_conn_msg *msg, const char *line)
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
	for (a = 0; *ptr && a < IRC_UTIL_SIZE(msg->args); ++a) {
		if (*ptr == ':') {
			msg->args[a] = ptr + 1;
			ptr = strchr(ptr, '\0');
		} else
			scan(&ptr, &msg->args[a]);
	}

	if (a >= IRC_UTIL_SIZE(msg->args))
		return errno = EMSGSIZE, false;
	if (msg->cmd == NULL)
		return errno = EBADMSG, false;

	return true;
}

static bool
create(struct irc_conn *conn)
{
	struct addrinfo *ai = conn->aip;
	int cflags = 0;

	cleanup(conn);

	if ((conn->fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol)) < 0)
		return false;
	if ((cflags = fcntl(conn->fd, F_GETFL)) < 0)
		return false;
	if (fcntl(conn->fd, F_SETFL, cflags | O_NONBLOCK) < 0)
		return false;

	return true;
}

static inline bool
update_ssl_state(struct irc_conn *conn, int ret)
{
	switch (SSL_get_error(conn->ssl, ret)) {
	case SSL_ERROR_WANT_READ:
		conn->ssl_state = IRC_CONN_SSL_STATE_NEED_READ;
		break;
	case SSL_ERROR_WANT_WRITE:
		conn->ssl_state = IRC_CONN_SSL_STATE_NEED_WRITE;
		break;
	case SSL_ERROR_SSL:
		return irc_conn_disconnect(conn), -1;
	default:
		break;
	}

	return true;
}

static ssize_t
input_ssl(struct irc_conn *conn, char *dst, size_t dstsz)
{
	int nr;

	if ((nr = SSL_read(conn->ssl, dst, dstsz)) <= 0)
		return update_ssl_state(conn, nr);

	conn->ssl_state = IRC_CONN_SSL_STATE_NONE;

	return nr;
}

static inline ssize_t
input_clear(struct irc_conn *conn, char *buf, size_t bufsz)
{
	ssize_t nr;

	if ((nr = recv(conn->fd, buf, bufsz, 0)) <= 0)
		return irc_conn_disconnect(conn), -1;

	return nr;
}

static bool
input(struct irc_conn *conn)
{
	size_t len = strlen(conn->in);
	size_t cap = sizeof (conn->in) - len - 1;
	ssize_t nr = 0;

	if (conn->flags & IRC_CONN_SSL)
		nr = input_ssl(conn, conn->in + len, cap);
	else
		nr = input_clear(conn, conn->in + len, cap);

	if (nr > 0)
		conn->in[len + nr] = '\0';

	return nr > 0;
}

static inline ssize_t
output_ssl(struct irc_conn *conn)
{
	int ns;

	if ((ns = SSL_write(conn->ssl, conn->out, strlen(conn->out))) <= 0)
		return update_ssl_state(conn, ns);

	return ns;
}

static inline ssize_t
output_clear(struct irc_conn *conn)
{
	ssize_t ns;

	if ((ns = send(conn->fd, conn->out, strlen(conn->out), 0)) < 0)
		return irc_conn_disconnect(conn), -1;

	return ns;
}

static bool
output(struct irc_conn *conn)
{
	ssize_t ns = 0;

	if (conn->flags & IRC_CONN_SSL)
		ns = output_ssl(conn);
	else
		ns = output_clear(conn);

	if (ns > 0) {
		/* Optimize if everything was sent. */
		if ((size_t)ns >= sizeof (conn->out) - 1)
			conn->out[0] = '\0';
		else
			memmove(conn->out, conn->out + ns, sizeof (conn->out) - ns);
	}

	return ns != -1;
}

static bool
handshake(struct irc_conn *conn)
{
	if (conn->flags & IRC_CONN_SSL) {
#if defined(IRCCD_WITH_SSL)
		int r;

		conn->state = IRC_CONN_STATE_HANDSHAKING;

		/*
		 * This function is called several time until it completes so we
		 * must keep the same context/ssl stuff once it has been
		 * created.
		 */
		if (!conn->ctx)
			conn->ctx = SSL_CTX_new(TLS_method());
		if (!conn->ssl) {
			conn->ssl = SSL_new(conn->ctx);

			SSL_set_fd(conn->ssl, conn->fd);
			SSL_set_connect_state(conn->ssl);
		}

		if ((r = SSL_do_handshake(conn->ssl)) <= 0)
			return update_ssl_state(conn, r);

		conn->state = IRC_CONN_STATE_READY;
#endif
	} else
		conn->state = IRC_CONN_STATE_READY;

	return true;
}

static bool
dial(struct irc_conn *conn)
{
	/* No more address available. */
	if (conn->aip == NULL)
		return irc_conn_disconnect(conn), false;

	for (; conn->aip; conn->aip = conn->aip->ai_next) {
		if (!create(conn)) {
			// irc_log_warn("server %s: %s", s->name, strerror(errno));
			continue;
		}

		/*
		 * With some luck, the connection completes immediately,
		 * otherwise we will need to wait until the socket is writable.
		 */
		if (connect(conn->fd, conn->aip->ai_addr, conn->aip->ai_addrlen) == 0)
			return handshake(conn);

		/* Connect "succeeds" but isn't complete yet. */
		if (errno == EINPROGRESS || errno == EAGAIN)
			return true;
	}

	return false;
}

static bool
lookup(struct irc_conn *conn)
{
	struct addrinfo hints = {
		.ai_socktype = SOCK_STREAM,
		.ai_flags = AI_NUMERICSERV
	};
	char service[16];
	int ret;

	snprintf(service, sizeof (service), "%hu", conn->port);

	if ((ret = getaddrinfo(conn->hostname, service, &hints, &conn->ai)) != 0) {
		// irc_log_warn gai_strerror(ret)
		return false;
	}

	conn->aip = conn->ai;

	return true;
}

static bool
check_connect(struct irc_conn *conn)
{
	int res, err = -1;
	socklen_t len = sizeof (int);

	/* Determine if the non blocking connect(2) call succeeded. */
	if ((res = getsockopt(conn->fd, SOL_SOCKET, SO_ERROR, &err, &len)) < 0 || err)
		return dial(conn);

	return handshake(conn);
}

bool
irc_conn_connect(struct irc_conn *conn)
{
	assert(conn);

	conn->state = IRC_CONN_STATE_CONNECTING;

	if (!lookup(conn))
		return irc_conn_disconnect(conn), false;

	return dial(conn), true;
}

void
irc_conn_disconnect(struct irc_conn *conn)
{
	assert(conn);

	cleanup(conn);
	conn->state = IRC_CONN_STATE_NONE;
}

static inline void
prepare_ssl(const struct irc_conn *conn, struct pollfd *pfd)
{
#if defined(IRCCD_WITH_SSL)
	switch (conn->ssl_state) {
	case IRC_CONN_SSL_STATE_NEED_READ:
		pfd->events |= POLLIN;
		break;
	case IRC_CONN_SSL_STATE_NEED_WRITE:
		pfd->events |= POLLOUT;
		break;
	default:
		break;
	}
#else
	(void)conn;
#endif
}

void
irc_conn_prepare(const struct irc_conn *conn, struct pollfd *pfd)
{
	assert(conn);
	assert(pfd);

	pfd->fd = conn->fd;

	if (conn->ssl_state)
		prepare_ssl(conn, pfd);
	else {
		switch (conn->state) {
		case IRC_CONN_STATE_CONNECTING:
			pfd->events = POLLOUT;
			break;
		case IRC_CONN_STATE_READY:
			pfd->events = POLLIN;

			if (conn->out[0])
				pfd->events |= POLLOUT;
			break;
		default:
			break;
		}
	}
}

bool
irc_conn_flush(struct irc_conn *conn, const struct pollfd *pfd)
{
	assert(conn);
	assert(pfd);

	switch (conn->state) {
	case IRC_CONN_STATE_CONNECTING:
		return check_connect(conn);
	case IRC_CONN_STATE_HANDSHAKING:
		return handshake(conn);
	case IRC_CONN_STATE_READY:
		if (pfd->revents & (POLLERR | POLLHUP))
			return irc_conn_disconnect(conn), false;
		if (pfd->revents & POLLIN && !input(conn))
			return irc_conn_disconnect(conn), false;
		if (pfd->revents & POLLOUT && !output(conn))
			return irc_conn_disconnect(conn), false;
		break;
	default:
		break;
	}

	return true;
}

bool
irc_conn_poll(struct irc_conn *conn, struct irc_conn_msg *msg)
{
	assert(conn);
	assert(msg);

	char *pos;
	size_t length;

	if (!(pos = strstr(conn->in, "\r\n")))
		return false;

	/* Turn end of the string at delimiter. */
	*pos = 0;
	length = pos - conn->in;

	if (length > 0)
		parse(msg, conn->in);

	memmove(conn->in, pos + 2, sizeof (conn->in) - (length + 2));

	return true;
}

bool
irc_conn_send(struct irc_conn *conn, const char *data)
{
	assert(conn);
	assert(data);

	if (strlcat(conn->out, data, sizeof (conn->out)) >= sizeof (conn->out))
		return errno = EMSGSIZE, false;
	if (strlcat(conn->out, "\r\n", sizeof (conn->out)) >= sizeof (conn->out))
		return errno = EMSGSIZE, false;

	return true;
}

void
irc_conn_finish(struct irc_conn *conn)
{
	assert(conn);
}
