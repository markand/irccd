/*
 * server.h -- an IRC server
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

#ifndef IRCCD_SERVER_H
#define IRCCD_SERVER_H

/**
 * \file server.h
 * \brief An IRC server.
 */

#include <stdarg.h>
#include <stddef.h>

#include "attrs.h"
#include "channel.h"
#include "event.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * \brief Default IRC server port.
 */
#define IRC_SERVER_DEFAULT_PORT 6667

/**
 * \brief Default IRC server command prefix to invoke plugin.
 */
#define IRC_SERVER_DEFAULT_PREFIX "!"

/**
 * \brief Default CTCP version to respond.
 */
#define IRC_SERVER_DEFAULT_CTCP_VERSION "IRC Client Daemon"

/**
 * \brief Default CTCP source to respond.
 */
#define IRC_SERVER_DEFAULT_CTCP_SOURCE "http://hg.malikania.fr/irccd"

struct irc_channel;
struct irc_server_conn;

/**
 * \brief IRC server options.
 */
enum irc_server_flags {
	/**
	 * Connect using SSL encryption.
	 *
	 * Only available when build with OpenSSL support, preprocessor macro
	 * ::IRCCD_WITH_SSL is defined if true.
	 */
	IRC_SERVER_FLAGS_SSL = (1 << 0),

	/**
	 * Automatically rejoin a channel after being kicked.
	 */
	IRC_SERVER_FLAGS_AUTO_REJOIN = (1 << 1),

	/**
	 * Automatically join a server upon invite.
	 */
	IRC_SERVER_FLAGS_JOIN_INVITE = (1 << 2),

	/**
	 * Don't attempt to connect to IPv4 address.
	 */
	IRC_SERVER_FLAGS_NO_IPV4        = (1 << 3),

	/**
	 * Don't attempt to connect to IPv6 address.
	 */
	IRC_SERVER_FLAGS_NO_IPV6        = (1 << 4)
};

/**
 * \brief Describe which user prefix is used for mode.
 */
struct irc_server_umode {
	/**
	 * (read-only)
	 *
	 * User mode on a channel. Example: `o`, `v`, etc.
	 *
	 * See https://defs.ircdocs.horse/defs/chanmodes for a detailed list.
	 */
	char mode;

	/**
	 * (read-only)
	 *
	 * Which symbol character is prepended to a nickname to indicate this
	 * mode.
	 */
	char symbol;
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

	/**
	 * (read-only)
	 *
	 * IRC server modes and their character prefix.
	 */
	struct irc_server_umode *prefixes;

