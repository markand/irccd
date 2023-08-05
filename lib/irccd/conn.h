/*
 * conn.h -- abstract IRC server connection
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

enum irc__conn_state {
	IRC__CONN_STATE_NONE,           /* Nothing, default. */
	IRC__CONN_STATE_CONNECTING,     /* Pending connect(2) call. */
	IRC__CONN_STATE_HANDSHAKING,    /* SSL connect+handshake. */
	IRC__CONN_STATE_READY           /* Ready for I/O. */
};

enum irc__conn_flags {
	IRC__CONN_SSL = (1 << 0)
};

#if defined(IRCCD_WITH_SSL)

enum irc__conn_ssl_act {
	IRC__CONN_SSL_ACT_NONE,
	IRC__CONN_SSL_ACT_READ,
	IRC__CONN_SSL_ACT_WRITE,
};

#endif

struct irc__conn {
	char hostname[IRC_HOST_LEN];
	unsigned short port;
	int fd;
	struct addrinfo *ai;
	struct addrinfo *aip;
	char in[IRC_BUF_LEN];
	char out[IRC_BUF_LEN];
	enum irc__conn_state state;
	enum irc__conn_flags flags;
	struct irc_server *sv;
	time_t statetime;

#if defined(IRCCD_WITH_SSL)
	SSL_CTX *ctx;
	SSL *ssl;
	enum irc__conn_ssl_act ssl_cond;
	enum irc__conn_ssl_act ssl_step;
#endif
};

struct irc__conn_msg {
	char *prefix;
	char *cmd;
	char *args[IRC_ARGS_MAX];
	char buf[IRC_MESSAGE_LEN];
};

int
irc__conn_connect(struct irc__conn *);

void
irc__conn_disconnect(struct irc__conn *);

void
irc__conn_prepare(const struct irc__conn *, struct pollfd *);

int
irc__conn_flush(struct irc__conn *, const struct pollfd *);

int
irc__conn_poll(struct irc__conn *, struct irc__conn_msg *);

int
irc__conn_send(struct irc__conn *, const char *);

void
irc__conn_finish(struct irc__conn *);

#if defined(__cplusplus)
}
#endif

#endif /* !IRCCD_CONN_H */
