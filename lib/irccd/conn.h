/*
 * conn.h -- private connection handler and helpers
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

#ifndef IRCCD_CONN_H
#define IRCCD_CONN_H

/*
 * \file conn.h
 * \brief Private connection handler and helpers.
 *
 * This module implements the IRC handshake and reconnection including
 * authentication as well.
 *
 * It also wraps the SSL/TLS transparently.
 *
 * It is implemented as a state machine using a coroutine, the server will
 * dequeue events using irc__conn_pull function.
 */

#include <sys/types.h>
#include <stddef.h>

#include <nce/io.h>
#include <nce/timer.h>

struct addrinfo;
struct conn;
struct conn_msg;
struct irc_server;
struct tls;

#define CONN_IN_SIZE 8096
#define CONN_OUT_SIZE 65536

/*
 * Private abstraction to the server connection using either plain or SSL
 * transport.
 */
struct conn {
	/* Connection state. */
	enum {
		STATE_RESOLVE,
		STATE_CONNECT,
		STATE_IDENT,
		STATE_READY
	} state;

	/* Link to the parent server. */
	struct irc_server *parent;

	/*
	 * Pointer to current endpoint to try and pointer to the original
	 * list.
	 */
	struct addrinfo *ai;
	struct addrinfo *ai_list;

	/*
	 * Input & output buffer and their respective sizes, not NUL
	 * terminated.
	 *
	 * Since conn is allocated on the heap its fine to have a fixed size
	 * array here.
	 */
	char in[CONN_IN_SIZE];
	size_t insz;
	char out[CONN_OUT_SIZE];
	size_t outsz;

	int fd;
	struct nce_io_coro io_fd;
	struct nce_timer_coro timer;
	struct nce_coro producer;

	/* OpenBSD's nice libtls. */
#ifdef IRCCD_WITH_SSL
	struct tls *tls;
#endif

	/* Transport callbacks */
	ssize_t (*recv)(struct conn *, void *, size_t, int *);
	ssize_t (*send)(struct conn *, const void *, size_t, int *);
};

/**
 * \struct conn_msg
 * \brief A raw IRC message parsed.
 */
struct conn_msg {
	int status;
	char *prefix;
	char *cmd;
	char **args;
	size_t argsz;
	char *buf;
};

/**
 * Create the connection object state machine.
 */
void
irc__conn_spawn(struct conn *conn, struct irc_server *parent);

/**
 * Check if connection is ready.
 *
 * This function returns true if the socket is connected and that at least the
 * IRC protocol has been successfully identified.
 */
int
irc__conn_ready(const struct conn *conn);

/**
 * Push some data to the output stream.
 *
 * The data must not be terminated by \r\n.
 *
 * \param data the data to push
 * \param datasz size of data to push
 * \return 0 on success
 * \return -ENOBUFS if not enough space
 */
int
irc__conn_push(struct conn *conn, const char *data, size_t datasz);

/**
 * Yield until data is available.
 */
void
irc__conn_pull(struct conn *conn, struct conn_msg *msg);

void
irc__conn_destroy(struct conn *conn);

int
irc__conn_msg_parse(struct conn_msg *msg, const char *line, size_t linesz);

int
irc__conn_msg_is_ctcp(const char *line);

char *
irc__conn_msg_ctcp(char *line);

void
irc__conn_msg_finish(struct conn_msg *msg);

#endif /* !IRCCD_CONN_H */
