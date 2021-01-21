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

#include <sys/queue.h>
#include <stdbool.h>
#include <stddef.h>

#include "config.h"

#if defined(IRCCD_WITH_SSL)
#       include <openssl/ssl.h>
#endif

#include "event.h"
#include "limits.h"

struct pollfd;

struct irc_channel;

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
	IRC_SERVER_FLAGS_AUTO_REJOIN   = (1 << 1),
	IRC_SERVER_FLAGS_JOIN_INVITE   = (1 << 2),
	IRC_SERVER_FLAGS_IPV4          = (1 << 3),
	IRC_SERVER_FLAGS_IPV6          = (1 << 4)
};

struct irc_server_prefix {
	char mode;
	char token;
};

#if defined(IRCCD_WITH_SSL)

enum irc_server_ssl_state {
	IRC_SERVER_SSL_NONE,
	IRC_SERVER_SSL_NEED_READ,
	IRC_SERVER_SSL_NEED_WRITE,
};

#endif

struct irc_server_namemode {
	char mode;
	char sym;
	char *name;
};

struct irc_server {
	/* Connection settings. */
	char name[IRC_ID_LEN];
	char hostname[IRC_HOST_LEN];
	char password[IRC_PASSWORD_LEN];
	unsigned short port;
	enum irc_server_flags flags;

	/* Plugin prefix. */
	char commandchar[IRC_CMDCHAR_LEN];

	/* IRC identity. */
	char nickname[IRC_NICKNAME_LEN];
	char username[IRC_USERNAME_LEN];
	char realname[IRC_REALNAME_LEN];
	char ctcpversion[IRC_CTCPVERSION_LEN];

	LIST_HEAD(, irc_channel) channels;

	/* Network connectivity. */
	int fd;
	struct addrinfo *ai;
	struct addrinfo *aip;
	char in[IRC_BUF_LEN];
	char out[IRC_BUF_LEN];
	enum irc_server_state state;

	/* OpenSSL support. */
#if defined(IRCCD_WITH_SSL)
	SSL_CTX *ctx;
	SSL *ssl;
	enum irc_server_ssl_state ssl_state;
#endif

	struct irc_event_whois bufwhois;

	/* Reference count. */
	size_t refc;

	/* IRC server settings. */
	char chantypes[8];
	struct irc_server_prefix prefixes[16];

	LIST_ENTRY(irc_server) link;
};

LIST_HEAD(irc_server_list, irc_server);

struct irc_server *
irc_server_new(const char *,
               const char *,
               const char *,
               const char *,
               const char *,
               unsigned int);

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
irc_server_names(struct irc_server *, const char *);

bool
irc_server_nick(struct irc_server *, const char *);

bool
irc_server_notice(struct irc_server *, const char *, const char *);

bool
irc_server_whois(struct irc_server *, const char *);

struct irc_server_namemode
irc_server_strip(const struct irc_server *, const char *);

void
irc_server_incref(struct irc_server *);

void
irc_server_decref(struct irc_server *);

#endif /* !IRCCD_SERVER_H */
