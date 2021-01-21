/*
 * event.c -- IRC event
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

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "event.h"
#include "server.h"

static inline void
scan(char **line, char **str)
{
	char *p = strchr(*line, ' ');

	if (p)
		*p = '\0';

	*str = *line;
	*line = p ? p + 1 : strchr(*line, '\0');
}

bool
irc_event_is_ctcp(const char *line)
{
	size_t length;

	if (!line)
		return false;
	if ((length = strlen(line)) < 2)
		return false;

	return line[0] == 0x1 && line[length - 1] == 0x1;
}

char *
irc_event_ctcp(char *line)
{
	/* Skip first \001. */
	if (*line == '\001')
		line++;

	/* Remove last \001. */
	line[strcspn(line, "\001")] = '\0';

	if (strncmp(line, "ACTION ", 7) == 0)
		line += 7;

	return line;
}

bool
irc_event_parse(struct irc_event_msg *ev, const char *line)
{
	assert(ev);
	assert(line);

	char *ptr = ev->buf;
	size_t a;

	memset(ev, 0, sizeof (*ev));
	strlcpy(ev->buf, line, sizeof (ev->buf));

	/*
	 * IRC message is defined as following:
	 *
	 * [:prefix] command arg1 arg2 [:last-argument]
	 */
	if (*line == ':')
		scan((++ptr, &ptr), &ev->prefix);     /* prefix */

	scan(&ptr, &ev->cmd);                         /* command */

	/* And finally arguments. */
	for (a = 0; *ptr && a < IRC_ARGS_MAX; ++a) {
		if (*ptr == ':') {
			ev->args[a] = ptr + 1;
			ptr = strchr(ptr, '\0');
		} else
			scan(&ptr, &ev->args[a]);
	}

	if (a >= IRC_ARGS_MAX)
		return errno = EMSGSIZE, false;
	if (ev->cmd == NULL)
		return errno = EBADMSG, false;

	return true;
}

bool
irc_event_str(const struct irc_event *ev, char *str, size_t strsz)
{
	assert(ev);
	assert(str);

	int written = 0;

	switch (ev->type) {
	case IRC_EVENT_CONNECT:
		written = snprintf(str, strsz, "EVENT-CONNECT %s", ev->server->name);
		break;
	case IRC_EVENT_DISCONNECT:
		written = snprintf(str, strsz, "EVENT-DISCONNECT %s", ev->server->name);
		break;
	case IRC_EVENT_INVITE:
		written = snprintf(str, strsz, "EVENT-INVITE %s %s %s %s", ev->server->name,
		    ev->msg.prefix, ev->msg.args[0], ev->msg.args[1]);
		break;
	case IRC_EVENT_JOIN:
		written = snprintf(str, strsz, "EVENT-JOIN %s %s %s", ev->server->name,
		    ev->msg.prefix, ev->msg.args[0]);
		break;
	case IRC_EVENT_KICK:
		written = snprintf(str, strsz, "EVENT-KICK %s %s %s %s %s", ev->server->name,
		    ev->msg.prefix, ev->msg.args[0], ev->msg.args[1],
		    ev->msg.args[2] ? ev->msg.args[2] : "");
		break;
	case IRC_EVENT_ME:
		written = snprintf(str, strsz, "EVENT-ME %s %s %s %s", ev->server->name,
		    ev->msg.prefix, ev->msg.args[0], ev->msg.args[1]);
		break;
	case IRC_EVENT_MESSAGE:
		written = snprintf(str, strsz, "EVENT-MESSAGE %s %s %s %s", ev->server->name,
		    ev->msg.prefix, ev->msg.args[0], ev->msg.args[1]);
		break;
	case IRC_EVENT_MODE:
		written = snprintf(str, strsz, "EVENT-MODE %s %s %s %s %s %s %s",
		    ev->server->name, ev->msg.prefix, ev->msg.args[0],
		    ev->msg.args[1] ? ev->msg.args[1] : "",
		    ev->msg.args[2] ? ev->msg.args[2] : "",
		    ev->msg.args[3] ? ev->msg.args[3] : "",
		    ev->msg.args[4] ? ev->msg.args[4] : "");
		break;
	case IRC_EVENT_NICK:
		written = snprintf(str, strsz, "EVENT-NICK %s %s %s", ev->server->name,
		    ev->msg.prefix, ev->msg.args[0]);
		break;
	case IRC_EVENT_NOTICE:
		written = snprintf(str, strsz, "EVENT-NOTICE %s %s %s %s", ev->server->name,
		    ev->msg.prefix, ev->msg.args[0], ev->msg.args[1]);
		break;
	case IRC_EVENT_PART:
		written = snprintf(str, strsz, "EVENT-PART %s %s %s %s", ev->server->name,
		    ev->msg.prefix, ev->msg.args[0],
		    ev->msg.args[1] ? ev->msg.args[1] : "");
		break;
	case IRC_EVENT_TOPIC:
		written = snprintf(str, strsz, "EVENT-TOPIC %s %s %s %s", ev->server->name,
		    ev->msg.prefix, ev->msg.args[0], ev->msg.args[1]);
		break;
	case IRC_EVENT_WHOIS:
		snprintf(str, strsz, "EVENT-WHOIS %s %s %s %s %s ",
		    ev->server->name, ev->whois->nickname, ev->whois->username,
		    ev->whois->realname, ev->whois->hostname);

		for (size_t i = 0; i < ev->whois->channelsz; ++i) {
			size_t s;

			/* Concat channel and their modes. */
			strlcat(str, (char []) {ev->whois->channels[i].mode, '\0' }, strsz);
			strlcat(str, ev->whois->channels[i].channel, strsz);

			if ((s = strlcat(str, " ", strsz)) >= strsz)
				goto emsgsize;

			written = s;
		}
		break;
	default:
		break;
	}

	return written > 0;

emsgsize:
	errno = EMSGSIZE;
	return false;
}
