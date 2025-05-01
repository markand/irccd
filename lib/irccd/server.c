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
#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
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
#define CONNECT_TIMEOUT 5.0     /* Seconds before marking a server as dead. */
#define PING_TIMEOUT    300.0   /* Seconds after assuming ping timeout. */

/*
 * Current connection state.
 */
enum conn_state {
	/*
	 * Connection state closed, conn->fd = -1.
	 */
	CONN_STATE_CLOSE,

	/*
	 * Attempt to call getaddrinfo() on the server hostname.
	 */
	CONN_STATE_RESOLVE,

	/*
	 * Connection is in progress.
	 */
	CONN_STATE_CONNECT,

	/*
	 * (SSL only)
	 *
	 * SSL handshaking is in progress.
	 */
	CONN_STATE_HANDSHAKE,

	/*
	 * Connection is ready for I/O.
	 */
	CONN_STATE_READY,

	/*
	 * Retry next connection or later.
	 */
	CONN_STATE_RETRY
};

/*
 * Reason why conn notified the server through conn->notify_async.
 */
enum conn_notify {
	/* Unused default value. */
	CONN_NOTIFY_NONE,

	/*
	 * Connection became ready, the server will perform the initial IRC
	 * protocol identification and authentication.
	 */
	CONN_NOTIFY_READY,

	/*
	 * Some data arrived in the incoming buffer and conn->insz is greater
	 * than zero.
	 *
	 * Server will dequeue the buffer with conn_dequeue.
	 */
	CONN_NOTIFY_INCOMING,

	/*
	 * Error happened in the connection.
	 */
	CONN_NOTIFY_DISCONNECT,
};

/*
 * Private abstraction to the server connection using either plain or SSL
 * transport.
 */
struct conn {
	/* Link to the parent server. */
	struct irc_server *parent;

	/* Current connection state. */
	enum conn_state state;

	/* Next connection state. */
	enum conn_state state_next;

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

	/* Socket and its watcher. */
	int fd;
	struct ev_io io_fd;

	/* Connection to server notification */
	enum conn_notify notify;
	struct ev_async  notify_async;

	/* OpenSSL boring stuff. */
#if defined(IRCCD_WITH_SSL)
	SSL_CTX *ctx;
	SSL *ssl;
#endif

	/* Multi purpose timer */
	struct ev_timer timer;

	/* Asynchronous state machine event */
	struct ev_async async_switch;

	/* Transport callbacks */
	ssize_t (*recv)(struct conn *, void *, size_t);
	ssize_t (*send)(struct conn *, const void *, size_t);
	int     (*want)(struct conn *);
};

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

/*
 * Tell if the nickname targets the bot itself.
 */
static inline int
is_self(const struct irc_server *s, const char *nickname)
{
	return strncasecmp(s->nickname, nickname, strlen(s->nickname)) == 0;
}

/*
 * Parse the IRC prefixes protocol in the form `(abcde)!@#$%`.
 */
