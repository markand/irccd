/*
 * test-bot.c -- test bot.h functions
 *
 * Copyright (c) 2013-2025 David Demelier <markand@malikania.fr>
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

#include <string.h>

#include <unity.h>

#include <irccd/irccd.h>
#include <irccd/server.h>
#include <irccd/util.h>

void
setUp(void)
{
}

void
tearDown(void)
{
	irc_bot_server_clear();
}

static inline struct irc_server *
server_new(const char *name)
{
	struct irc_server *s;

	s = irc_server_new(name);
	irc_server_set_ident(s, "test", "test", "test");
	irc_server_set_params(s, "127.0.0.1", 6667, 0);

	return s;
}

static void
servers_add(void)
{
	struct irc_server *s, *s1, *s2, *s3;

	s1 = server_new("malikania");
	s2 = server_new("freenode");
	s3 = server_new("oftc");

	/* irc.servers -> s1 */
	irc_bot_server_add(s1);
	s = irccd->servers;
	TEST_ASSERT_EQUAL(1, s->refc);
	TEST_ASSERT_EQUAL(s, s1);
	s = s->next;
	TEST_ASSERT(!s);

	/* irc.servers -> s1 -> s2 */
	irc_bot_server_add(s2);
	s = irccd->servers;
	TEST_ASSERT_EQUAL(1, s->refc);
	TEST_ASSERT_EQUAL(s, s1);
	s = s->next;
	TEST_ASSERT_EQUAL(s, s2);
	s = s->next;
	TEST_ASSERT(!s);

	/* irc.servers -> s1 -> s2 -> s3 */
	irc_bot_server_add(s3);
	s = irccd->servers;
	TEST_ASSERT_EQUAL(1, s->refc);
	TEST_ASSERT_EQUAL(s, s1);
	s = s->next;
	TEST_ASSERT_EQUAL(s, s2);
	s = s->next;
	TEST_ASSERT_EQUAL(s, s3);
	s = s->next;
	TEST_ASSERT(!s);
}

static void
servers_remove(void)
{
	struct irc_server *s, *s1, *s2, *s3;

	s1 = server_new("1");
	s2 = server_new("2");
	s3 = server_new("3");

	/* Protect deletion from irc_bot_remove_server. */
	irc_server_incref(s1);
	irc_server_incref(s2);
	irc_server_incref(s3);

	/* irc.servers -> s1 -> s2 -> s3 */
	irc_bot_server_add(s1);
	irc_bot_server_add(s2);
	irc_bot_server_add(s3);

	/* irc.servers -> s3 -> [s2] -> s1 */
	/* irc.servers -> s3 -> s1 */
	irc_bot_server_remove(s2->name);
	TEST_ASSERT_EQUAL(2, s1->refc);
	TEST_ASSERT_EQUAL(1, s2->refc);
	TEST_ASSERT_EQUAL(2, s3->refc);
	s = irccd->servers;
	TEST_ASSERT_EQUAL(s, s1);
	s = s->next;
	TEST_ASSERT_EQUAL(s, s3);
	s = s->next;
	TEST_ASSERT(!s);

	/* irc.servers -> s3 -> [s1] */
	/* irc.servers -> s3 */
	irc_bot_server_remove(s1->name);
	TEST_ASSERT_EQUAL(1, s1->refc);
	TEST_ASSERT_EQUAL(1, s2->refc);
	TEST_ASSERT_EQUAL(2, s3->refc);
	s = irccd->servers;
	TEST_ASSERT_EQUAL(s, s3);
	s = s->next;
	TEST_ASSERT(!s);

	/* irc.servers -> [s3] */
	/* irc.servers -> NULL */
	irc_bot_server_remove(s3->name);
	TEST_ASSERT_EQUAL(1, s1->refc);
	TEST_ASSERT_EQUAL(1, s2->refc);
	TEST_ASSERT_EQUAL(1, s3->refc);
	s = irccd->servers;
	TEST_ASSERT(!s);

	irc_server_decref(s1);
	irc_server_decref(s2);
	irc_server_decref(s3);
}

static void
servers_clear(void)
{
	struct irc_server *s1, *s2, *s3;

	s1 = server_new("1");
	s2 = server_new("2");
	s3 = server_new("3");

	/* Protect deletion from irc_bot_remove_server. */
	irc_server_incref(s1);
	irc_server_incref(s2);
	irc_server_incref(s3);

	irc_bot_server_add(s1);
	irc_bot_server_add(s2);
	irc_bot_server_add(s3);
	irc_bot_server_clear();

	TEST_ASSERT_EQUAL(1, s1->refc);
	TEST_ASSERT_EQUAL(1, s2->refc);
	TEST_ASSERT_EQUAL(1, s3->refc);
	TEST_ASSERT(!irccd->servers);
}

int
main(void)
{
	irc_bot_init(NULL);

	UNITY_BEGIN();

	RUN_TEST(servers_add);
	RUN_TEST(servers_remove);
	RUN_TEST(servers_clear);

	return UNITY_END();
}
