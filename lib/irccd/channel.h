/*
 * channel.h -- an IRC server channel
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

#ifndef IRCCD_CHANNEL_H
#define IRCCD_CHANNEL_H

/**
 * \file channel.h
 * \brief an IRC server channel.
 */

#include <stddef.h>

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * \brief Describe a channel user.
 */
struct irc_channel_user {
	/**
	 * (read-write)
	 *
	 * Nickname.
	 */
	char *nickname;

	/**
	 * (read-write)
	 *
	 * Modes for this specific user.
	 */
	int modes;

	/**
	 * \cond IRC_PRIVATE
	 */

	/**
	 * (private)
	 *
	 * Next user in the linked list.
	 */
	struct irc_channel_user *next;

	/**
	 * \endcond IRC_PRIVATE
	 */
};

/**
 * \brief Channel flags.
 */
enum irc_channel_flags {
	/**
	 * No flags.
	 */
	IRC_CHANNEL_FLAGS_NONE   = (0),

	/**
	 * Channel is joined.
	 */
	IRC_CHANNEL_FLAGS_JOINED = (1 << 0)
};

/**
 * \brief Describe a IRC channel
 *
 * This structure is handled by a ::irc_server and modified to contain a list
 * of users upon join/parts and so on.
 *
 * It is also there to serve the purpose of auto-joining channel and as such,
 * is present in the server list even if the server is not connected yet. The
 * field ::irc_channel::flags can be used to detect its condition.
 *
 * All of the fields should not be edited directly.
 */
struct irc_channel {
	/**
	 * (read-only)
	 *
	 * Channel name (including possible prefix).
	 */
	char *name;

	/**
	 * (read-only, optional)
	 *
	 * An optional password required to join a channel.
	 */
	char *password;

	/**
	 * (read-only)
	 *
	 * Channel flags.
	 */
	enum irc_channel_flags flags;

	/**
	 * (read-only, optional)
	 *
	 * List of users present in the channel.
	 */
	struct irc_channel_user *users;

	/**
	 * \cond IRC_PRIVATE
	 */

	/**
	 * (private)
	 *
	 * Next channel in the linked list.
	 */
	struct irc_channel *next;

	/**
	 * \endcond IRC_PRIVATE
	 */
};

/**
 * Create a new IRC channel.
 *
 * Note: the name of the channel will be set to lowercase to find channels
 * using case-insensitive names as IRC protocol especially allows mixed case.
 *
 * \pre name != NULL
 * \param name the channel name
 * \param password an optional password if not NULL
 * \param flags optional channel flags
 * \return a newly allocated channel
 */
struct irc_channel *
irc_channel_new(const char *name,
                const char *password,
                enum irc_channel_flags flags);

/**
 * Register a nickname into the channel.
 *
 * \pre ch != NULL
 * \pre nickname != NULL
 * \param ch the channel to update
 * \param nickname the new nickname to add (will be copied)
 * \param modes the user's modes
 */
void
irc_channel_add(struct irc_channel *ch, const char *nickname, int modes);

/**
 * Find a user from the channel.
 *
 * \pre ch != NULL
 * \pre nickname != NULL
 * \param ch the channel to update
 * \param nickname the user nickname to find
 * \return the user information if found, NULL otherwise
 */
const struct irc_channel_user *
irc_channel_get(const struct irc_channel *ch, const char *nickname);

/**
 * Update user modes in a channel.
 *
 * \pre ch != NULL
 * \pre nickname != NULL
 * \param ch the channel to update
 * \param nickname the user nickname to find
 * \param modes the new modes to apply
 */
void
irc_channel_set(struct irc_channel *ch, const char *nickname, int modes);

/**
 * Clear the channel's user list.
 *
 * \pre ch != NULL
 * \param ch the channel to clear
 */
void
irc_channel_clear(struct irc_channel *ch);

/**
 * Indicate how many users are present in the channel.
 *
 * \pre ch != NULL
 * \param ch the channel
 * \return the number of users
 */
size_t
irc_channel_count(const struct irc_channel *ch);

/**
 * Remove a user from the channel, if present.
 *
 * \pre ch != NULL
 * \pre nickname != NULL
 * \param ch the channel to update
 * \param nickname the user nickname to remove
 */
void
irc_channel_remove(struct irc_channel *ch, const char *nickname);

/**
 * Free the channel entirely.
 *
 * \param ch the channel to free (maybe NULL)
 */
void
irc_channel_free(struct irc_channel *ch);

#if defined(__cplusplus)
}
#endif

#endif /* !IRCCD_CHANNEL_H */
