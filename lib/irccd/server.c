/*
 * server.c -- an IRC server
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

#include <sys/socket.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include <ev.h>

#include <utlist.h>

#include "config.h"

#if defined(IRCCD_WITH_SSL)
#       include <openssl/err.h>
#       include <openssl/ssl.h>
#endif

#include "channel.h"
#include "event.h"
#include "irccd.h"
#include "log.h"
#include "server.h"
#include "util.h"

#define RECONNECT_DELAY 30.0    /* Seconds to wait before reconnecting. */
#define CONNECT_TIMEOUT 1800.0  /* Seconds before marking a server as dead. */

#if 0
enum conn_err {
	/*
	 * No error.
	 */
	CONN_ERR_OK,

	/*
	 * The connect() call failed, conn->errval contains errno value.
	 */
	CONN_ERR_CONNECT,

	/*
	 * The recv() call failed, conn->errval contains errno value.
	 */
	CONN_ERR_RECV,

	/*
	 * The send() call failed, conn->errval contains errno value.
	 */
	CONN_ERR_SEND,

	/*
	 * The getaddrinfo() call failed, conn->errval contains its return code.
	 */
	CONN_ERR_GETADDRINFO,

	/*
	 * A SSL function failed, conn->errval contains its return code.
	 */
	CONN_ERR_SSL,

	/*
	 * An other logic error happened, conn->errval contains a errno status.
	 */
	CONN_ERR_GENERIC
};
#endif

/*
 * Current connection state.
 */
enum conn_state {
	/*
	 * Connection state closed, conn->fd = -1.
	 */
	CONN_STATE_CLOSED,

	/*
	 * Connection is in progress.
	 */
	CONN_STATE_CONNECTING,

	/*
	 * (SSL only)
	 *
	 * SSL handshaking is in progress.
	 */
	CONN_STATE_HANDSHAKING,

	/*
	 * Connection is ready for I/O.
	 */
	CONN_STATE_READY
};

/*
 * Reason why conn notified the server through conn->async_ev.
 */
enum conn_async {
	/* Unused default value. */
	CONN_ASYNC_NONE,

	/*
	 * Connection became ready, the server will perform the initial IRC
	 * protocol identification and authentication.
	 */
	CONN_ASYNC_READY,

	/*
	 * Some data arrived in the incoming buffer and conn->insz is greater
	 * than zero.
	 *
	 * Server will dequeue the buffer with conn_dequeue.
	 */
	CONN_ASYNC_INCOMING

	/*
	 * Error happened in the connection.
	 */
	CONN_ASYNC_ERROR,
};

struct conn {
	/* Link to the parent server. */
	const struct irc_server *parent;

	/* Current connection state. */
	enum conn_state state;

	/*
	 * Pointer to current endpoint to try and pointer to the original
	 * list.
	 */
	struct addrinfo *ai;
	struct addrinfo *ai_list;

	/*
	 * Input & output buffer and their respective sizes, not NUL
	 * terminated.
	 */
	char in[IRC_BUF_LEN];
	size_t insz;
	char out[IRC_BUF_LEN];
	size_t outsz;

	/* Socket, its watcher and a connect/ping timer */
	int fd;
	struct ev_io io_fd;
	struct ev_timer timer_fd;

	/* Connection to server notification */
	struct ev_async async_ev;
	enum conn_async async_reason;

	/* OpenSSL boring stuff. */
#if defined(IRCCD_WITH_SSL)
	SSL_CTX *ctx;
	SSL *ssl;
#endif

	/* Transport callbacks */
	ssize_t (*recv)(struct conn *, void *, size_t);
	ssize_t (*send)(struct conn *, const void *, size_t);
	int     (*want)(struct conn *);
};

struct msg {
	char *prefix;
	char *cmd;
	char *args[IRC_ARGS_MAX];
	char buf[IRC_MESSAGE_LEN + 1];
};

/*
 * With the given amount of callbacks and functions required to parse and
 * exchange with the IRC server, add prototypes of very function.
 */

/* channels */

#if 0
static struct irc_channel * channels_add(struct irc_server *,
                                         const char *,
                                         const char *,
                                         enum irc_channel_flags);
static void                 channels_clear(struct irc_server *);
static void                 channels_free(struct irc_server *);
static void                 channels_remove(struct irc_server *,
                                            struct irc_channel *);
#endif

/* server */
static void server_clear(struct irc_server *);
static void server_connect(struct irc_server *);
static void server_disconnect(struct irc_server *);
static void server_free(struct irc_server *);
static void server_identify(struct irc_server *);
static void server_switch(struct irc_server *, enum irc_server_state);
static int  server_is_self_nickname(const struct irc_server *, const char *);
static int  server_find_mode(struct irc_server *, int);

/* message parsing */
static int  msg_parse(struct msg *, const char *, size_t);
static void msg_scan(char **, char **);