	/**
	 * (read-only)
	 *
	 * Number of elements in ::irc_server::prefixes.
	 */
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

/**
 * Construct a new IRC server with the given name.
 *
 * The server is only created and require few more calls before attempting to
 * connect.
 *
 * \param name the IRC server identifier (not NULL)
 * \return a IRC server
 */
struct irc_server *
irc_server_new(const char *name);

/**
 * Set server hostname.
 *
 * \pre Server must not be connected.
 * \param hostname the new hostname (not NULL)
 */
void
irc_server_set_hostname(struct irc_server *server, const char *hostname);

/**
 * Set server optional flags.
 *
 * \pre Server must not be connected.
 * \param flags the new flags
 */
void
irc_server_set_flags(struct irc_server *server, enum irc_server_flags flags);

/**
 * Set server port.
 *
 * \pre Server must not be connected.
 * \param port the IRC port (in range 1..65535)
 */
void
irc_server_set_port(struct irc_server *server, unsigned int port);

/**
 * Set IRC nickname.
 *
 * If the server is connected, a change request will be send and nickname be
 * applied asynchronously. Otherwise it is replaced immediately.
 *
 * \param nickname the new nickname (not NULL)
 */
void
irc_server_set_nickname(struct irc_server *server, const char *nickname);

/**
 * Set IRC username.
 *
 * \pre Server must not be connected.
 * \param username the IRC username (not NULL)
 */
void
irc_server_set_username(struct irc_server *server, const char *username);

/**
 * Set IRC realname.
 *
 * \pre Server must not be connected.
 * \param username the IRC realname (not NULL)
 */
void
irc_server_set_realname(struct irc_server *server, const char *realname);

/**
 * Set a CTCP reply.
 *
 * Argument key is a CTCP reply which is case-insensitive.
 *
 * If value is NULL, the CTCP reply will be ignored.
 *
 * \param key a CTCP key to reply (not NULL)
 * \param value the CTCP response
 */
void
irc_server_set_ctcp(struct irc_server *server, const char *key, const char *value);

/**
 * Set the command message prefix.
 *
 * The prefix character is used to determine if a message is intented to be
 * parsed for a plugin.
 *
 * Example, by setting '~' as prefix, a message in the form `~ask will I be
 * rich?` will be detected as a command for the plugin `ask` with the content
 * `will I be rich?`.
 *
 * \param prefix the prefix character to set (may be NULL)
 * \sa IRC_SERVER_DEFAULT_PREFIX
 */
void
irc_server_set_prefix(struct irc_server *server, const char *prefix);

/**
 * Set a password to connect to the IRC server.
 *
 * This function must be used before the server is connected.
 *
 * \param password the server password (may be NULL)
 */
void
irc_server_set_password(struct irc_server *server, const char *password);

/**
 * Start the connection mechanism.
 *
 * An IRC server connects and perform all necessary steps on its own including
 * trying to reconnect when error occurs.
 *
 * For convenience, this function does nothing if the server is already
 * connected.
 */
void
irc_server_connect(struct irc_server *server);

/**
 * Force disconnection of the server.
 *
 * If the server was successfully connected, plugins are notified of the
 * disconnection.
 *
 * For convenience, this function does nothing if the server is already
 * disconnected.
 */
void
irc_server_disconnect(struct irc_server *server);

/**
 * Force a reconnection immediately.
 *
 * This function is primarily designed to force a reconnection immediately when
 * a server was disconnected and user do not want to wait the delay before
 * retrying. It's also possible to use it whenever the server seems disconnected
 * but irccd was unable to detect it on its own.
 */
void
irc_server_reconnect(struct irc_server *server);

/**
 * Find a IRC channel by name.
 *
 * \param name the channel name to find
 * \return the channel or NULL if not found
 */
const struct irc_channel *
irc_server_channels_find(struct irc_server *server, const char *name);

/**
 * Send a raw message to the IRC server using printf(3) format style.
 *
 * The message must not be terminated by `\r\n` which is done by the function
 * itself.
 *
 * \warning use this function with care
 * \param fmt the format string
 * \return 0 on success
 * \return -ENOTCONN if the server is not connected
 * \return -ENOBUFS if the output buffer could not fit the message
 */
IRC_ATTR_PRINTF(2, 3)
int
irc_server_send(struct irc_server *server, const char *fmt, ...);

/**
 * Similar to ::irc_server_send using a `va_list`.
 */
IRC_ATTR_PRINTF(2, 0)
int
irc_server_send_va(struct irc_server *server, const char *fmt, va_list ap);

/**
 * Invite the target nickname into the channel.
 *
 * \param channel the channel name
 * \param target the nickname target
 * \return 0 on success or -1 on error
 */
int
irc_server_invite(struct irc_server *server, const char *channel, const char *target);

/**
 * Join a channel.
 *
 * If the channel is already joined, this function does nothing.
 *
 * If the server is not connected, the channel is added to the list to be joined
 * upon successful connection. Otherwise the join command is sent immediately.
 *
 * \param name the channel name to join
 * \param password an optional password (may be NULL)
 * \return 0 on success or -1 on error
 */
int
irc_server_join(struct irc_server *server, const char *name, const char *password);

/**
 * Kick someone from a channel.
 *
 * \param channel the channel name
 * \param target the target nickname to kick
 * \param reason an optional reason (may be NULL)
 * \return 0 on success or -1 on error
 */
int
irc_server_kick(struct irc_server *server, const char *channel, const char *target, const char *reason);

/**
 * Leave a channel.
 *
 * \param channel the channel name to leave
 * \param reason an optional reason (may be NULL)
 * \return 0 on success or -1 on error
 */
int
irc_server_part(struct irc_server *server, const char *channel, const char *reason);

/**
 * Change a channel topic content.
 *
 * \param channel the channel name
 * \param topic the new topic
 * \return 0 on success or -1 on error
 */
int
irc_server_topic(struct irc_server *server, const char *channel, const char *topic);

/**
 * Send a message to a channel or a nickname.
 *
 * \param target the channel or target
 * \param message the message content
 * \return 0 on success or -1 on error
 */
int
irc_server_message(struct irc_server *server, const char *target, const char *message);

/**
 * Send a CTCP action emote to a channel or a nickname (like `/me` in most
 * clients).
 *
 * \param target the channel or target
 * \param message the message content
 * \return 0 on success or -1 on error
 */
int
irc_server_me(struct irc_server *server, const char *target, const char *message);

/**
 * Set both user or channel modes.
 *
 * \param target the channel or target
 * \param modes the list of modes
 * \param args the arguments to the modes
 * \return 0 on success or -1 on error
 */
int
irc_server_mode(struct irc_server *server, const char *target, const char *mode, const char *args);

/**
 * Request a names listing for a channel.
 *
 * This function is asynchronous and will generate a ::irc_event of type names
 * when the list has been received entirely.
 *
 * \param channel the channel name
 * \return 0 on success or -1 on error
 */
int
irc_server_names(struct irc_server *server, const char *channel);

/**
 * Send a private notice.
 *
 * \param target the channel or target (not NULL)
 * \param message the notice content (not NULL)
 * \return 0 on success or -1 on error
 */
int
irc_server_notice(struct irc_server *server, const char *target, const char *message);

/**
 * Request a whois information for a target.
 *
 * This function is asynchronous and will generate a ::irc_event of type whois
 * when the information has been received entirely.
 *
 * \param target the target nickname (not NULL)
 * \return 0 on success or -1 on error
 */
int
irc_server_whois(struct irc_server *server, const char *target);

/**
 * Strip a user nickname from its optional prefixes if any.
 *
 * For example, various event could possibly add a prefix to a nickname such as
 * `@markand`, this function will advance the pointer so that only `markand`
 * shows up. It is necessary to use this function than rolling out your own as
 * prefixes are server defined which means that both `@markand` or `[markand]`
 * could be appropriate nicknames if neither `@` or `[` are user mode prefixes.
 *
 * This function also returns the modes applied to the nickname if present.
 *
 * \param nickname the nickname to shift
 * \return the modes for this nickname or 0 if none (and nickname is untouched)
 */
int
irc_server_strip(const struct irc_server *server, const char **nickname);

/**
 * Increment the reference count for this server.
 *
 * This function is mostly intended for plugins which may keep track of a server
 * while being removed from the bot.
 */
void
irc_server_incref(struct irc_server *server);

/**
 * Decrement the server and possibly freeing it if it was the last reference
 * count.
 */
void
irc_server_decref(struct irc_server *server);

#if defined(__cplusplus)
}
#endif

#endif /* !IRCCD_SERVER_H */
