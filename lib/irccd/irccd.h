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

#include "peer.h"
#include "plugin.h"
#include "rule.h"
#include "server.h"

extern struct irc {
	struct irc_server_list servers;
	struct irc_peer_list peers;
	struct irc_plugin_list plugins;
	struct irc_plugin_loader_list plugin_loaders;
	struct irc_rule_list rules;
} irc;

void
irc_bot_init(void);

void
irc_bot_server_add(struct irc_server *);

struct irc_server *
irc_bot_server_find(const char *);

void
irc_bot_server_remove(const char *);

void
irc_bot_server_clear(void);

void
irc_bot_plugin_add(struct irc_plugin *);

void
irc_bot_plugin_find(const char *);

struct irc_plugin *
irc_bot_plugin_get(const char *);

void
irc_bot_plugin_remove(const char *);

void
irc_bot_plugin_loader_add(struct irc_plugin_loader *);

void
irc_bot_rule_insert(struct irc_rule *, size_t);

void
irc_bot_rule_remove(size_t);

void
irc_bot_rule_clear(void);

void
irc_bot_post(void (*)(void *), void *);

void
irc_bot_run(void);

#endif /* !IRCCD_H */
