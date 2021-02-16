/*
 * test-event.c -- test event.h functions
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

#define GREATEST_USE_ABBREVS 0
#include <greatest.h>

#include <irccd/conn.h>
#include <irccd/event.h>

GREATEST_TEST
basics_parse_simple(void)
{
	/* This is a TOPIC message. */
	struct irc_conn conn = {
		.in = ":malikania.fr 332 boris #test :Welcome to #test :: a testing channel\r\n"
	};
	struct irc_conn_msg msg = {0};

	irc_conn_poll(&conn, &msg);

	GREATEST_ASSERT_STR_EQ("malikania.fr", msg.prefix);
	GREATEST_ASSERT_STR_EQ("332", msg.cmd);
	GREATEST_ASSERT_STR_EQ("boris", msg.args[0]);
	GREATEST_ASSERT_STR_EQ("#test", msg.args[1]);
	GREATEST_ASSERT_STR_EQ("Welcome to #test :: a testing channel", msg.args[2]);

	GREATEST_PASS();
}

GREATEST_TEST
basics_parse_noprefix(void)
{
	/* Ping messages usually don't have a prefix. */
	struct irc_conn conn = {
		.in = "PING :malikania.fr\r\n"
	};
	struct irc_conn_msg msg = {0};

	irc_conn_poll(&conn, &msg);

	GREATEST_ASSERT(!msg.prefix);
	GREATEST_ASSERT_STR_EQ("PING", msg.cmd);
	GREATEST_ASSERT_STR_EQ("malikania.fr", msg.args[0]);

	GREATEST_PASS();
}

GREATEST_SUITE(suite_basics)
{
	GREATEST_RUN_TEST(basics_parse_simple);
	GREATEST_RUN_TEST(basics_parse_noprefix);
}

GREATEST_MAIN_DEFS();

int
main(int argc, char **argv)
{
	GREATEST_MAIN_BEGIN();
	GREATEST_RUN_SUITE(suite_basics);
	GREATEST_MAIN_END();

	return 0;
}
