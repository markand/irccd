/*
 * server.h -- an IRC server
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

#ifndef IRCCD_SERVER_H
#define IRCCD_SERVER_H

#include <stdbool.h>
#include <stddef.h>

#if defined(IRC_HAVE_SSL)
#       include <openssl/ssl.h>
#endif

#include "limits.h"

struct pollfd;

struct irc_channel;
struct irc_event;

enum irc_server_state {
	IRC_SERVER_STATE_DISCONNECTED,
	IRC_SERVER_STATE_CONNECTING,
	IRC_SERVER_STATE_HANDSHAKING,
	IRC_SERVER_STATE_CONNECTED,
	IRC_SERVER_STATE_WAITING,
	IRC_SERVER_STATE_NUM
};

enum irc_server_flags {
	IRC_SERVER_FLAGS_SSL           = (1 << 0),
	IRC_SERVER_FLAGS_AUTO_REJOIN   = (1 << 1)
};

struct irc_server_prefix {
	char mode;
	char token;
};

#if defined(IRCCD_HAVE_SSL)

enum irc_server_ssl_state {
	IRC_SERVER_SSL_NONE,
	IRC_SERVER_SSL_NEED_READ,
	IRC_SERVER_SSL_NEED_WRITE,
};

#endif

struct irc_server {
	/* Connection settings. */
	char name[IRC_NAME_MAX];
	char hostname[IRC_HOST_MAX];
	unsigned short port;
	enum irc_server_flags flags;

	/* Plugin prefix. */
	char commandchar[IRC_COMMANDCHAR_MAX];

	/* IRC identity. */
	char nickname[IRC_NICKNAME_MAX];
	char username[IRC_USERNAME_MAX];
	char realname[IRC_REALNAME_MAX];
	char ctcpversion[IRC_CTCPVERSION_MAX];

	/* Joined channels. */
	struct irc_channel *channels;
	size_t channelsz;

	/* Network connectivity. */
	int fd;
	struct addrinfo *ai;
	struct addrinfo *aip;
	char in[IRC_BUF_MAX];
	char out[IRC_BUF_MAX];
	enum irc_server_state state;

	/* OpenSSL support. */
#if defined(IRCCD_HAVE_SSL)
	SSL_CTX *ctx;
	SSL *ssl;
	enum irc_server_ssl_state ssl_state;
#endif

	/* IRC server settings. */
	char chantypes[8];
	struct irc_server_prefix prefixes[16];
};

void
irc_server_connect(struct irc_server *);

void
irc_server_disconnect(struct irc_server *);

void
irc_server_prepare(const struct irc_server *, struct pollfd *);

void
irc_server_flush(struct irc_server *, const struct pollfd *);

bool
irc_server_poll(struct irc_server *, struct irc_event *);

struct irc_channel *
irc_server_find(struct irc_server *, const char *);

bool
irc_server_send(struct irc_server *, const char *, ...);

bool
irc_server_invite(struct irc_server *, const char *, const char *);

bool
irc_server_join(struct irc_server *, const char *, const char *);

bool
irc_server_kick(struct irc_server *, const char *, const char *, const char *);

bool
irc_server_part(struct irc_server *, const char *, const char *);

bool
irc_server_topic(struct irc_server *, const char *, const char *);

bool
irc_server_message(struct irc_server *, const char *, const char *);

bool
irc_server_me(struct irc_server *, const char *, const char *);

bool
irc_server_mode(struct irc_server *,
                const char *,
                const char *,
                const char *,
                const char *,
                const char *);

bool
irc_server_notice(struct irc_server *, const char *, const char *);

void
irc_server_finish(struct irc_server *);

#endif /* !IRCCD_SERVER_H */
