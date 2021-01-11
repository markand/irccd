/*
 * main.c -- irccd(1) main file
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

#include <poll.h>
#include <stdio.h>
#include <err.h>

#include "event.h"
#include "server.h"

int
main(int argc, char **argv)
{
	struct irc_server s = {
		.name = "malikania",
		.host = "malikania.fr",
		.port = 6667,
		.nickname = "circ",
		.username = "circ",
		.realname = "circ"
	};
	struct irc_event ev;

	struct pollfd fd;

	irc_server_connect(&s);
	irc_server_join(&s, "#test", NULL);

	for (;;) {
		irc_server_prepare(&s, &fd);

		if (poll(&fd, 1, -1) < 0)
			err(1, "poll");

		irc_server_flush(&s, &fd);

		while (irc_server_poll(&s, &ev)) {
			switch (ev.type) {
			case IRC_EVENT_MESSAGE:
				printf("message, origin=%s,channel=%s,message=%s\n",
				    ev.message.origin,ev.message.channel, ev.message.message);
				break;
			case IRC_EVENT_ME:
				printf("me, origin=%s,channel=%s,message=%s\n",
				    ev.me.origin,ev.me.channel, ev.me.message);
				break;
			}
		}
	}
}
