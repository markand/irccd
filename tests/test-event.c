/*
 * test-event.c -- test event.h functions
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

#include <unity.h>

#include <irccd/event.h>

void
setUp(void)
{
}

void
tearDown(void)
{
}

static void
basics_parse_simple(void)
{
#if 0
	/* This is a TOPIC message. */
	struct irc__conn conn = {
		.in = ":malikania.fr 332 boris #test :Welcome to #test :: a testing channel\r\n"
	};
	struct irc__conn_msg msg = {0};

	irc__conn_poll(&conn, &msg);

	TEST_ASSERT_EQUAL_STRING("malikania.fr", msg.prefix);
	TEST_ASSERT_EQUAL_STRING("332", msg.cmd);
	TEST_ASSERT_EQUAL_STRING("boris", msg.args[0]);
	TEST_ASSERT_EQUAL_STRING("#test", msg.args[1]);
	TEST_ASSERT_EQUAL_STRING("Welcome to #test :: a testing channel", msg.args[2]);
#endif
}

static void
basics_parse_noprefix(void)
{
#if 0
	/* Ping messages usually don't have a prefix. */
	struct irc__conn conn = {
		.in = "PING :malikania.fr\r\n"
	};
	struct irc__conn_msg msg = {0};

	irc__conn_poll(&conn, &msg);

	GREATEST_ASSERT(!msg.prefix);
	TEST_ASSERT_EQUAL_STRING("PING", msg.cmd);
	TEST_ASSERT_EQUAL_STRING("malikania.fr", msg.args[0]);

#endif
}

int
main(void)
{
	UNITY_BEGIN();

	RUN_TEST(basics_parse_simple);
	RUN_TEST(basics_parse_noprefix);

	return UNITY_END();
}
