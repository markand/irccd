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

#include <stdio.h>
#include <err.h>

#include <irccd/event.h>
#include <irccd/irccd.h>
#include <irccd/js-plugin.h>
#include <irccd/plugin.h>
#include <irccd/log.h>
#include <irccd/server.h>
#include <irccd/transport.h>

#include <irccd/rule.h>

static struct irc_plugin js = {
	.name = "example"
};

#include <string.h>

static void
dump(void)
{
	for (size_t i = 0; i < irc.rulesz; ++i) {
		printf("== rule %zd ==\n", i);
		printf("servers => %s\n", irc.rules[i].servers);
	}
}

int
main(int argc, char **argv)
{
	struct irc_rule r = {0};

	irc_rule_add(r.servers, "malikania");
	//irc_rule_add(r.servers, "freenode");

	printf("%d\n", irc_rule_match(&r, "malikania", "", "", "", ""));

#if 0
	irc_rule_add(r.servers, "malikania");
	irc_bot_insert_rule(&r, 0);
	strcpy(r.servers, "freenode:");
	irc_bot_insert_rule(&r, 15);
	strcpy(r.servers, "oftc:");
	irc_bot_insert_rule(&r, 0);
	strcpy(r.servers, "jean:");
	irc_bot_insert_rule(&r, 1);

	puts("BEFORE");
	dump();
	irc_bot_remove_rule(3);
	puts("AFTER");
	dump();
#endif


}
