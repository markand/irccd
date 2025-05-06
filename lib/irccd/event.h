/*
 * event.h -- IRC event
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

#ifndef IRCCD_EVENT_H
#define IRCCD_EVENT_H

#include <stddef.h>

#if defined(__cplusplus)
extern "C" {
#endif

struct irc_server;

/**
 * \brief Describe an IRC event.
 *
 * This enumeration indicates which kind of event (not mandatory IRC related)
 * happened on a server. It is used in conjunction with underlying sub
 * structures of type irc_event_* that is store into the generic irc_event
 * structure.
 *
 * Some events may contain specific data that is accessible via the anonymous
 * union inside the struct irc_event, refer to the enumerator description to
 * check which member must be accessed in that case.
 */
enum irc_event_type {
	/**
	 * Not a valid event type.
	 */
	IRC_EVENT_UNKNOWN,

	/**
	 * Specific plugin invocation.
	 *
	 * This is happening when a user invoke a plugin by combining the server
	 * prefix and plugin identifier like `!plugin list`. The message field
	 * may be used with this event.
	 *
	 * Use with ::irc_event_message (and ::irc_event::message).
	 */
	IRC_EVENT_COMMAND,

	/**
	 * When a server is successfully connected.
	 *
	 * No field used within ::irc_event.
	 */
	IRC_EVENT_CONNECT,

	/**
	 * When a server has disconnected.
	 *
	 * No field used within ::irc_event.
	 */
	IRC_EVENT_DISCONNECT,

	/**
	 * When an invite event has been received.
	 *
	 * Use with ::irc_event_invite (and ::irc_event::invite).
	 */
	IRC_EVENT_INVITE,

	/**
	 * When a join event has been received.
	 *
	 * Use with ::irc_event_join (and ::irc_event::join).
	 */
	IRC_EVENT_JOIN,

	/**
	 * When someone has been kicked out from a channel.
	 *
	 * Use with ::irc_event_kick (and ::irc_event::kick).
	 */
	IRC_EVENT_KICK,

	/**
	 * Event  about  CTCP  ACTION  (also known as
	 * /me) received.
	 *
	 * Use with ::irc_event_message (and ::irc_event::message).
	 */
	IRC_EVENT_ME,

	/**
	 * Event when a standard message has been received inside a channel or
	 * from a query.
	 *
	 * Use with ::irc_event_message (and ::irc_event::message).
	 */
	IRC_EVENT_MESSAGE,

	/**
	 * Event when a channel mode or the irccd daemon modes have changed.
	 *
	 * Use with ::irc_event_mode (and ::irc_event::mode).
	 */
	IRC_EVENT_MODE,

	/**
	 * Event when all names from a channel has been received.
	 *
	 * Use with ::irc_event_names (and ::irc_event::names).
	 */
	IRC_EVENT_NAMES,

	/**
	 * Event when someone's nickname has been changed which can also target
	 * the bot itself.
	 *
	 * Note: the bot tracks its own nickname and update the server
	 * accordingly.
	 *
	 * Use with ::irc_event_nick (and ::irc_event::nick).
	 */
	IRC_EVENT_NICK,

	/**
	 * Event when a notice has been received.
	 *
	 * Use with ::irc_event_notice (and ::irc_event::notice).
	 */
	IRC_EVENT_NOTICE,

	/**
	 * Event when someone left a channel.
	 *
	 * Use with ::irc_event_part (and ::irc_event::part).
	 */
	IRC_EVENT_PART,

	/**
	 * Event when a channel topic has been changed.
	 *
	 * Use with ::irc_event_topic (and ::irc_event::topic).
	 */
	IRC_EVENT_TOPIC,

	/**
	 * Event when a whois information has been received about a user.
	 *
	 * Use with ::irc_event_whois (and ::irc_event::whois).
	 */
	IRC_EVENT_WHOIS
};

/**
 * \brief Invite event.
 */
