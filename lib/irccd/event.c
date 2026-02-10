/*
 * event.c -- IRC event
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

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "event.h"
#include "server.h"
#include "util.h"

int
irc_event_str(const struct irc_event *ev, char *str, size_t strsz)
{
	assert(ev);
	assert(str);

	int written = 0;

	switch (ev->type) {
	case IRC_EVENT_CONNECT:
		written = snprintf(str, strsz, "EVENT-CONNECT %s",
		    ev->server->name);
		break;
	case IRC_EVENT_DISCONNECT:
		written = snprintf(str, strsz, "EVENT-DISCONNECT %s",
		    ev->server->name);
		break;
	case IRC_EVENT_INVITE:
		written = snprintf(str, strsz, "EVENT-INVITE %s %s %s",
		    ev->server->name, ev->invite.origin, ev->invite.channel);
		break;
	case IRC_EVENT_JOIN:
		written = snprintf(str, strsz, "EVENT-JOIN %s %s %s",
		    ev->server->name, ev->join.origin, ev->join.channel);
		break;
	case IRC_EVENT_KICK:
		written = snprintf(str, strsz, "EVENT-KICK %s %s %s %s %s",
		    ev->server->name, ev->kick.origin, ev->kick.channel,
		    ev->kick.target, ev->kick.reason ? ev->kick.reason : "");
		break;
	case IRC_EVENT_ME:
		written = snprintf(str, strsz, "EVENT-ME %s %s %s %s",
		    ev->server->name, ev->message.origin, ev->message.channel,
		    ev->message.message);
		break;
	case IRC_EVENT_MESSAGE:
		written = snprintf(str, strsz, "EVENT-MESSAGE %s %s %s %s",
		    ev->server->name, ev->message.origin, ev->message.channel,
		    ev->message.message);
		break;
	case IRC_EVENT_MODE:
		snprintf(str, strsz, "EVENT-MODE %s %s %s %s ",
		    ev->server->name, ev->mode.origin, ev->mode.channel,
		    ev->mode.mode);

		for (char **mode = ev->mode.args; *mode; ++mode)
			written = irc_util_strlcat(str, *mode, strsz);

		break;
	case IRC_EVENT_NICK:
		written = snprintf(str, strsz, "EVENT-NICK %s %s %s",
		    ev->server->name, ev->nick.origin, ev->nick.nickname);
		break;
	case IRC_EVENT_NOTICE:
		written = snprintf(str, strsz, "EVENT-NOTICE %s %s %s %s",
		    ev->server->name, ev->notice.origin, ev->notice.channel,
		    ev->notice.notice);
		break;
	case IRC_EVENT_PART:
		written = snprintf(str, strsz, "EVENT-PART %s %s %s %s",
		    ev->server->name, ev->part.origin, ev->part.channel,
		    ev->part.reason ? ev->part.reason : "");
		break;
	case IRC_EVENT_TOPIC:
		written = snprintf(str, strsz, "EVENT-TOPIC %s %s %s %s",
		    ev->server->name, ev->topic.origin, ev->topic.channel,
		    ev->topic.topic);
		break;
	case IRC_EVENT_WHOIS:
		snprintf(str, strsz, "EVENT-WHOIS %s %s %s %s %s",
		    ev->server->name, ev->whois.nickname, ev->whois.username,
		    ev->whois.realname, ev->whois.hostname);
		break;
	default:
		break;
	}

	return written <= 0 ? -1 : 0;
}

void
irc_event_finish(struct irc_event *ev)
{
	assert(ev);

	switch (ev->type) {
	case IRC_EVENT_INVITE:
		free(ev->invite.origin);
		free(ev->invite.channel);
		break;
	case IRC_EVENT_JOIN:
		free(ev->join.origin);
		free(ev->join.channel);
		break;
	case IRC_EVENT_KICK:
		free(ev->kick.origin);
		free(ev->kick.channel);
		free(ev->kick.target);
		free(ev->kick.reason);
		break;
	case IRC_EVENT_COMMAND:
	case IRC_EVENT_ME:
	case IRC_EVENT_MESSAGE:
		free(ev->message.origin);
		free(ev->message.channel);
		free(ev->message.message);
		break;
	case IRC_EVENT_MODE:
		free(ev->mode.origin);
		free(ev->mode.channel);
		free(ev->mode.mode);
		for (char **p = ev->mode.args; p && *p; ++p)
			free(*p);
		free(ev->mode.args);
		break;
	case IRC_EVENT_NAMES:
		free(ev->names.channel);
		for (size_t i = 0; i < ev->names.usersz; ++i)
			free(ev->names.users[i].nickname);
		free(ev->names.users);
		break;
	case IRC_EVENT_NICK:
		free(ev->nick.origin);
		free(ev->nick.nickname);
		break;
	case IRC_EVENT_NOTICE:
		free(ev->notice.origin);
		free(ev->notice.channel);
		free(ev->notice.notice);
		break;
	case IRC_EVENT_PART:
		free(ev->part.origin);
		free(ev->part.channel);
		free(ev->part.reason);
		break;
	case IRC_EVENT_TOPIC:
		free(ev->topic.origin);
		free(ev->topic.channel);
		free(ev->topic.topic);
		break;
	case IRC_EVENT_WHOIS:
		free(ev->whois.nickname);
		free(ev->whois.username);
		free(ev->whois.realname);
		free(ev->whois.hostname);
		for (size_t i = 0; i < ev->whois.channelsz; ++i)
			free(ev->whois.channels[i].name);
		free(ev->whois.channels);
		break;
	default:
		break;
	}

	memset(ev, 0, sizeof (*ev));
}
