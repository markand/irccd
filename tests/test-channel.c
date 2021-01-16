/*
 * test-channel.c -- test util.h functions
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

#include <irccd/channel.h>

GREATEST_TEST
basics_add(void)
{
	struct irc_channel ch = {0};

	irc_channel_add(&ch, "markand", '@');
	GREATEST_ASSERT_EQ(ch.usersz, 1U);
	GREATEST_ASSERT_EQ(ch.users[0].mode, '@');
	GREATEST_ASSERT_STR_EQ(ch.users[0].nickname, "markand");

	irc_channel_add(&ch, "markand", '@');
	GREATEST_ASSERT_EQ(ch.usersz, 1U);
	GREATEST_ASSERT_EQ(ch.users[0].mode, '@');
	GREATEST_ASSERT_STR_EQ(ch.users[0].nickname, "markand");

	irc_channel_add(&ch, "jean", 0);
	GREATEST_ASSERT_EQ(ch.usersz, 2U);
	GREATEST_ASSERT_EQ(ch.users[0].mode, 0);
	GREATEST_ASSERT_STR_EQ(ch.users[0].nickname, "jean");
	GREATEST_ASSERT_EQ(ch.users[1].mode, '@');
	GREATEST_ASSERT_STR_EQ(ch.users[1].nickname, "markand");

	irc_channel_add(&ch, "zoe", 0);
	GREATEST_ASSERT_EQ(ch.usersz, 3U);
	GREATEST_ASSERT_EQ(ch.users[0].mode, 0);
	GREATEST_ASSERT_STR_EQ(ch.users[0].nickname, "jean");
	GREATEST_ASSERT_EQ(ch.users[1].mode, '@');
	GREATEST_ASSERT_STR_EQ(ch.users[1].nickname, "markand");
	GREATEST_ASSERT_EQ(ch.users[2].mode, 0);
	GREATEST_ASSERT_STR_EQ(ch.users[2].nickname, "zoe");

	GREATEST_PASS();
}

GREATEST_TEST
basics_remove(void)
{
	struct irc_channel ch = {0};

	irc_channel_add(&ch, "markand", '@');
	irc_channel_add(&ch, "jean", 0);
	irc_channel_add(&ch, "zoe", 0);

	irc_channel_remove(&ch, "jean");
	GREATEST_ASSERT_EQ(ch.usersz, 2U);
	GREATEST_ASSERT_EQ(ch.users[0].mode, '@');
	GREATEST_ASSERT_STR_EQ(ch.users[0].nickname, "markand");
	GREATEST_ASSERT_EQ(ch.users[1].mode, 0);
	GREATEST_ASSERT_STR_EQ(ch.users[1].nickname, "zoe");

	irc_channel_remove(&ch, "zoe");
	GREATEST_ASSERT_EQ(ch.usersz, 1U);
	GREATEST_ASSERT_EQ(ch.users[0].mode, '@');
	GREATEST_ASSERT_STR_EQ(ch.users[0].nickname, "markand");

	irc_channel_remove(&ch, "markand");
	GREATEST_ASSERT_EQ(ch.usersz, 0U);
	GREATEST_ASSERT(!ch.users);

	GREATEST_PASS();
}

GREATEST_TEST
basics_set_mode(void)
{
	struct irc_channel ch = {0};

	irc_channel_add(&ch, "jean", '@');
	irc_channel_set_user_mode(&ch, "jean", '+');
	irc_channel_set_user_mode(&ch, "nobody", '+');

	GREATEST_ASSERT_EQ(ch.usersz, 1);
	GREATEST_ASSERT_EQ(ch.users[0].mode, '+');
	GREATEST_ASSERT_STR_EQ(ch.users[0].nickname, "jean");
	GREATEST_PASS();
}

GREATEST_TEST
basics_set_nick(void)
{
	struct irc_channel ch = {0};

	irc_channel_add(&ch, "jean", '@');
	irc_channel_set_user_nick(&ch, "jean", "francis");
	irc_channel_set_user_nick(&ch, "nobody", "francis");

	GREATEST_ASSERT_EQ(ch.usersz, 1);
	GREATEST_ASSERT_EQ(ch.users[0].mode, '@');
	GREATEST_ASSERT_STR_EQ(ch.users[0].nickname, "francis");
	GREATEST_PASS();
}

GREATEST_SUITE(suite_basics)
{
	GREATEST_RUN_TEST(basics_add);
	GREATEST_RUN_TEST(basics_remove);
	GREATEST_RUN_TEST(basics_set_mode);
	GREATEST_RUN_TEST(basics_set_nick);
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
