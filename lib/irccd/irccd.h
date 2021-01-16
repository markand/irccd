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

#include <stdbool.h>
#include <stddef.h>

#include "rule.h"

#define IRC_BOT_RULE_MAX 256

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
	struct irc_rule rules[IRC_BOT_RULE_MAX];
	size_t rulesz;
} irc;

void
irc_bot_init(void);

void
irc_bot_add_server(struct irc_server *);

struct irc_server *
irc_bot_find_server(const char *);

void
irc_bot_remove_server(const char *);

void
irc_bot_clear_servers(void);

void
irc_bot_add_plugin(const struct irc_plugin *);

struct irc_plugin *
irc_bot_find_plugin(const char *);

void
irc_bot_remove_plugin(const char *);

bool
irc_bot_insert_rule(const struct irc_rule *, size_t);

void
irc_bot_remove_rule(size_t);

void
irc_bot_post(void (*)(void *), void *);

void
irc_bot_run(void);

#endif /* !IRCCD_H */