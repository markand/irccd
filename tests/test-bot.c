/*
 * test-bot.c -- test bot.h functions
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

#include <string.h>

#define GREATEST_USE_ABBREVS 0
#include <greatest.h>

#include <irccd/irccd.h>
#include <irccd/server.h>
#include <irccd/util.h>

static struct irc_server *
server_new(const char *name)
{
	struct irc_server *s;

	s = irc_util_calloc(1, sizeof (*s));
	strlcpy(s->name, name, sizeof (s->name));

	return s;
}

static void
clean(void *udata)
{
	(void)udata;

	irc_bot_clear_servers();
}

GREATEST_TEST
servers_add(void)
{
	struct irc_server *s1, *s2, *s3;

	s1 = server_new("malikania");
	s2 = server_new("freenode");
	s3 = server_new("oftc");

	/* irc.servers -> s1 */
	irc_bot_add_server(s1);
	GREATEST_ASSERT_EQ(1, irc.serversz);
	GREATEST_ASSERT_EQ(1, s1->refc);
	GREATEST_ASSERT_EQ(s1, irc.servers);
	GREATEST_ASSERT_EQ(NULL, s1->prev);
	GREATEST_ASSERT_EQ(NULL, s1->next);

	/* irc.servers -> s2 -> s1 */
	irc_bot_add_server(s2);
	GREATEST_ASSERT_EQ(2, irc.serversz);
	GREATEST_ASSERT_EQ(1, s1->refc);
	GREATEST_ASSERT_EQ(1, s2->refc);
	GREATEST_ASSERT_EQ(s2, irc.servers);
	GREATEST_ASSERT_EQ(s1, s2->next);
	GREATEST_ASSERT_EQ(NULL, s2->prev);
	GREATEST_ASSERT_EQ(NULL, s1->next);
	GREATEST_ASSERT_EQ(s2, s1->prev);

	/* irc.servers -> s3 -> s2 -> s1 */
	irc_bot_add_server(s3);
	GREATEST_ASSERT_EQ(3, irc.serversz);
	GREATEST_ASSERT_EQ(1, s1->refc);
	GREATEST_ASSERT_EQ(1, s2->refc);
	GREATEST_ASSERT_EQ(1, s3->refc);
	GREATEST_ASSERT_EQ(s3, irc.servers);
	GREATEST_ASSERT_EQ(s2, s3->next);
	GREATEST_ASSERT_EQ(NULL, s3->prev);
	GREATEST_ASSERT_EQ(s1, s2->next);
	GREATEST_ASSERT_EQ(s3, s2->prev);
	GREATEST_ASSERT_EQ(NULL, s1->next);
	GREATEST_ASSERT_EQ(s2, s1->prev);

	GREATEST_PASS();
}

GREATEST_TEST
servers_remove(void)
{
	struct irc_server *s1, *s2, *s3;

	s1 = server_new("1");
	s2 = server_new("2");
	s3 = server_new("3");

	/* Protect deletion from irc_bot_remove_server. */
	irc_server_incref(s1);
	irc_server_incref(s2);
	irc_server_incref(s3);

	/* irc.servers -> s3 -> s2 -> s1 */
	irc_bot_add_server(s1);
	irc_bot_add_server(s2);
	irc_bot_add_server(s3);

	/* irc.servers -> s3 -> [s2] -> s1 */
	/* irc.servers -> s3 -> s1 */
	irc_bot_remove_server(s2->name);
	GREATEST_ASSERT_EQ(2, irc.serversz);
	GREATEST_ASSERT_EQ(2, s1->refc);
	GREATEST_ASSERT_EQ(1, s2->refc);
	GREATEST_ASSERT_EQ(2, s3->refc);
	GREATEST_ASSERT_EQ(NULL, s2->next);
	GREATEST_ASSERT_EQ(NULL, s2->prev);
	GREATEST_ASSERT_EQ(s1, s3->next);
	GREATEST_ASSERT_EQ(NULL, s3->prev);
	GREATEST_ASSERT_EQ(NULL, s1->next);
	GREATEST_ASSERT_EQ(s3, s1->prev);

	/* irc.servers -> s3 -> [s1] */
	/* irc.servers -> s3 */
	irc_bot_remove_server(s1->name);
	GREATEST_ASSERT_EQ(1, irc.serversz);
	GREATEST_ASSERT_EQ(1, s1->refc);
	GREATEST_ASSERT_EQ(1, s2->refc);
	GREATEST_ASSERT_EQ(2, s3->refc);
	GREATEST_ASSERT_EQ(NULL, s1->next);
	GREATEST_ASSERT_EQ(NULL, s1->prev);
	GREATEST_ASSERT_EQ(NULL, s3->next);
	GREATEST_ASSERT_EQ(NULL, s3->prev);

	/* irc.servers -> [s3] */
	/* irc.servers -> NULL */
	irc_bot_remove_server(s3->name);
	GREATEST_ASSERT_EQ(0, irc.serversz);
	GREATEST_ASSERT_EQ(NULL, irc.servers);
	GREATEST_ASSERT_EQ(1, s1->refc);
	GREATEST_ASSERT_EQ(1, s2->refc);
	GREATEST_ASSERT_EQ(1, s3->refc);
	GREATEST_ASSERT_EQ(NULL, s3->next);
	GREATEST_ASSERT_EQ(NULL, s3->prev);

	irc_server_decref(s1);
	irc_server_decref(s2);
	irc_server_decref(s3);

	GREATEST_PASS();
}

GREATEST_TEST
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

	irc_bot_add_server(s1);
	irc_bot_add_server(s2);
	irc_bot_add_server(s3);
	irc_bot_clear_servers();

	GREATEST_ASSERT_EQ(0, irc.serversz);
	GREATEST_ASSERT_EQ(NULL, irc.servers);
	GREATEST_ASSERT_EQ(1, s1->refc);
	GREATEST_ASSERT_EQ(NULL, s1->next);
	GREATEST_ASSERT_EQ(NULL, s1->prev);
	GREATEST_ASSERT_EQ(1, s2->refc);
	GREATEST_ASSERT_EQ(NULL, s2->next);
	GREATEST_ASSERT_EQ(NULL, s2->prev);
	GREATEST_ASSERT_EQ(1, s3->refc);
	GREATEST_ASSERT_EQ(NULL, s3->next);
	GREATEST_ASSERT_EQ(NULL, s3->prev);
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
