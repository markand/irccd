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
#include <stddef.h>
#include <time.h>

#include "channel.h"
#include "event.h"
#include "limits.h"

#if defined(__cplusplus)
extern "C" {
#endif

struct pollfd;

struct irc_conn;
struct irc_channel;

enum irc_server_state {
	IRC_SERVER_STATE_NONE,
	IRC_SERVER_STATE_DISCONNECTED,
	IRC_SERVER_STATE_CONNECTING,
	IRC_SERVER_STATE_CONNECTED,
	IRC_SERVER_STATE_WAITING,
	IRC_SERVER_STATE_NUM
};

enum irc_server_flags {
	IRC_SERVER_FLAGS_SSL           = (1 << 0),
	IRC_SERVER_FLAGS_AUTO_REJOIN   = (1 << 1),
	IRC_SERVER_FLAGS_JOIN_INVITE   = (1 << 2),
	IRC_SERVER_FLAGS_AUTO_RECO     = (1 << 3),
	IRC_SERVER_FLAGS_IPV4          = (1 << 4),
	IRC_SERVER_FLAGS_IPV6          = (1 << 5)
};

struct irc_server_user {
	char nickname[IRC_NICKNAME_LEN];
	char username[IRC_USERNAME_LEN];
	char host[IRC_HOST_LEN];
};

struct irc_server_ident {
	char nickname[IRC_NICKNAME_LEN];
	char username[IRC_USERNAME_LEN];
	char realname[IRC_REALNAME_LEN];
	char password[IRC_PASSWORD_LEN];
	char ctcpversion[IRC_CTCPVERSION_LEN];
};

struct irc_server_params {
	char chantypes[IRC_CHANTYPES_LEN];
	char charset[IRC_CHARSET_LEN];
	char casemapping[IRC_CASEMAPPING_LEN];
	unsigned int chanlen;
	unsigned int nicklen;
	unsigned int topiclen;
	unsigned int awaylen;
	unsigned int kicklen;
	struct {
		char mode;              /* Mode (e.g. ov). */
		char symbol;            /* Symbol used (e.g. @+). */
	} prefixes[IRC_USERMODES_LEN];
};

struct irc_server {
	char name[IRC_ID_LEN];
	char prefix[IRC_PREFIX_LEN];
	struct irc_server_ident ident;
	struct irc_server_params params;
	enum irc_server_state state;
	enum irc_server_flags flags;
	struct irc_channel_list channels;
	struct irc_event_whois bufwhois;
	struct irc_conn *conn;
	size_t refc;
	time_t lost_tp, last_tp;
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
irc_server_reconnect(struct irc_server *);

void
irc_server_prepare(const struct irc_server *, struct pollfd *);

void
irc_server_flush(struct irc_server *, const struct pollfd *);

int
irc_server_poll(struct irc_server *, struct irc_event *);

struct irc_channel *
irc_server_find(struct irc_server *, const char *);

int
irc_server_send(struct irc_server *, const char *, ...);

int
irc_server_invite(struct irc_server *, const char *, const char *);

int
irc_server_join(struct irc_server *, const char *, const char *);

int
irc_server_kick(struct irc_server *, const char *, const char *, const char *);

int
irc_server_part(struct irc_server *, const char *, const char *);

int
irc_server_topic(struct irc_server *, const char *, const char *);

int
irc_server_message(struct irc_server *, const char *, const char *);

int
irc_server_me(struct irc_server *, const char *, const char *);

int
irc_server_mode(struct irc_server *, const char *, const char *, const char *);

int
irc_server_names(struct irc_server *, const char *);

int
irc_server_nick(struct irc_server *, const char *);

int
irc_server_notice(struct irc_server *, const char *, const char *);

int
irc_server_whois(struct irc_server *, const char *);

int
irc_server_strip(const struct irc_server *, const char **);

void
irc_server_split(const char *, struct irc_server_user *);

void
irc_server_incref(struct irc_server *);

void
irc_server_decref(struct irc_server *);

#if defined(__cplusplus)
}
#endif

#endif /* !IRCCD_SERVER_H */
