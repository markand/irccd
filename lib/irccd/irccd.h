/*
 * irccd.h -- main irccd object
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

#ifndef IRCCD_H
#define IRCCD_H

#include <stddef.h>

struct irc_server;
struct irc_plugin;
struct irc_peer;

extern struct irc {
	struct irc_peer *peers;
	size_t peersz;
	struct irc_plugin *plugins;
	size_t pluginsz;
	struct irc_server *servers;
	size_t serversz;
} irc;

void
irc_init(void);

void
irc_add_server(const struct irc_server *);

struct irc_server *
irc_find_server(const char *);

void
irc_del_server(const char *);

void
irc_add_plugin(const struct irc_plugin *);

struct irc_plugin *
irc_find_plugin(const char *);

void
irc_del_plugin(const char *);

void
irc_post(void (*)(void *), void *);

void
irc_run(void);

#endif /* !IRCCD_H */