static void
modes_parse(struct irc_server *s, const char *value)
{
	(void)s;
	(void)value;

	char modes[16] = {}, syms[16] = {};
	size_t modesz, symsz;

	/* Just make sure in case this line would come twice. */
	s->prefixes  = irc_util_free(s->prefixes);
	s->prefixesz = 0;

	if (sscanf(value, "(%15[^)])%15s", modes, syms) == 2) {
		modesz = strlen(modes);
		symsz  = strlen(syms);

		if (modesz != symsz) {
			irc_log_warn("server %s: broken support prefix string", s->name);
			return;
		}

		s->prefixes  = irc_util_calloc(modesz, sizeof (*s->prefixes));
		s->prefixesz = modesz;

		for (size_t i = 0; i < modesz; ++i) {
			s->prefixes[i].mode   = modes[i];
			s->prefixes[i].symbol = syms[i];
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

/*
 * Return a human friendly description of an remote address.
 */
static const char *
conn__info(const struct conn *conn, const struct addrinfo *ai)
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
 * Change the connection state asynchronously to avoid infinite recursion when
 * one of the function fails.
 */
static void
conn__switch(struct conn *conn, enum conn_state new_state)
{
	assert(new_state != CONN_STATE_CLOSE);

	conn->state_next = new_state;
	ev_async_send(irc_bot_loop(), &conn->async_switch);
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

/*
 * Reset most of the connection prior to reconnect.
 *
 * Clears the following:
 *
 * - socket
 * - buffers
 * - SSL
 * - timer
 */
static void
conn__close(struct conn *conn)
{
	memset(conn->in,  0, sizeof (conn->in));
	memset(conn->out, 0, sizeof (conn->out));
	conn->insz  = 0;
	conn->outsz = 0;

	if (conn->fd != -1) {
		close(conn->fd);
		conn->fd = -1;
	}

	ev_io_stop(irc_bot_loop(), &conn->io_fd);
	ev_timer_stop(irc_bot_loop(), &conn->timer);

#if defined(IRCCD_WITH_SSL)
	if (conn->ssl)
		SSL_free(conn->ssl);
	if (conn->ctx)
		SSL_CTX_free(conn->ctx);

	conn->ssl = NULL;
	conn->ctx = NULL;
#endif
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
		else
			irc_log_warn("server %s: recv: %s",
			    conn->parent->name, strerror(errno));
	} else if (nr == 0) {
		irc_log_warn("server %s: remote closed connection", conn->parent->name);
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
		else
			irc_log_warn("server %s: send: %s",
			    conn->parent->name, strerror(errno));
	}

	return ns;
}

static int
conn__plain_want(struct conn *conn)
{
	int flags = EV_READ;

	if (conn->outsz)
		flags |= EV_WRITE;

	return flags;
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
 * Function called to initiate/continue a SSL handshake. Usually called from the
 * socket event handler multiple times until it completes successfully.
 *
 * May switch to:
 *
 * - CONN_STATE_CONNECT
 * - CONN_STATE_READY
 */
static void
conn__tls_handshake(struct conn *conn)
{
	int flags;

	switch (SSL_do_handshake(conn->ssl)) {
	case 0:
		/*
		 * Handshake still in progress, update the watcher flags if
		 * needed unless SSL_get_error returns something else (which
		 * usually should not).
		 */
		if ((flags = conn->want(conn)) < 0)
			conn__switch(conn, CONN_STATE_RETRY);
		else
			conn__watch(conn, flags);
		break;
	case -1:
		irc_log_warn("server %s: SSL handshake failed", conn->parent->name);
		conn__switch(conn, CONN_STATE_RETRY);
		break;
	default:
		conn__switch(conn, CONN_STATE_READY);
		break;
	}
}

#endif

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

/*
 * Create the conn->fd socket with the current addrinfo pointer value.
 */
static int
conn__socket(struct conn *conn)
{
	const struct addrinfo *ai = conn->ai;
	int flags = 0;

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
 * Attempt to resolve the server IRC hostname into a broken down list of
 * addrinfo in the conn->ai_list field.
 *
 * May switch to:
 *
 * - CONN_STATE_CONNECT
 * - CONN_STATE_RETRY
 */
static void
conn__resolve(struct conn *conn)
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

	/*
	 * If this function fail there is nothing we can do except going
	 * directly to the reconnect later step.
	 */
	if ((rc = getaddrinfo(conn->parent->hostname, service, &hints, &conn->ai_list)) != 0) {
		irc_log_warn("server %s: getaddrinfo: %s", conn->parent->name, gai_strerror(rc));
		rc = -1;
		conn__switch(conn, CONN_STATE_RETRY);
	} else {
		rc = 0;

		for (const struct addrinfo *ai = conn->ai_list; ai; ai = ai->ai_next)
			irc_log_debug("server %s: resolves to %s",
			    conn->parent->name, conn__info(conn, ai));

		/* Start with first one. */
		conn->ai = conn->ai_list;
		conn__switch(conn, CONN_STATE_CONNECT);
	}
}

/*
 * Attempt to connect to the current conn->ai remote address.
 *
 * May switch to:
 *
 * - CONN_STATE_HANDSHAKE
 * - CONN_STATE_RETRY
 */
static void
conn__connect(struct conn *conn)
{
	if (conn__socket(conn) < 0) {
		conn__switch(conn, CONN_STATE_RETRY);
		return;
	}

	/* Possible immediate success (unlikely). */
	if (connect(conn->fd, conn->ai->ai_addr, conn->ai->ai_addrlen) == 0) {
		irc_log_debug("server %s: connect successful", conn->parent->name);
		ev_io_set(&conn->io_fd, conn->fd, EV_READ | EV_WRITE);
		ev_io_start(irc_bot_loop(), &conn->io_fd);
		conn__switch(conn, CONN_STATE_HANDSHAKE);
		return;
	}

	/*
	 * Connection is in progress, we will need to wait that
	 * conn__io_cb complete it by itself.
	 */
	if (errno == EINPROGRESS || errno == EAGAIN) {
		irc_log_debug("server %s: connect in progress", conn->parent->name);
		ev_timer_stop(irc_bot_loop(), &conn->timer);
		ev_timer_set(&conn->timer, 0.0, CONNECT_TIMEOUT);
		ev_timer_start(irc_bot_loop(), &conn->timer);
		ev_io_set(&conn->io_fd, conn->fd, EV_WRITE);
		ev_io_start(irc_bot_loop(), &conn->io_fd);
	} else {
		irc_log_warn("server %s: connect: %s", conn->parent->name, strerror(errno));
		conn__switch(conn, CONN_STATE_HANDSHAKE);
	}
}

/*
 * Function called when connection requires DNS resolving of addresses.
 *
 * This function should only happen on closed socket.
 */
static void
conn__async_resolve(struct conn *conn)
{
	conn->state = CONN_STATE_RESOLVE;

	irc_log_info("server %s: resolving hostname '%s'",
		conn->parent->name, conn->parent->hostname);

	/* First of all, get rid of previous content if any. */
	if (conn->ai_list)
		freeaddrinfo(conn->ai_list);

	conn->ai = conn->ai_list = NULL;

	/* No timer, required here. */
	ev_timer_stop(irc_bot_loop(), &conn->timer);

	/*
	 * Start DNS resolving for the IRC server hostname, failing at
	 * this point is quite uncommon and will defer a retry later.
	 */
	conn__resolve(conn);
}

/*
 * Attempt to connect to the current endpoint.
 */
static void
conn__async_connect(struct conn *conn)
{
	conn->state = CONN_STATE_CONNECT;

	irc_log_info("server %s: trying remote '%s'",
	    conn->parent->name, conn__info(conn, conn->ai));
	conn__connect(conn);
}

/*
 * Perform an SSL handshake if the server is configured that way, otherwise
 * change to ready state.
 */
static void
conn__async_handshake(struct conn *conn)
{
	if (conn->parent->flags & IRC_SERVER_FLAGS_SSL) {
#if defined(IRCCD_WITH_SSL)
		conn->state = CONN_STATE_HANDSHAKE;
		conn->ctx = SSL_CTX_new(TLS_method());
		conn->ssl = SSL_new(conn->ctx);

		SSL_set_fd(conn->ssl, conn->fd);
		SSL_set_connect_state(conn->ssl);

		ev_timer_again(irc_bot_loop(), &conn->timer);

		/*
		 * Perform an initial SSL handshake, this may take a while so io
		 * callback will do the rest.
		 */
		conn__tls_handshake(conn);
#else
		abort();
#endif
	} else {
		/*
		 * Socket event may already arrive so make sure conn__io_cb
		 * is not called with CONN_STATE_HANDSHAKE in the meantime.
		 */
		conn->state = CONN_STATE_READY;
		conn__switch(conn, CONN_STATE_READY);
	}
}

/*
 * Enable read watcher and notify the server that event has happened.
 */
static void
conn__async_ready(struct conn *conn)
{
	conn->state = CONN_STATE_READY;

	/* Start with EV_READ only for now. */
	conn__watch(conn, EV_READ);

	/* Modify timer to watch for ping timeout. */
	ev_timer_stop(irc_bot_loop(), &conn->timer);
	ev_timer_set(&conn->timer, PING_TIMEOUT, PING_TIMEOUT);
	ev_timer_start(irc_bot_loop(), &conn->timer);

	/*
	 * Notify parent of success, it will start the IRC protocol exchange by
	 * itself.
	 */
	conn->notify = CONN_NOTIFY_READY;
	ev_async_send(irc_bot_loop(), &conn->notify_async);
}

static void
conn__async_retry(struct conn *conn)
{
	/* Cleanup socket and stuff, at this point it's not recoverable. */
	conn__close(conn);

	/*
	 * Depending on the current state we may not do the same thing:
	 *
	 * If we are trying to connect to a remote that didn't succeed, try
	 * immediately the next remote address in the list. Otherwise, wait
	 * a little and restart from scratch.
	 */
	switch (conn->state) {
	case CONN_STATE_RESOLVE:
	case CONN_STATE_READY:
		/*
		 * We were either unable to get DNS remote information or
		 * successfully connected to a server. Wait a little because it
		 * make no sense to retry immediately.
		 */
		irc_log_warn("server %s: retrying in few seconds...", conn->parent->name);
		ev_timer_set(&conn->timer, RECONNECT_DELAY, 0.0);
		ev_timer_start(irc_bot_loop(), &conn->timer);
		break;
	case CONN_STATE_CONNECT:
	case CONN_STATE_HANDSHAKE:
		/*
		 * Connnect or handshake failed, try the next remote
		 * immediately if available.
		 */
		if (conn->ai->ai_next) {
			conn->ai = conn->ai->ai_next;
			irc_log_warn("server %s: trying next remote '%s'",
			    conn->parent->name, conn__info(conn, conn->ai));
			conn__switch(conn, CONN_STATE_CONNECT);
		} else {
			/*
			 * We have iterated through the entire list of remote
			 * addresses, wait a little and resolve again.
			 */
			irc_log_warn("server %s: no more addresses available",
			    conn->parent->name);
			ev_timer_set(&conn->timer, RECONNECT_DELAY, 0.0);
			ev_timer_start(irc_bot_loop(), &conn->timer);
		}
		break;
	default:
		break;
	}

	conn->state = CONN_STATE_RETRY;
}

static void
conn__async_switch_cb(struct ev_loop *loop, struct ev_async *self, int revents)
{
	(void)loop;
	(void)revents;

	struct conn *conn;

	conn = IRC_UTIL_CONTAINER_OF(self, struct conn, async_switch);

	static void (* const cb[])(struct conn *) = {
		[CONN_STATE_RESOLVE]     = conn__async_resolve,
		[CONN_STATE_CONNECT]     = conn__async_connect,
		[CONN_STATE_HANDSHAKE]   = conn__async_handshake,
		[CONN_STATE_READY]       = conn__async_ready,
		[CONN_STATE_RETRY]       = conn__async_retry
	};

	assert(conn->state_next != CONN_STATE_CLOSE);

	cb[conn->state_next](conn);
}

/*
 * Callback on connection timer expiry.
 */
static void
conn__timer_cb(struct ev_loop *loop, struct ev_timer *self, int revents)
{
	(void)loop;
	(void)revents;

	struct conn *conn = IRC_UTIL_CONTAINER_OF(self, struct conn, timer);

	switch (conn->state) {
	case CONN_STATE_CONNECT:
		irc_log_warn("server %s: timeout while connecting", conn->parent->name);
		break;
	case CONN_STATE_HANDSHAKE:
		irc_log_warn("server %s: timeout during SSL handshake", conn->parent->name);
		break;
	case CONN_STATE_READY:
		irc_log_warn("server %s: ping timeout", conn->parent->name);

		/*
		 * Notify parent on error because it's only meaningful to
		 * dispatch this event if the connection to the IRC server was
		 * already complete.
		 */
		conn->notify = CONN_NOTIFY_DISCONNECT;
		ev_async_send(loop, &conn->notify_async);
		break;
	case CONN_STATE_RETRY:
		irc_log_info("server %s: auto-reconnecting now", conn->parent->name);
		conn__switch(conn, CONN_STATE_RESOLVE);
		break;
	default:
		break;
	}
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

	int rc = 0, err = ECONNREFUSED;
	socklen_t len = sizeof (int);

	/*
	 * Determine if the non blocking connect(2) call succeeded, otherwise
	 * we re-try again a connect to the next endpoint.
	 */
	if ((rc = getsockopt(conn->fd, SOL_SOCKET, SO_ERROR, &err, &len)) < 0 || err) {
		irc_log_warn("server %s: connect: %s", conn->parent->name, strerror(err));
		conn__switch(conn, CONN_STATE_RETRY);
	} else {
		ev_timer_again(irc_bot_loop(), &conn->timer);
		conn__switch(conn, CONN_STATE_HANDSHAKE);
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
	conn__tls_handshake(conn);
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

	if (rc < 0) {
		/* Oops, connection lost. Retry. */
		conn__switch(conn, CONN_STATE_RETRY);

		/* Notify parent of disconnection. */
		conn->notify = CONN_NOTIFY_DISCONNECT;
		ev_async_send(irc_bot_loop(), &conn->notify_async);
	} else {
		/* Update condition if buffers are modified */
		conn__watch(conn, conn->want(conn));

		if (conn->insz > 0) {
			conn->notify = CONN_NOTIFY_INCOMING;
			ev_async_send(irc_bot_loop(), &conn->notify_async);
		}

		/* Feed ping timer. */
		ev_timer_again(irc_bot_loop(), &conn->timer);
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
		[CONN_STATE_CONNECT]   = conn__io_connecting,
		[CONN_STATE_HANDSHAKE] = conn__io_handshaking,
		[CONN_STATE_READY]     = conn__io_ready
	};

	struct conn *conn = IRC_CONTAINER_OF(self, struct conn, io_fd);

	io[conn->state](conn, revents);
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
 * Destroy entirely the connection meaning the pointer must not be used
 * anymore.
 */
static void
conn_free(struct conn *conn)
{
	conn__close(conn);

	if (conn->ai_list)
		freeaddrinfo(conn->ai_list);

	ev_async_stop(irc_bot_loop(), &conn->notify_async);
	ev_async_stop(irc_bot_loop(), &conn->async_switch);

	free(conn);
}

static struct conn *
conn_new(struct irc_server *parent, void (*notify_cb)(struct ev_loop *, struct ev_async *, int))
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
	} else {
		conn->recv = conn__plain_recv;
		conn->send = conn__plain_send;
		conn->want = conn__plain_want;
	}

	ev_init(&conn->io_fd, conn__io_cb);
	ev_init(&conn->timer, conn__timer_cb);

	ev_async_init(&conn->async_switch, conn__async_switch_cb);
	ev_async_start(irc_bot_loop(), &conn->async_switch);

	/* Connection to parent notification. */
	ev_async_init(&conn->notify_async, notify_cb);
	ev_async_start(irc_bot_loop(), &conn->notify_async);

	conn__switch(conn, CONN_STATE_RESOLVE);

	return conn;
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

static struct irc_channel *
channels_add(struct irc_server *s,
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
 * Remove the channel from the server.
 *
 * Explicitly allow ch to be NULL for convenience.
 */
static inline void
channels_remove(struct irc_server *s, struct irc_channel *ch)
{
	if (ch) {
		LL_DELETE(s->channels, ch);
		irc_channel_free(ch);
	}
}

static void
handle_connect(struct irc_server *s, struct msg *msg)
{
	(void)msg;

	struct irc_channel *ch;
	struct irc_event ev = {};

	/* Now join all channels that were requested. */
	LL_FOREACH(s->channels, ch)
		irc_server_join(s, ch->name, ch->password);

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

	irc_bot_dispatch(&ev);
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
			modes_parse(s, value);
			irc_log_info("server %s: prefixes:           %s",
			    s->name, value);
		} else if (strcmp(key, "CHANTYPES") == 0) {
			s->chantypes = irc_util_strdupfree(s->chantypes, value);
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

	if (is_self(s, ev.join.origin))
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
	if (is_self(s, ev.kick.target)) {
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

	if (is_self(s, ev.part.origin)) {
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
	if (is_self(s, ev.nick.origin)) {
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
}

static void
handle_error(struct irc_server *s, struct msg *msg)
{
	if (msg->args[0])
		irc_log_warn("server %s: %s", s->name, msg->args[0]);
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

static void
handler(struct irc_server *s, struct msg *msg)
{
	const struct handler *h;

	h = bsearch(msg->cmd, handlers, IRC_UTIL_SIZE(handlers),
	    sizeof (struct handler), handler_cmp);

	if (h)
		h->handle(s, msg);
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
	server_clear(s);
	server_channels_free(s);

	if (s->conn)
		conn_free(s->conn);

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

static inline void
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
	(void)revents;

	struct conn *conn = IRC_UTIL_CONTAINER_OF(self, struct conn, notify_async);

	switch (conn->notify) {
	case CONN_NOTIFY_DISCONNECT:
		irc_log_warn("server %s: connection lost", conn->parent->name);
		handle_disconnect(conn->parent);
		break;
	case CONN_NOTIFY_READY:
		irc_log_info("server %s: connection ready, identifying", conn->parent->name);
		server_identify(conn->parent);
		break;
	case CONN_NOTIFY_INCOMING:
		irc_log_debug("server %s: fetching incoming events", conn->parent->name);
		server_dequeue(conn->parent);
		break;
	default:
		break;
	}

	/* Reset reason for debugging purposes. */
	conn->notify = CONN_NOTIFY_NONE;
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

	if (s->conn) {
		irc_log_info("server %s: already connected", s->name);
		return;
	}

#if !defined(IRCCD_WITH_SSL)
	if (conn->flags & IRC_CONN_SSL) {
		irc_log_warn("server %s: SSL requested but not available", s->name);
		return;
	}
#endif

	/* Create the connection and start it now. */
	s->conn = conn_new(s, server_async_cb);
}

void
irc_server_disconnect(struct irc_server *s)
{
	assert(s);

	if (!s->conn) {
		irc_log_info("server %s: already disconnected", s->name);
		return;
	}

	conn_free(s->conn);
	s->conn = NULL;

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

	struct conn *conn = s->conn;
	char buf[IRC_BUF_LEN];
	va_list ap;

	if (conn->state != CONN_STATE_READY) {
		errno = ENOTCONN;
		return -1;
	}

	va_start(ap, fmt);
	vsnprintf(buf, sizeof (buf), fmt, ap);
	va_end(ap);

	return conn_push(conn, buf, strlen(buf));
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

	const struct conn *conn = s->conn;
	struct irc_channel *ch;
	int ret = 1;

	/*
	 * Search if there is already a channel pending or joined. If the
	 * server is connected we send a join command otherwise we put it there
	 * and wait for connection.
	 */
	if (!(ch = irc_server_find(s, name)))
		ch = channels_add(s, name, pass, IRC_CHANNEL_FLAGS_NONE);

	if (!(ch->flags & IRC_CHANNEL_FLAGS_JOINED) && conn->state == CONN_STATE_READY) {
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
