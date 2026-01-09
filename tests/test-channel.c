/*
 * test-channel.c -- test util.h functions
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

#include <irccd/channel.h>

void
setUp(void)
{
}

void
tearDown(void)
{
}

static void
basics_add(void)
{
	struct irc_channel *ch;
	struct irc_channel_user *user;

	ch = irc_channel_new("#test", NULL, IRC_CHANNEL_FLAGS_JOINED);
	TEST_ASSERT_EQUAL_STRING("#test", ch->name);
	TEST_ASSERT(!ch->password);
	TEST_ASSERT(ch->flags & IRC_CHANNEL_FLAGS_JOINED);

	irc_channel_add(ch, "markand", 1);
	user = ch->users;
	TEST_ASSERT_EQUAL(1, user->modes);
	TEST_ASSERT_EQUAL_STRING("markand", user->nickname);

	irc_channel_add(ch, "markand", 2);
	user = ch->users;
	TEST_ASSERT_EQUAL(1, user->modes);
	TEST_ASSERT_EQUAL_STRING("markand", user->nickname);

	irc_channel_add(ch, "jean", 4);
	user = ch->users;
	TEST_ASSERT_EQUAL(4, user->modes);
	TEST_ASSERT_EQUAL_STRING("jean", user->nickname);
	user = user->next;
	TEST_ASSERT_EQUAL(1, user->modes);
	TEST_ASSERT_EQUAL_STRING("markand", user->nickname);

	irc_channel_add(ch, "zoe", 0);
	user = ch->users;
	TEST_ASSERT_EQUAL(0, user->modes);
	TEST_ASSERT_EQUAL_STRING("zoe", user->nickname);
	user = user->next;
	TEST_ASSERT_EQUAL(4, user->modes);
	TEST_ASSERT_EQUAL_STRING("jean", user->nickname);
	user = user->next;
	TEST_ASSERT_EQUAL(1, user->modes);
	TEST_ASSERT_EQUAL_STRING("markand", user->nickname);

	irc_channel_free(ch);
}

static void
basics_remove(void)
{
	struct irc_channel *ch;
	struct irc_channel_user *user;

	ch = irc_channel_new("#test", NULL, 1);

	irc_channel_add(ch, "markand", 1);
	irc_channel_add(ch, "jean", 0);
	irc_channel_add(ch, "zoe", 0);

	irc_channel_remove(ch, "jean");
	user = ch->users;
	TEST_ASSERT_EQUAL(0, user->modes);
	TEST_ASSERT_EQUAL_STRING("zoe", user->nickname);
	user = user->next;
	TEST_ASSERT_EQUAL(1, user->modes);
	TEST_ASSERT_EQUAL_STRING("markand", user->nickname);

	irc_channel_remove(ch, "zoe");
	user = ch->users;
	TEST_ASSERT_EQUAL(1, user->modes);
	TEST_ASSERT_EQUAL_STRING("markand", user->nickname);

	/* Also test case sensitivity. */
	irc_channel_remove(ch, "MaRKaND");
	TEST_ASSERT(!ch->users);

	irc_channel_free(ch);
}

int
main(void)
{
	UNITY_BEGIN();

	RUN_TEST(basics_add);
	RUN_TEST(basics_remove);

	return UNITY_END();
}
