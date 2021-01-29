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
#include <irccd/log.h>
#include <irccd/server.h>
#include <irccd/transport.h>
#include <irccd/util.h>
#include <irccd/plugin.h>
#include <irccd/rule.h>

#include "dl-plugin.h"
#include "js-plugin.h"

static int
run(int argc, char **argv)
{
	(void)argc;

	if (strcmp(argv[0], "version") == 0)
		puts(IRCCD_VERSION);

	return 0;
}

int
main(int argc, char **argv)
{
	--argc;
	++argv;

	if (argc > 0)
		return run(argc, argv);

	irc_bot_init();

	/* TODO: temp. */
	irc_log_set_verbose(true);

	irc_bot_plugin_loader_add(dl_plugin_loader_new());
	irc_bot_plugin_loader_add(js_plugin_loader_new());

	irc_bot_plugin_find("foo");

}
