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

#include <irccd/irccd.h>
#include <irccd/js-plugin.h>
#include <irccd/log.h>
#include <irccd/plugin.h>
#include <irccd/server.h>
#include <irccd/transport.h>
#include <irccd/util.h>

int
main(int argc, char **argv)
{
	struct irc_server s = {
		.name = "malikania",
		.username = "circ",
		.nickname = "circ",
		.hostname = "malikania.fr",
		.port = 6667
	};
	struct irc_plugin p = {
		.name = "fuck"
	};

	irc_log_set_verbose(true);
	irc_bot_init();

	irc_transport_bind("/tmp/irccd.sock");
	//irc_server_join(&s, "#test", NULL);
	irc_js_plugin_open(&p, "/Users/markand/test.js");
	//irc_bot_add_server(irc_util_memdup(&s, sizeof (s)));
	irc_bot_add_plugin(&p);
	irc_bot_run();
}