/* private connection functions */
static void          conn__io_cb(struct ev_loop *, struct ev_io *, int);
static void          conn__io_connecting(struct conn *, int);
static void          conn__io_handshaking(struct conn *, int);
static void          conn__io_ready(struct conn *, int);
static int           conn__lookup(struct conn *);
static int           conn__recv(struct conn *);
static int           conn__send(struct conn *);
static int           conn__socket(struct conn *);
static void          conn__timer_cb(struct ev_loop *, struct ev_timer *, int);
static void          conn__watch(struct conn *, int);

/* private connection transport with SSL */

#if defined(IRCCD_WITH_SSL)
static int           conn__tls_handshake(struct conn *);
static ssize_t       conn__tls_recv(struct conn *, void *, size_t);
static ssize_t       conn__tls_send(struct conn *, const void *, size_t);
static int           conn__tls_want(struct conn *);
#endif

/* connection transport without SSL */

static ssize_t       conn__plain_recv(struct conn *, void *, size_t);
static ssize_t       conn__plain_send(struct conn *, const void *, size_t);
static int           conn__plain_want(struct conn *);

/* public connection functions */
static struct conn * conn_new(struct irc_server *);
static void          conn_close(struct conn *);
static int           conn_dequeue(struct conn *, struct msg *);
static int           conn_push(struct conn *, const char *, size_t);
static void          conn_free(struct conn *);

/* handlers */

static void handle_connect(struct irc_server *, struct msg *);
static void handle_disconnect(struct irc_server *);
static void handle_support(struct irc_server *, struct msg *);
static void handle_invite(struct irc_server *, struct msg *);
static void handle_join(struct irc_server *, struct msg *);
static void handle_kick(struct irc_server *, struct msg *);
static void handle_mode(struct irc_server *, struct msg *);
static void handle_part(struct irc_server *, struct msg *);
static void handle_msg(struct irc_server *, struct msg *);
static void handle_nick(struct irc_server *, struct msg *);
static void handle_notice(struct irc_server *, struct msg *);
static void handle_topic(struct irc_server *, struct msg *);
static void handle_ping(struct irc_server *, struct msg *);
static void handle_names(struct irc_server *, struct msg *);
static void handle_endofnames(struct irc_server *, struct msg *);
static void handle_nicknameinuse(struct irc_server *, struct msg *);
static void handle_error(struct irc_server *, struct msg *);
static void handle_whoisuser(struct irc_server *, struct msg *);
static void handle_whoischannels(struct irc_server *, struct msg *);
static void handle_endofwhois(struct irc_server *, struct msg *);

static int              handler_cmp(const void *, const void *);
static struct handler * handler_find(const char *);
static void             handler(struct irc_server *, struct msg *);

/* {{{ message parsing */

