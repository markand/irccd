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

#include <stddef.h>

#include "limits.h"

struct irc_server;

enum irc_event_type {
	IRC_EVENT_UNKNOWN,
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

struct irc_event {
	enum irc_event_type type;
	struct irc_server *server;

	/*
	 * Raw arguments.
	 *   [0]: prefix
	 */
	char args[IRC_ARGS_MAX][IRC_MESSAGE_MAX];
	size_t argsz;

	/* Conveniently organized union depending on the type. */
	union {
		struct {
			char *origin;
			char *channel;
			char *nickname;
		} invite;

		struct {
			char *origin;
			char *channel;
		} join;

		struct {
			char *origin;
			char *channel;
			char *target;
			char *reason;
		} kick;

		struct {
			char *origin;
			char *channel;
			char *message;
		} me;

		struct {
			char *origin;
			char *channel;
			char *message;
		} message;

		struct {
			char *origin;
			char *channel;
			char *mode;
			char *limit;
			char *user;
			char *mask;
		} mode;

		struct {
			char *origin;
			char *nickname;
		} nick;

		struct {
			char *origin;
			char *channel;
			char *message;
		} notice;

		struct {
			char *origin;
			char *channel;
			char *reason;
		} part;

		struct {
			char *origin;
			char *channel;
			char *topic;
		} topic;

		struct {
			char *nick;
			char *user;
			char *hostname;
			char *realname;
			char **channels;
		} whois;
	};
};

#endif /* !IRCCD_EVENT_H */
