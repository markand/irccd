/*
 * conn.h -- an IRC server channel
 *
 * Copyright (c) 2013-2023 David Demelier <markand@malikania.fr>
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

#include "config.h"

#if defined(IRCCD_WITH_SSL)
#       include <openssl/ssl.h>
#endif

#include <time.h>

#include "limits.h"

#if defined(__cplusplus)
extern "C" {
#endif

struct addrinfo;
struct pollfd;

struct irc_server;

enum irc_conn_state {
	IRC_CONN_STATE_NONE,            /* Nothing, default. */
	IRC_CONN_STATE_CONNECTING,      /* Pending connect(2) call. */
	IRC_CONN_STATE_HANDSHAKING,     /* SSL connect+handshake. */
	IRC_CONN_STATE_READY            /* Ready for I/O. */
};

enum irc_conn_flags {
	IRC_CONN_SSL = (1 << 0)
};

#if defined(IRCCD_WITH_SSL)

enum irc_conn_ssl_act {
	IRC_CONN_SSL_ACT_NONE,
	IRC_CONN_SSL_ACT_READ,
	IRC_CONN_SSL_ACT_WRITE,
};

#endif

struct irc_conn {
	char hostname[IRC_HOST_LEN];
	unsigned short port;
	int fd;
	struct addrinfo *ai;
	struct addrinfo *aip;
	char in[IRC_BUF_LEN];
	char out[IRC_BUF_LEN];
	enum irc_conn_state state;
	enum irc_conn_flags flags;
	struct irc_server *sv;
	time_t statetime;

#if defined(IRCCD_WITH_SSL)
	SSL_CTX *ctx;
	SSL *ssl;
	enum irc_conn_ssl_act ssl_cond;
	enum irc_conn_ssl_act ssl_step;
#endif
};

struct irc_conn_msg {
	char *prefix;
	char *cmd;
	char *args[IRC_ARGS_MAX];
	char buf[IRC_MESSAGE_LEN];
};

int
irc_conn_connect(struct irc_conn *);

void
irc_conn_disconnect(struct irc_conn *);

void
irc_conn_prepare(const struct irc_conn *, struct pollfd *);

int
irc_conn_flush(struct irc_conn *, const struct pollfd *);

int
irc_conn_poll(struct irc_conn *, struct irc_conn_msg *);

int
irc_conn_send(struct irc_conn *, const char *);

void
irc_conn_finish(struct irc_conn *);

#if defined(__cplusplus)
}
#endif

#endif /* !IRCCD_CONN_H */