static void
msg_scan(char **line, char **str)
{
	char *p = strchr(*line, ' ');

	if (p)
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

	if (a >= IRC_UTIL_SIZE(msg->args)) {
		errno = EMSGSIZE;
		return -1;
	}
	if (msg->cmd == NULL) {
		errno = EBADMSG;
		return -1;
	}

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

static struct irc_channel *
server_channels_add(struct irc_server *s,
                    const char *name,
                    const char *password,
                    enum irc_channel_flags flags)
{
	struct irc_channel *ch;

	if ((ch = irc_server_find(s, name)))
		ch->flags |= flags;
	else {
		ch = irc_channel_new(name, password, flags);
		LL_PREPEND(s->channels, ch);
	}

	return ch;
}

/*
 * Clear the server channels, meaning the joining state get's reset and nickname
 * list emptied.
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

/* }}} */

/* {{{ connection abstraction */

static struct conn *
conn_new(struct irc_server *parent)
{
	struct conn *conn;

	conn = irc_util_calloc(1, sizeof (*conn));
	conn->parent = parent;
	conn->fd = -1;

	if (parent->flags & IRC_SERVER_FLAGS_SSL) {
#if defined(IRCCD_WITH_SSL)
		conn->recv = conn__tls_recv;
		conn->send = conn__tls_send;
		conn->want = conn__tls_want;
#else
		abort();
#endif
	}

	ev_init(&conn->io_fd, conn__io_cb);
	ev_init(&conn->timer_fd, conn__timer_cb);

	return conn;
}

static ssize_t
conn__plain_recv(struct conn *conn, void *buf, size_t bufsz)
{
	ssize_t nr;

	while ((nr = recv(conn->fd, buf, bufsz, MSG_NOSIGNAL)) < 0 && errno == EINTR)
		continue;

	if (nr < 0) {
		/*
		 * Called with no events, we allow it and simply return no data
		 * for now.
		 */
		if (errno == EWOULDBLOCK || errno == EAGAIN)
			nr = 0;
		else {
			conn->errtype = CONN_ERR_RECV;
			conn->errval  = errno;
		}
	} else if (nr == 0) {
		conn->errtype = CONN_ERR_RECV;
		conn->errval  = ECONNRESET;
		nr = -1;
	}

	return nr;
}

static ssize_t
conn__plain_send(struct conn *conn, const void *buf, size_t bufsz)
{
	ssize_t ns;

	while ((ns = send(conn->fd, buf, bufsz, MSG_NOSIGNAL)) < 0 && errno != EINTR)
		continue;

	/*
	 * Similar to conn_plain_recv, accept a call with not able to send for
	 * now.
	 */
	if (ns < 0) {
		if (errno == EWOULDBLOCK || errno == EAGAIN)
			ns = 0;
		else {
			conn->errtype = CONN_ERR_SEND;
			conn->errval  = errno;
		}
	}

	return ns;
}

static int
conn__recv(struct conn *conn)
{
	ssize_t nr;
	size_t limit;

	limit = sizeof (conn->in) - conn->insz;

	if ((nr = conn->recv(conn, &conn->in[conn->insz], limit)) > 0)
		conn->insz += nr;

	return nr;
}

/*
 * Attempt to send the output buffer from the connection. This function should
 * usually not be called if the output buffer is empty.
 */
static int
conn__send(struct conn *conn)
{
	ssize_t ns;

	if (conn->outsz == 0)
		return 0;

	ns = conn->send(conn, conn->out, conn->outsz);

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
conn__lookup(struct conn *conn)
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

	snprintf(service, sizeof (service), "%hu", conn->parent->port);

	if ((rc = getaddrinfo(conn->parent->hostname, service, &hints, &conn->ai)) != 0) {
		conn->state   = CONN_STATE_ERROR;
		conn->errtype = CONN_ERR_GETADDRINFO;
		conn->errval  = rc;
		ev_async_send(irc_bot_loop(), &conn->async_ev);
	} else
		conn->aip = conn->ai;

	return rc;
}

static int
conn_push(struct conn *conn, const char *data, size_t datasz)
{
	assert(conn);
	assert(data);

	if (datasz + 2 >= sizeof (conn->out) - conn->outsz)
		return -1;

	memcpy(&conn->out[conn->outsz], data, datasz);
	conn->outsz += datasz;
	memcpy(&conn->out[conn->outsz], "\r\n", 2);
	conn->outsz += 2;

	/* Update watcher condition. */
	conn__watch(conn, EV_READ | EV_WRITE);

	return 0;
}

static inline void
conn_free(struct conn *conn)
{
	if (!conn)
		return;

	conn_close(conn);
	free(conn);
}

static int
conn_dequeue(struct conn *conn, struct msg *msg)
{
	char *pos;
	size_t length;

	if (!(pos = memmem(conn->in, conn->insz, "\r\n", 2)))
		return 0;

	length = pos - conn->in;

	if (length > 0 && length < IRC_MESSAGE_LEN)
		msg_parse(msg, conn->in, length);

	/* (Re)move the first message received. */
	memmove(conn->in, pos + 2, sizeof (conn->in) - (length + 2));
	conn->insz -= length + 2;

	return 1;
}
/*
 * Callback on connection timer expiry.
 */
static void
conn_timer_cb(struct ev_loop *loop, struct ev_timer *self, int revents)
{
	(void)loop;
	(void)revents;

	struct conn *conn = IRC_UTIL_CONTAINER_OF(self, struct conn, timer_fd);

	conn->state   = CONN_STATE_ERROR;
	conn->errtype = CONN_ERR_GENERIC;
	conn->errval  = ETIMEDOUT;

	conn->async_reason = CONN_ASYNC_STATE;
	ev_async_send(loop, &conn->async_ev);
}

/*
 * Check if the connection completed.
 *
 * This callback is invoked when connect() has started but not completed
 * immediately. The condition must be writable and is watched for this event
 * only.
 */
static void
conn__io_connecting(struct conn *conn, int revents)
{
	assert(revents & EV_WRITE);

	int rc = 0, err = 0;
	socklen_t len = sizeof (int);

	/*
	 * Determine if the non blocking connect(2) call succeeded, otherwise
	 * we re-try again a connect to the next endpoint.
	 */
	if ((rc = getsockopt(conn->fd, SOL_SOCKET, SO_ERROR, &err, &len)) < 0 || err) {
		conn->errtype      = CONN_ERR_CONNECT;
		conn->errval       = err;
		conn->state        = CONN_STATE_ERROR;
		conn->async_reason = CONN_ASYNC_STATE;
		ev_timer_stop(irc_bot_loop(), &conn->timer_fd);
		ev_async_send(irc_bot_loop(), &conn->async_ev);
	} else {
		ev_timer_again(irc_bot_loop(), &conn->timer_fd);

		if (conn->parent->flags & IRC_SERVER_FLAGS_SSL) {
#if defined(IRCCD_WITH_SSL)
			/*
			 * Connection is successful but still not ready if SSL
			 * is required.
			 *
			 * Create a fake watcher event so that conn__io_cb get's
			 * called again with our normal state machine.
			 */
			conn->state = CONN_STATE_HANDSHAKING;

			conn->ctx = SSL_CTX_new(TLS_method());
			conn->ssl = SSL_new(conn->ctx);

			SSL_set_fd(conn->ssl, conn->fd);
			SSL_set_connect_state(conn->ssl);

			ev_feed_event(irc_bot_loop(), &conn->io_fd, EV_READ | EV_WRITE);
#else
			/* SSL requested on a non-SSL build. */
			abort();
#endif
		} else {
			/*
			 * Connection is ready, notify the server so it can
			 * start its IRC authentication logic.
			 */
			conn->state = CONN_STATE_READY;
			conn->async_reason = CONN_ASYNC_STATE;
			ev_async_send(irc_bot_loop(), &conn->async_ev);
		}
	}
}

/*
 * Check if the SSL connection has completed successfully.
 */
static void
conn__io_handshaking(struct conn *conn, int revents)
{
	(void)conn;
	(void)revents;

#if defined(IRCCD_WITH_SSL)
	switch (conn__tls_handshake(conn)) {
	case 0:
		/*
		 * Handshake still in progress, update the watcher flags if
		 * needed.
		 */
		conn__watch(conn, conn->want(conn));
		break;
	case -1:
		/*
		 * Unrecoverable error, notify server that the SSL handshake has
		 * failed.
		 */
		conn->state        = CONN_STATE_ERROR;
		conn->async_reason = CONN_ASYNC_STATE;
		break;
	default:
		/* Handshake completed, notify server for readiness. */
		conn->state        = CONN_STATE_READY;
		conn->async_reason = CONN_ASYNC_STATE;
		ev_async_send(irc_bot_loop(), &conn->async_ev);
		break;
	}
#else
	return 0;
#endif
}

static void
conn__io_ready(struct conn *conn, int revents)
{
	int rc = 0;

	if (revents & EV_READ)
		rc = conn__recv(conn);
	if (rc == 0 && (revents & EV_WRITE))
		rc = conn__send(conn);

	/*
	 * On error, conn__recv or conn__send updates the conn error status
	 * otherwise check that the incoming buffer has changed to notify the
	 * server of incoming data.
	 */
	if (rc < 0) {
		conn->state        = CONN_STATE_ERROR;
		conn->async_reason = CONN_ASYNC_STATE;
		ev_async_send(irc_bot_loop(), &conn->async_ev);
	} else {
		/* Update condition if buffers are modified */
		conn__watch(conn, conn->want(conn));

		if (conn->insz > 0) {
			conn->async_reason = CONN_ASYNC_MESSAGE;
			ev_async_send(irc_bot_loop(), &conn->async_ev);
		}
	}
}

/*
 * Callback on socket events.
 */
static void
conn__io_cb(struct ev_loop *loop, struct ev_io *self, int revents)
{
	(void)loop;

	static void (*const io[])(struct conn *, int) = {
		[CONN_STATE_CONNECTING]  = conn__io_connecting,
		[CONN_STATE_HANDSHAKING] = conn__io_handshaking,
		[CONN_STATE_READY]       = conn__io_ready
	};

	struct conn *conn = IRC_CONTAINER_OF(self, struct conn, io_fd);

	io[conn->state](conn, revents);
}

/**
 * Close all sockets, timers and SSL stuff from the connection.
 */
static void
conn_close(struct conn *conn)
{
	conn->errtype = CONN_ERR_OK;
	conn->errval = 0;

	if (conn->ai)
		freeaddrinfo(conn->ai);

	conn->ai  = NULL;
	conn->aip = NULL;

	memset(conn->in,  0, sizeof (conn->in));
	memset(conn->out, 0, sizeof (conn->out));
	conn->insz  = 0;
	conn->outsz = 0;

	if (conn->fd != -1) {
		close(conn->fd);
		conn->fd = -1;
	}

	ev_io_stop(irc_bot_loop(), &conn->io_fd);
	ev_timer_stop(irc_bot_loop(), &conn->timer_fd);
	ev_async_stop(irc_bot_loop(), &conn->async_ev);

#if defined(IRCCD_WITH_SSL)
	if (conn->ssl)
		SSL_free(conn->ssl);
	if (conn->ctx)
		SSL_CTX_free(conn->ctx);

	conn->ssl = NULL;
	conn->ctx = NULL;
#endif
}

/*
 * Create the conn->fd socket with the current addrinfo pointer value.
 */
static int
conn__socket(struct conn *conn)
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

/*
 * Change the connection watcher flags.
 */
static void
conn__watch(struct conn *conn, int flags)
{
	assert(ev_is_active(&conn->io_fd));

	if (flags != conn->io_fd.events) {
		ev_io_stop(irc_bot_loop(), &conn->io_fd);
		ev_io_modify(&conn->io_fd, flags);
		ev_io_start(irc_bot_loop(), &conn->io_fd);
	}
}

#if defined(IRCCD_WITH_SSL)

/*
 * Indicate what the SSL connection wants.
 */
static int
conn__tls_want(struct conn *conn)
{
	int reason, flags = 0;

	switch ((reason = SSL_get_error(conn->ssl, 0))) {
	case SSL_ERROR_WANT_READ:
		flags |= EV_READ;
		break;
	case SSL_ERROR_WANT_WRITE:
		flags |= EV_WRITE;
		break;
	default:
		flags = -1;
		break;
	}

	return flags;
}

static ssize_t
conn__tls_recv(struct conn *conn, void *buf, size_t bufsz)
{
	size_t nr = 0;

	/*
	 * Returns 0 or 1 only, re-use conn__tls_want to inspect the
	 * SSL_get_error return value. Caller will automatically call it again
	 * if needed.
	 */
	if (SSL_read_ex(conn->ssl, buf, bufsz, &nr) == 0)
		return (ssize_t)conn__tls_want(conn);

	return (ssize_t)nr;
}

static ssize_t
conn__tls_send(struct conn *conn, const void *buf, size_t bufsz)
{
	size_t ns = 0;

	/* See conn__tls_recv also. */
	if (SSL_write_ex(conn->ssl, buf, bufsz, &ns) == 0 && conn__tls_want(conn) < 0)
		ns = -1;

	return (ssize_t)ns;
}

/*
 * Function called to continue a SSL handshake. Usually called from the socket
 * event handler multiple times until it completes successfully.
 */
static int
conn__tls_handshake(struct conn *conn)
{
	int rc;

	if ((rc = SSL_do_handshake(conn->ssl)) < 0 && conn__tls_want(conn) < 0)
		rc = -1;

	return rc;
}

#endif

/* }}} */

/* {{{ IRC server handlers */

static void
handle_connect(struct irc_server *s, struct msg *msg)
{
	(void)msg;

	struct irc_channel *ch;
	struct irc_event ev = {};

	/* Now join all channels that were requested. */
	LL_FOREACH(s->channels, ch)
		irc_server_join(s, ch->name, ch->password);

	s->state = IRC_SERVER_STATE_RUNNING;

	ev.type = IRC_EVENT_CONNECT;
	ev.server = s;

	irc_log_info("server %s: connection complete", s->name);
	irc_bot_dispatch(&ev);
}

static void
handle_disconnect(struct irc_server *s)
{
	struct irc_event ev = {};

	ev.type = IRC_EVENT_DISCONNECT;
	ev.server = s;

	irc_log_info("server %s: connection lost", s->name);
	irc_bot_dispatch(&ev);
}

static void
read_support_prefix(struct irc_server *s, const char *value)
{
	(void)s;
	(void)value;

	printf("prefix: %s\n", value);
#if 0
	char modes[IRC_UTIL_SIZE(s->prefixes) + 1] = {0};
	char tokens[IRC_UTIL_SIZE(s->prefixes) + 1] = {0};
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
#endif
}

static void
read_support_chantypes(struct irc_server *s, const char *value)
{
	s->chantypes = irc_util_strdupfree(s->chantypes, value);
}

static void
handle_support(struct irc_server *s, struct msg *msg)
{
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
			s->channel_max = atoi(value);
			irc_log_info("server %s: channel name limit: %u",
			    s->name, s->channel_max);
		} else if (strcmp(key, "NICKLEN") == 0) {
			s->nickname_max = atoi(value);
			irc_log_info("server %s: nickname limit:     %u",
			    s->name, s->nickname_max);
		} else if (strcmp(key, "TOPICLEN") == 0) {
			s->topic_max = atoi(value);
			irc_log_info("server %s: topic limit:        %u",
			    s->name, s->topic_max);
		} else if (strcmp(key, "AWAYLEN") == 0) {
			s->away_max = atoi(value);
			irc_log_info("server %s: away message limit: %u",
			    s->name, s->away_max);
		} else if (strcmp(key, "KICKLEN") == 0) {
			s->kick_max = atoi(value);
			irc_log_info("server %s: kick reason limit:  %u",
			    s->name, s->kick_max);
		}
		else if (strcmp(key, "CHARSET") == 0) {
			s->charset = irc_util_strdupfree(s->charset, value);
			irc_log_info("server %s: charset:            %s",
			    s->name, s->charset);
		} else if (strcmp(key, "CASEMAPPING") == 0) {
			s->casemapping = irc_util_strdupfree(s->casemapping, value);
			irc_log_info("server %s: case mapping:       %s",
			    s->name, s->casemapping);
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
handle_join(struct irc_server *s, struct msg *msg)
{
	struct irc_event ev = {};

	ev.type = IRC_EVENT_JOIN;
	ev.server = s;
	ev.join.origin = irc_util_strdup(msg->prefix);
	ev.join.channel = irc_util_strdup(msg->args[0]);

	channels_add(s, ev.join.channel, NULL, IRC_CHANNEL_FLAGS_JOINED);

	if (server_is_self_nickname(s, ev.join.origin))
		irc_log_info("server %s: joined channel %s", s->name, ev.join.channel);

	irc_bot_dispatch(&ev);
}

static void
handle_kick(struct irc_server *s, struct msg *msg)
{
	struct irc_channel *ch;
	struct irc_event ev = {};

	ev.type = IRC_EVENT_KICK;
	ev.server = s;
	ev.kick.origin = irc_util_strdup(msg->prefix);
	ev.kick.channel = irc_util_strdup(msg->args[0]);
	ev.kick.target = irc_util_strdup(msg->args[1]);
	ev.kick.reason = msg->args[2] ? irc_util_strdup(msg->args[2]) : NULL;

	ch = channels_add(s, ev.kick.channel, NULL, 1);

	/*
	 * If the bot was kicked itself mark the channel as not joined and
	 * rejoin it automatically if the option is set.
	 */
	if (server_is_self_nickname(s, ev.kick.target)) {
		ch->flags &= IRC_CHANNEL_FLAGS_JOINED;
		irc_channel_clear(ch);

		if (s->flags & IRC_SERVER_FLAGS_AUTO_REJOIN) {
			irc_server_join(s, ch->name, ch->password);
			irc_log_info("server %s: auto-rejoining %s after kick",
			    s->name, ch->name);
		}
	} else
		irc_channel_remove(ch, ev.kick.target);

	irc_bot_dispatch(&ev);
}

static void
handle_mode(struct irc_server *s, struct msg *msg)
{
	(void)s;
	(void)msg;
#if 0
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

	if (!(ch = irc_server_find(s, ev.mode.channel)))
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
		if ((mode = server_find_mode(s, *p)) == 0) {
			++argindex;
			continue;
		}
		if (!msg->args[argindex] || !(u = irc_channel_get(ch, msg->args[argindex]))) {
			++argindex;
			continue;
		}

		++argindex;

		if (action == '+')
			irc_channel_set(ch, const char *nickname, int modes)
			u->modes |= (1 << mode);
		else
			u->modes &= ~(1 << mode);
	}

skip:
	irc_bot_dispatch(&ev);
#endif
}

static void
handle_part(struct irc_server *s, struct msg *msg)
{
	struct irc_channel *ch;
	struct irc_event ev = {};

	ev.type = IRC_EVENT_PART;
	ev.server = s;
	ev.part.origin = irc_util_strdup(msg->prefix);
	ev.part.channel = irc_util_strdup(msg->args[0]);
	ev.part.reason = msg->args[1] ? irc_util_strdup(msg->args[1]) : NULL;

	ch = irc_server_find(s, ev.part.channel);

	if (server_is_self_nickname(s, ev.part.origin)) {
		channels_remove(s, ch);
		irc_log_info("server %s: leaving channel %s", s->name, ev.part.channel);
	} else
		irc_channel_remove(ch, ev.part.origin);

	irc_bot_dispatch(&ev);
}

static void
handle_msg(struct irc_server *s, struct msg *msg)
{
	struct irc_server_user user = {};
	struct irc_event ev = {};

	ev.server = s;

	/*
	 * Detect CTCP commands which are PRIVMSG with a special boundaries.
	 *
	 * Example:
	 * PRIVMSG jean :\001ACTION I'm eating\001
	 * PRIVMSG jean :\001VERSION\001
	 */
	if (msg_is_ctcp(msg->args[1])) {
		irc_server_split(msg->prefix, &user);

		if (strcmp(msg->args[1], "\x01""CLIENTINFO\x01") == 0)
			irc_server_notice(s, user.nickname,
			    "\x01""CLIENTINFO ACTION CLIENTINFO SOURCE TIME VERSION\x01");
		else if (strcmp(msg->args[1], "\x01""SOURCE\x01") == 0)
			irc_server_send(s, "NOTICE %s :\x01SOURCE %s\x01",
			    user.nickname, s->ctcp_source
			    ? s->ctcp_source
			    : "http://hg.malikania.fr/irccd");
		else if (strcmp(msg->args[1], "\x01""TIME\x01") == 0) {
			time_t now = time(NULL);

			irc_server_send(s, "NOTICE %s :\x01TIME %s\x01",
			    user.nickname, ctime(&now));
		} else if (strcmp(msg->args[1], "\x01VERSION\x01") == 0) {
			if (strlen(s->ctcp_version) != 0)
				irc_server_send(s, "NOTICE %s :\x01VERSION %s\x01",
				    user.nickname, s->ctcp_version);
		} else if (strncmp(msg->args[1], "\x01""ACTION", 7) == 0) {
			ev.type = IRC_EVENT_ME;
			ev.message.origin = irc_util_strdup(msg->prefix);
			ev.message.channel = irc_util_strdup(msg->args[0]);
			ev.message.message = irc_util_strdup(msg_ctcp(msg->args[1]));
		}
	} else {
		ev.type = IRC_EVENT_MESSAGE;
		ev.message.origin = irc_util_strdup(msg->prefix);
		ev.message.channel = irc_util_strdup(msg->args[0]);
		ev.message.message = irc_util_strdup(msg->args[1]);
	}

	irc_bot_dispatch(&ev);
}

static void
handle_nick(struct irc_server *s, struct msg *msg)
{
	struct irc_event ev = {};

	ev.type = IRC_EVENT_NICK;
	ev.server = s;
	ev.nick.origin = irc_util_strdup(msg->prefix);
	ev.nick.nickname = irc_util_strdup(msg->args[0]);

	/* Update nickname if it is myself. */
	if (server_is_self_nickname(s, ev.nick.origin)) {
		irc_log_info("server %s: nick change %s -> %s", s->name,
		    s->nickname, ev.nick.nickname);
		s->nickname = irc_util_strdupfree(s->nickname, ev.nick.nickname);
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
	(void)msg;

	if (msg->args[0])
		irc_server_send(s, "PONG :%s", msg->args[0]);
}

static void
handle_names(struct irc_server *s, struct msg *msg)
{
	struct irc_channel *ch;
	char *p, *token;
	int modes = 0;

	ch = channels_add(s, msg->args[2], NULL, 1);

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
	FILE *fp;
	size_t length;
	const struct irc_channel *ch;
	const struct irc_channel_user *u;
	struct irc_event ev = {};

	ev.type = IRC_EVENT_NAMES;
	ev.server = s;
	ev.names.channel = irc_util_strdup(msg->args[1]);

	/* Construct a string list for every user in the channel. */
	ch = irc_server_find(s, ev.names.channel);

	if (!(fp = open_memstream(&ev.names.names, &length)))
		return;

	LL_FOREACH(ch->users, u) {
		for (size_t i = 0; i < s->prefixesz; ++i)
			if (u->modes & (1 << i))
				fprintf(fp, "%c", s->prefixes[i].symbol);

		fprintf(fp, "%s", u->nickname);

		if (u->next)
			fputc(' ', fp);
	}

	fclose(fp);

	irc_bot_dispatch(&ev);
}

static void
handle_nicknameinuse(struct irc_server *s, struct msg *msg)
{
	(void)msg;

	irc_log_warn("server %s: nickname %s is already in use", s->name, s->nickname);
	server_disconnect(s);
}

static void
handle_error(struct irc_server *s, struct msg *msg)
{
	struct irc_event ev = {};

	if (msg->args[0])
		irc_log_warn("server %s: %s", s->name, msg->args[0]);

	ev.server = s;
	ev.type = IRC_EVENT_DISCONNECT;

	server_disconnect(s);
	irc_bot_dispatch(&ev);
}

static void
handle_whoisuser(struct irc_server *s, struct msg *msg)
{
	(void)s;

	s->bufwhois.nickname = irc_util_strdup(msg->args[1]);
	s->bufwhois.username = irc_util_strdup(msg->args[2]);
	s->bufwhois.hostname = irc_util_strdup(msg->args[3]);
	s->bufwhois.realname = irc_util_strdup(msg->args[5]);
}

static void
handle_whoischannels(struct irc_server *s, struct msg *msg)
{
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
handle_endofwhois(struct irc_server *s, struct msg *msg)
{
	(void)msg;

	struct irc_event ev = {};

	ev.server = s;
	ev.type = IRC_EVENT_WHOIS;
	ev.whois = s->bufwhois;

	irc_bot_dispatch(&ev);

	/* Get rid of buffered whois. */
	memset(&s->bufwhois, 0, sizeof (s->bufwhois));
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

static inline struct handler *
handler_find(const char *name)
{
	return bsearch(name, handlers, IRC_UTIL_SIZE(handlers),
	    sizeof (struct handler), handler_cmp);
}

static void
handler(struct irc_server *s, struct msg *msg)
{
	const struct handler *h;

	if ((h = handler_find(msg->cmd)))
		h->handle(s, msg);
}

/* }}} */

/* {{{ server private routines */

static inline int
server_find_mode(struct irc_server *s, int mode)
{
	for (size_t i = 0; i < s->prefixesz; ++i)
		if (s->prefixes[i].mode == mode)
			return i;

	return 0;
}

static inline int
server_is_self_nickname(const struct irc_server *s, const char *nick)
{
	return strncasecmp(s->nickname, nick, strlen(s->nickname)) == 0;
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

/*
 * Disconnect the server, cleanup its resource and reconnect later.
 *
 * This function is called at any time but it does not generate a plugin event
 * unless the current state was running meaning that we were successfully
 * connected to a IRC server before getting closed.
 */
static void
server_disconnect(struct irc_server *s)
{
	conn_close(s->conn);
	channels_clear(s);
	server_clear(s);

	if (s->state == IRC_SERVER_STATE_RUNNING)
		handle_disconnect(s);

	s->state = IRC_SERVER_STATE_DISCONNECTED;

	if (s->flags & IRC_SERVER_FLAGS_AUTO_RECONNECT) {
		irc_log_info("server %s: retrying in %d seconds",
		    s->name, (int)RECONNECT_DELAY);

		ev_timer_stop(irc_bot_loop(), &conn->timer_fd);
		ev_timer_set(&conn->timer_fd, RECONNECT_DELAY, 0.0);
		ev_timer_start(irc_bot_loop(), &conn->timer_fd);
	}
}

static void server_free(struct irc_server *s)
{
	conn_free(s);
	channels_free(s);
	free(s);
}

static void
conn_connect(struct conn *conn)
{
	/*
	 * If this pointer is NULL, it means we have not ever tried to connect
	 * to any server yet. Start with an initial DNS lookup.
	 */
	if (!conn->ai_list)
		conn__resolve(conn);






#if 0
	for (; conn->aip; conn->aip = conn->aip->ai_next) {
		/* Attempt to initialize a socket with the current conn->aip. */
		if (conn_socket(s) < 0)
			continue;

		/*
		 * With some luck, the connection completes immediately,
		 * otherwise we will need to wait until the socket is writable.
		 */
		if (connect(conn->fd, conn->aip->ai_addr, conn->aip->ai_addrlen) == 0) {
			server_switch(s, IRC_SERVER_STATE_HANDSHAKING);
			return;
		}

		if (errno == EINPROGRESS || errno == EAGAIN) {
			/* Connect "succeeds" but isn't complete yet. */
			server_switch(s, IRC_SERVER_STATE_CONNECTING);
			return;
		}

		irc_log_warn("server %s: connect: %s", s->name, strerror(errno));
	}

	/* No more address available. */
	if (conn->aip == NULL) {
		// TODO: dispatch error
	}
#endif
}

/*
 * Start protocol exchange with the IRC server.
 */
static void
server_identify(struct irc_server *s)
{
	/*
	 * Use multi-prefix extension to keep track of all combined "modes" in
	 * a channel.
	 *
	 * https://ircv3.net/specs/extensions/multi-prefix-3.1.html
	 */
	irc_server_send(s, "CAP REQ :multi-prefix");

	if (s->password)
		irc_server_send(s, "PASS %s", s->password);

	irc_server_send(s, "NICK %s", s->nickname);
	irc_server_send(s, "USER %s %s %s :%s", s->username, s->username, s->username, s->realname);
	irc_server_send(s, "CAP END");
}

static void
server_dequeue(struct irc_server *s)
{
	struct msg msg;

	while (conn_dequeue(s->conn, &msg))
		handler(s, &msg);
}

static void
server_async_cb(struct ev_loop *loop, struct ev_async *self, int revents)
{
	(void)loop;
	(void)self;
	(void)revents;

	struct conn *conn = IRC_UTIL_CONTAINER_OF(self, struct conn, async_ev);

	switch (conn->async_reason) {
	case CONN_ASYNC_ERROR:
		irc_log_warn("server %s: connection lost", conn->parent->name);
		handle_disconnect(conn->parent);
		break;
	case CONN_ASYNC_READY:
		irc_log_info("server %s: connection ready, identifying", conn->parent->name);
		server_identify(conn->parent);
		break;
	case CONN_ASYNC_INCOMING:
		irc_log_debug("server %s: fetching incoming events", conn->parent->name);
		server_dequeue(conn->parent);
		break;
	default:
		break;
	}

	/* Reset reason for debugging purposes. */
	conn->async_reason = CONN_ASYNC_NONE;
}

/* }}} */

struct irc_server *
irc_server_new(const char *name)
{
	assert(name);

	struct irc_server *s;
	struct conn *conn;

	s = irc_util_calloc(1, sizeof (*s));

	/* Private connection. */
	s->conn = conn = conn_new(s);
	ev_async_init(&conn->async_ev, server_async_cb);
	ev_async_start(irc_bot_loop(), &conn->async_ev);

	/* Predefined sane defaults. */
	s->name         = irc_util_strdup(name);
	s->port         = IRC_SERVER_DEFAULT_PORT;
	s->prefix       = irc_util_strdup(IRC_SERVER_DEFAULT_PREFIX);
	s->ctcp_version = irc_util_strdup(IRC_SERVER_DEFAULT_CTCP_VERSION);

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

	if (version)
		s->ctcp_version = irc_util_strdupfree(s->ctcp_version, version);
	if (source)
		s->ctcp_source = irc_util_strdupfree(s->ctcp_source, source);
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

	/* Start with no state as getaddrinfo could even fail. */
	s->state = IRC_SERVER_STATE_DISCONNECTED;

#if !defined(IRCCD_WITH_SSL)
	if (conn->flags & IRC_CONN_SSL) {
		irc_log_warn("server %s: SSL requested but not available", conn->sv->name);
		errno = EINVAL;
		return -1;
	}
#endif

	conn_connect(s->conn);
}

void
irc_server_disconnect(struct irc_server *s)
{
	assert(s);

	s->state = IRC_SERVER_STATE_DISCONNECTED;

	conn_clear(s);
	channels_clear(s);
	server_clear(s);
}

void
irc_server_reconnect(struct irc_server *s)
{
	assert(s);

	irc_server_disconnect(s);
	irc_server_connect(s);
}

struct irc_channel *
irc_server_find(struct irc_server *s, const char *name)
{
	assert(s);
	assert(name);

	struct irc_channel *ch;

	LL_FOREACH(s->channels, ch)
		if (strcasecmp(ch->name, name) == 0)
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

	if (s->state != IRC_SERVER_STATE_IDENTIFYING && s->state != IRC_SERVER_STATE_RUNNING) {
		errno = ENOTCONN;
		return -1;
	}

	va_start(ap, fmt);
	vsnprintf(buf, sizeof (buf), fmt, ap);
	va_end(ap);

	return conn_push(s, buf, strlen(buf));
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
		ch = channels_add(s, name, pass, IRC_CHANNEL_FLAGS_NONE);

	if (!(ch->flags & IRC_CHANNEL_FLAGS_JOINED) && s->state == IRC_SERVER_STATE_RUNNING) {
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
irc_server_nick(struct irc_server *s, const char *nick)
{
	assert(s);
	assert(nick);

	if (s->state <= IRC_SERVER_STATE_DISCONNECTED) {
		irc_util_strlcpy(s->nickname, nick, sizeof (s->nickname));
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

	if (--s->refc == 0)
		server_free(s);
}
