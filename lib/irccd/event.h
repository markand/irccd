/*
 * event.h -- IRC event
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

#ifndef IRCCD_EVENT_H
#define IRCCD_EVENT_H

#include <stdbool.h>
#include <stddef.h>

#include "limits.h"

struct irc_server;

enum irc_event_type {
	IRC_EVENT_UNKNOWN,
	IRC_EVENT_COMMAND,
	IRC_EVENT_CONNECT,
	IRC_EVENT_DISCONNECT,
	IRC_EVENT_INVITE,
	IRC_EVENT_JOIN,
	IRC_EVENT_KICK,
	IRC_EVENT_ME,
	IRC_EVENT_MESSAGE,
	IRC_EVENT_MODE,
	IRC_EVENT_NAMES,
	IRC_EVENT_NICK,
	IRC_EVENT_NOTICE,
	IRC_EVENT_PART,
	IRC_EVENT_TOPIC,
	IRC_EVENT_WHOIS
};

struct irc_event_msg {
	char *prefix;
	char *cmd;
	char *args[IRC_ARGS_MAX];
	char buf[IRC_MESSAGE_MAX];
};

struct irc_event {
	enum irc_event_type type;
	struct irc_server *server;
	struct irc_event_msg msg;
};

bool
irc_event_is_ctcp(const char *);

char *
irc_event_ctcp(char *);

bool
irc_event_parse(struct irc_event_msg *, const char *);

bool
irc_event_str(const struct irc_event *, char *, size_t);

#endif /* !IRCCD_EVENT_H */