struct irc_event_invite {
	/**
	 * (read-only)
	 *
	 * Event origin.
	 */
	char *origin;

	/**
	 * (read-only)
	 *
	 * The channel where the bot is invited to.
	 */
	char *channel;
};

/**
 * \brief Join event.
 */
struct irc_event_join {
	/**
	 * (read-only)
	 *
	 * Event origin.
	 */
	char *origin;

	/**
	 * (read-only)
	 *
	 * The channel that the nickname joined.
	 */
	char *channel;
};

/**
 * \brief Kick event.
 */
struct irc_event_kick {
	/**
	 * (read-only)
	 *
	 * Event origin.
	 */
	char *origin;

	/**
	 * (read-only)
	 *
	 * The channel on which the target was kicked from.
	 */
	char *channel;

	/**
	 * (read-only)
	 *
	 * The target that was kicked.
	 */
	char *target;

	/**
	 * (read-only, optional)
	 *
	 * The reason why the target has been kicked.
	 */
	char *reason;
};

/**
 * \brief Command or normal message event.
 */
struct irc_event_message {
	/**
	 * (read-only)
	 *
	 * Event origin.
	 */
	char *origin;

	/**
	 * (read-only)
	 *
	 * The channel or nickname target.
	 */
	char *channel;

	/**
	 * (read-only)
	 *
	 * The message content.
	 */
	char *message;
};

/**
 * \brief Channel or user mode change event.
 */
struct irc_event_mode {
	/**
	 * (read-only)
	 *
	 * Event origin.
	 */
	char *origin;

	/**
	 * (read-only)
	 *
	 * The channel or irccd's nickname on which mode were changed.
	 */
	char *channel;

	/**
	 * (read-only)
	 *
	 * The mode character.
	 */
	char *mode;

	/**
	 * (read-only, optional)
	 *
	 * A NULL terminated list of arguments.
	 */
	char **args;
};

/**
 * \brief End of names listing event.
 */
struct irc_event_names {
	/**
	 * (read-only)
	 *
	 * The channel were the names list was generated.
	 */
	char *channel;

	/**
	 * (read-only, optional)
	 *
	 * A list of stripped nicknames present in the channel and their
	 * associated modes.
	 */
	struct {
		/**
		 * (read-only)
		 *
		 * Stripped nickname.
		 */
		char *nickname;

		/**
		 * (read-only)
		 *
		 * Optional user modes in this channel as bitmask representing
		 * mode index in ::irc_server::prefixes.
		 */
		int modes;
	} *users;

	/**
	 * (read-only)
	 *
	 * Number of elements in ::irc_event_names::users.
	 */
	size_t usersz;
};

/**
 * \brief Nick change event.
 */
struct irc_event_nick {
	/**
	 * (read-only)
	 *
	 * Event origin.
	 */
	char *origin;

	/**
	 * (read-only)
	 *
	 * The new nickname.
	 */
	char *nickname;
};

/**
 * \brief Notice event.
 */
struct irc_event_notice {
	/**
	 * (read-only)
	 *
	 * Event origin.
	 */
	char *origin;

	/**
	 * (read-only)
	 *
	 * The channel or target that receives the notice.
	 */
	char *channel;

	/**
	 * (read-only)
	 *
	 * The notice message content.
	 */
	char *notice;
};

/**
 * \brief Part event.
 */
struct irc_event_part {
	/**
	 * (read-only)
	 *
	 * Event origin.
	 */
	char *origin;

	/**
	 * (read-only)
	 *
	 * The channel on which the nickname left.
	 */
	char *channel;

	/**
	 * (read-only, optional)
	 *
	 * The reason why the nickname left the channel.
	 */
	char *reason;
};

/**
 * \brief Topic change event.
 */
struct irc_event_topic {
	/**
	 * (read-only)
	 *
	 * Event origin.
	 */
	char *origin;

	/**
	 * (read-only)
	 *
	 * The channel on which the topic has been changed.
	 */
	char *channel;

