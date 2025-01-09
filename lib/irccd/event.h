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

#include "limits.h"

#if defined(__cplusplus)
extern "C" {
#endif

struct irc_server;

/*
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
	IRC_EVENT_UNKNOWN,      /* Unknown or private use.      */
	IRC_EVENT_COMMAND,      /* Use irc_event_message.       */
	IRC_EVENT_CONNECT,      /* No specific data.            */
	IRC_EVENT_DISCONNECT,   /* No specific data.            */
	IRC_EVENT_INVITE,       /* Use irc_event_invite.        */
	IRC_EVENT_JOIN,         /* Use irc_event_join.          */
	IRC_EVENT_KICK,         /* Use irc_event_kick.          */
	IRC_EVENT_ME,           /* Use irc_event_message        */
	IRC_EVENT_MESSAGE,      /* Use irc_event_message.       */
	IRC_EVENT_MODE,         /* Use irc_event_mode.          */
	IRC_EVENT_NAMES,        /* Use irc_event_names.         */
	IRC_EVENT_NICK,         /* Use irc_event_nick.          */
	IRC_EVENT_NOTICE,       /* Use irc_event_notice.        */
	IRC_EVENT_PART,         /* Use irc_event_part.          */
	IRC_EVENT_TOPIC,        /* Use irc_event_topic.         */
	IRC_EVENT_WHOIS         /* Use irc_event_whois.         */
};

struct irc_event_invite {
	char *origin;
	char *channel;
};

struct irc_event_join {
	char *origin;
	char *channel;
};

struct irc_event_kick {
	char *origin;
	char *channel;
	char *target;
	char *reason;
};

struct irc_event_message {
	char *origin;
	char *channel;
	char *message;
};

struct irc_event_mode {
	char *origin;
	char *channel;
	char *mode;
	char **args;
};

struct irc_event_names {
	char *channel;
	char *names;
};

struct irc_event_nick {
	char *origin;
	char *nickname;
};

struct irc_event_notice {
	char *origin;
	char *channel;
	char *notice;
};

struct irc_event_part {
	char *origin;
	char *channel;
	char *reason;
};

struct irc_event_topic {
	char *origin;
	char *channel;
	char *topic;
};

struct irc_event_whois {
	char *nickname;
	char *username;
	char *realname;
	char *hostname;
	struct {
		char *name;
		int modes;
	} *channels;
	size_t channelsz;
};

struct irc_event {
	enum irc_event_type type;
	struct irc_server *server;
	union {
		struct irc_event_invite invite;
		struct irc_event_join join;
		struct irc_event_kick kick;
		struct irc_event_message message;
		struct irc_event_mode mode;
		struct irc_event_names names;
		struct irc_event_nick nick;
		struct irc_event_notice notice;
		struct irc_event_part part;
		struct irc_event_topic topic;
		struct irc_event_whois whois;
	};
};

int
irc_event_str(const struct irc_event *, char *, size_t);

void
irc_event_finish(struct irc_event *);

#if defined(__cplusplus)
}
#endif

#endif /* !IRCCD_EVENT_H */
