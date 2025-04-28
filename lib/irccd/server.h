/*
 * server.h -- an IRC server
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

#ifndef IRCCD_SERVER_H
#define IRCCD_SERVER_H

#include <stddef.h>
#include <time.h>

#include "channel.h"
#include "event.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define IRC_SERVER_DEFAULT_PORT 6667
#define IRC_SERVER_DEFAULT_PREFIX "!"
#define IRC_SERVER_DEFAULT_CTCP_VERSION "IRC Client Daemon"

struct irc__conn;
struct irc_channel;

/**
 * \brief Current IRC server state.
 */
enum irc_server_state {
	/**
	 * Socket closed, no connection.
	 */
	IRC_SERVER_STATE_DISCONNECTED,

	/**
	 * Raw TCP socket connection in progress.
	 */
	IRC_SERVER_STATE_CONNECTING,

	/**
	 * SSL connection is handshaking.
	 */
	IRC_SERVER_STATE_HANDSHAKING,

	/**
	 * IRC server protocol exchange and authentication.
	 */
	IRC_SERVER_STATE_IDENTIFYING,

	/**
	 * Connection complete and running.
	 */
	IRC_SERVER_STATE_RUNNING,

	/**
	 * Unused sentinel value.
	 */
	IRC_SERVER_STATE_LAST
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
	char *nickname;
	char *username;
	char *host;
};

struct irc_server_umode {
	char mode;              /* Mode (e.g. ov). */
	char symbol;            /* Symbol used (e.g. @+). */
};

/**
 * \brief IRC server connection
 *
 * This large structure holds the connection to the IRC server and all of its
 * information including joined channels and such.
 *
 * All fields are visible to the user for convenience but should only be edited
 * through the appropriate `irc_server_set_*` functions.
 */
struct irc_server {
	/**
	 * (read-only)
	 *
	 * Server name.
	 */
	char *name;

	/**
	 * (read-only)
	 *
	 * IRC hostname.
	 */
	char *hostname;

	/**
	 * (read-only)
	 *
	 * IRC server port number.
	 */
	unsigned int port;


	/**
	 * (read-only, optional)
	 *
	 * Prefix command character to invoke plugins.
	 */
	char *prefix;

	/**
	 * (read-only)
	 *
	 * IRC nickname to use.
	 */
	char *nickname;

	/**
	 * (read-only)
	 *
	 * IRC username to use.
	 */
	char *username;

	/**
	 * (read-only)
	 *
	 * IRC real name to use.
	 */
	char *realname;

	/**
	 * (read-only, optional)
	 *
	 * Password for connecting to the IRC server.
	 */
	char *password;

	/**
	 * (read-only, optional)
	 *
	 * CTCP response to VERSION.
	 */
	char *ctcp_version;

	/**
	 * (read-only, optional)
	 *
	 * CTCP response to SOURCE.
	 */
	char *ctcp_source;

	/**
	 * (read-only)
	 *
	 * Server flags.
	 */
	enum irc_server_flags flags;

	/**
	 * (read-only)
	 *
	 * Current server state.
	 */
	enum irc_server_state state;

	/**
	 * (read-only)
	 *
	 * List of channels to join or joined.
	 */
	struct irc_channel *channels;

	/**
	 * (read-only, optional)
	 *
	 * Channel types prefixes.
	 *
	 * This string contains every characters that are allowed in the
	 * beginning of a channel name (e.g. `#~`).
	 */
	char *chantypes;

	/**
	 * (read-only, optional)
	 *
	 * Charset that the server expects.
	 */
	char *charset;

	/**
	 * (read-only, optional)
	 *
	 */
	char *casemapping;

	/**
	 * (read-only)
	 *
	 * Maximum length for a channel.
	 */
	unsigned int channel_max;

	/**
	 * (read-only)
	 *
	 * Maximum length for a nickname.
	 */
	unsigned int nickname_max;

	/**
	 * (read-only)
	 *
	 * Maximum length for a topic.
	 */
	unsigned int topic_max;

	/**
	 * (read-only)
	 *
	 * Maximum length for an away message.
	 */
	unsigned int away_max;

	/**
	 * (read-only)
	 *
	 * Maximum length for a kick reason message.
	 */
	unsigned int kick_max;

	struct irc_server_umode *prefixes;
	size_t prefixesz;

	/**
	 * \cond IRC_PRIVATE
	 */

	/**
	 * (private)
	 *
	 * Private connection handle.
	 */
	void *conn;

	/**
	 * (private)
	 *
	 * Whois being constructed.
	 */
	struct irc_event_whois bufwhois;

	/**
	 * (private)
	 *
	 * Reference count.
	 */
	size_t refc;

	/**
	 * (private)
	 *
	 * Next server in the linked list.
	 */
	struct irc_server *next;

	/**
	 * \endcond IRC_PRIVATE
	 */
};

struct irc_server *
irc_server_new(const char *name);

void
irc_server_set_ident(struct irc_server *s,
                     const char *nickname,
                     const char *username,
                     const char *realname);

void
irc_server_set_params(struct irc_server *s,
                      const char *hostname,
                      unsigned int port,
                      enum irc_server_flags flags);

void
irc_server_set_ctcp(struct irc_server *s,
                    const char *version,
                    const char *source);

void
irc_server_set_prefix(struct irc_server *s, const char *prefix);

void
irc_server_set_password(struct irc_server *s, const char *password);

void
irc_server_connect(struct irc_server *);

void
irc_server_disconnect(struct irc_server *);

void
irc_server_reconnect(struct irc_server *);

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