	/**
	 * (read-only)
	 *
	 * The new topic.
	 */
	char *topic;
};

/**
 * \brief End of whois information event.
 */
struct irc_event_whois {
	/**
	 * (read-only)
	 *
	 * Nickname.
	 */
	char *nickname;

	/**
	 * (read-only)
	 *
	 * User name.
	 */
	char *username;

	/**
	 * (read-only)
	 *
	 * Real name.
	 */
	char *realname;

	/**
	 * (read-only)
	 *
	 * Hostname part.
	 */
	char *hostname;

	/**
	 * (read-only)
	 *
	 * Channels on which the user is present.
	 *
	 * Note: this list may be empty if the user decides to hide its channel
	 * list.
	 */
	struct {
		/**
		 * (read-only)
		 *
		 * The channel name.
		 */
		char *name;

		/**
		 * (read-only)
		 *
		 * User modes on this channel.
		 */
		int modes;
	} *channels;

	/**
	 * (read-only)
	 *
	 * Number of elements in ::irc_event_whois::channels;
	 */
	size_t channelsz;
};

/**
 * \brief Generic fat IRC event
 *
 * This structure holds every kind of event using a tagged union and a server as
 * common fields for all events.
 */
struct irc_event {
	/**
	 * (read-only)
	 *
	 * Event type.
	 *
	 * See the enumeration constants to find out which appropriate union
	 * member you can access.
	 */
	enum irc_event_type type;

	/**
	 * (read-only)
	 *
	 * The server that generated the event.
	 */
	struct irc_server *server;

	/**
	 * (read-only, optional)
	 *
	 * The union grouping all kind of events.
	 */
	union {
		/**
		 * (read-only)
		 *
		 * For use with ::IRC_EVENT_INVITE.
		 */
		struct irc_event_invite invite;

		/**
		 * (read-only)
		 *
		 * For use with ::IRC_EVENT_JOIN.
		 */
		struct irc_event_join join;

		/**
		 * (read-only)
		 *
		 * For use with ::IRC_EVENT_KICK.
		 */
		struct irc_event_kick kick;

		/**
		 * (read-only)
		 *
		 * For use with ::IRC_EVENT_COMMAND.
		 * For use with ::IRC_EVENT_MESSAGE.
		 */
		struct irc_event_message message;

		/**
		 * (read-only)
		 *
		 * For use with ::IRC_EVENT_MODE.
		 */
		struct irc_event_mode mode;

		/**
		 * (read-only)
		 *
		 * For use with ::IRC_EVENT_NAMES.
		 */
		struct irc_event_names names;

		/**
		 * (read-only)
		 *
		 * For use with ::IRC_EVENT_NICK.
		 */
		struct irc_event_nick nick;

		/**
		 * (read-only)
		 *
		 * For use with ::IRC_EVENT_NOTICE.
		 */
		struct irc_event_notice notice;

		/**
		 * (read-only)
		 *
		 * For use with ::IRC_EVENT_PART.
		 */
		struct irc_event_part part;

		/**
		 * (read-only)
		 *
		 * For use with ::IRC_EVENT_TOPIC.
		 */
		struct irc_event_topic topic;

		/**
		 * (read-only)
		 *
		 * For use with ::IRC_EVENT_WHOIS.
		 */
		struct irc_event_whois whois;
	};
};

/**
 * Convert an event into a human readable string.
 *
 * \pre ev != NULL
 * \pre str != NULL
 * \param ev the event to stringify
 * \param str the destination string
 * \param strsz the maximum number of bytes to write in str
 * \return 0 on success or -1 on error
 */
int
irc_event_str(const struct irc_event *ev, char *str, size_t strsz);

/**
 * Destroy the event.
 *
 * This function is usually not needed from user code.
 *
 * \pre ev != NULL
 * \param ev the event to destroy
 */
void
irc_event_finish(struct irc_event *ev);

#if defined(__cplusplus)
}
#endif

#endif /* !IRCCD_EVENT_H */
