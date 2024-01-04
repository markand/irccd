/*
 * test-bot.c -- test bot.h functions
 *
 * Copyright (c) 2013-2024 David Demelier <markand@malikania.fr>
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

#define GREATEST_USE_ABBREVS 0
#include <greatest.h>

#include <irccd/irccd.h>
#include <irccd/server.h>
#include <irccd/util.h>

static void
clean(void *udata)
{
	(void)udata;

	irc_bot_server_clear();
}

GREATEST_TEST
servers_add(void)
{
	struct irc_server *s, *s1, *s2, *s3;

	s1 = irc_server_new("malikania", "x", "x", "x", "localhost", 6667);
	s2 = irc_server_new("freenode", "x", "x", "x", "localhost", 6667);
	s3 = irc_server_new("oftc", "x", "x", "x", "localhost", 6667);

	/* irc.servers -> s1 */
	irc_bot_server_add(s1);
	s = irc.servers;
	GREATEST_ASSERT_EQ(1, s->refc);
	GREATEST_ASSERT_EQ(s, s1);
	s = s->next;
	GREATEST_ASSERT(!s);

	/* irc.servers -> s1 -> s2 */
	irc_bot_server_add(s2);
	s = irc.servers;
	GREATEST_ASSERT_EQ(1, s->refc);
	GREATEST_ASSERT_EQ(s, s1);
	s = s->next;
	GREATEST_ASSERT_EQ(s, s2);
	s = s->next;
	GREATEST_ASSERT(!s);

	/* irc.servers -> s1 -> s2 -> s3 */
	irc_bot_server_add(s3);
	s = irc.servers;
	GREATEST_ASSERT_EQ(1, s->refc);
	GREATEST_ASSERT_EQ(s, s1);
	s = s->next;
	GREATEST_ASSERT_EQ(s, s2);
	s = s->next;
	GREATEST_ASSERT_EQ(s, s3);
	s = s->next;
	GREATEST_ASSERT(!s);

	GREATEST_PASS();
}

GREATEST_TEST
servers_remove(void)
{
	struct irc_server *s, *s1, *s2, *s3;

	s1 = irc_server_new("1", "x", "x", "x", "localhost", 6667);
	s2 = irc_server_new("2", "x", "x", "x", "localhost", 6667);
	s3 = irc_server_new("3", "x", "x", "x", "localhost", 6667);

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
	GREATEST_ASSERT_EQ(2, s1->refc);
	GREATEST_ASSERT_EQ(1, s2->refc);
	GREATEST_ASSERT_EQ(2, s3->refc);
	s = irc.servers;
	GREATEST_ASSERT_EQ(s, s1);
	s = s->next;
	GREATEST_ASSERT_EQ(s, s3);
	s = s->next;
	GREATEST_ASSERT(!s);

	/* irc.servers -> s3 -> [s1] */
	/* irc.servers -> s3 */
	irc_bot_server_remove(s1->name);
	GREATEST_ASSERT_EQ(1, s1->refc);
	GREATEST_ASSERT_EQ(1, s2->refc);
	GREATEST_ASSERT_EQ(2, s3->refc);
	s = irc.servers;
	GREATEST_ASSERT_EQ(s, s3);
	s = s->next;
	GREATEST_ASSERT(!s);

	/* irc.servers -> [s3] */
	/* irc.servers -> NULL */
	irc_bot_server_remove(s3->name);
	GREATEST_ASSERT_EQ(1, s1->refc);
	GREATEST_ASSERT_EQ(1, s2->refc);
	GREATEST_ASSERT_EQ(1, s3->refc);
	s = irc.servers;
	GREATEST_ASSERT(!s);

	irc_server_decref(s1);
	irc_server_decref(s2);
	irc_server_decref(s3);

	GREATEST_PASS();
}

GREATEST_TEST
servers_clear(void)
{
	struct irc_server *s1, *s2, *s3;

	s1 = irc_server_new("1", "x", "x", "x", "localhost", 6667);
	s2 = irc_server_new("2", "x", "x", "x", "localhost", 6667);
	s3 = irc_server_new("3", "x", "x", "x", "localhost", 6667);

	/* Protect deletion from irc_bot_remove_server. */
	irc_server_incref(s1);
	irc_server_incref(s2);
	irc_server_incref(s3);

	irc_bot_server_add(s1);
	irc_bot_server_add(s2);
	irc_bot_server_add(s3);
	irc_bot_server_clear();

	GREATEST_ASSERT_EQ(1, s1->refc);
	GREATEST_ASSERT_EQ(1, s2->refc);
	GREATEST_ASSERT_EQ(1, s3->refc);
	GREATEST_ASSERT(!irc.servers);
	GREATEST_PASS();
}

GREATEST_SUITE(suite_servers)
{
	GREATEST_SET_SETUP_CB(clean, NULL);
	GREATEST_SET_TEARDOWN_CB(clean, NULL);
	GREATEST_RUN_TEST(servers_add);
	GREATEST_RUN_TEST(servers_remove);
	GREATEST_RUN_TEST(servers_clear);
}

GREATEST_MAIN_DEFS();

int
main(int argc, char **argv)
{
	GREATEST_MAIN_BEGIN();
	GREATEST_RUN_SUITE(suite_servers);
	GREATEST_MAIN_END();

	return 0;
}
