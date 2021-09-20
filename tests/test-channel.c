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
	struct irc_channel *ch;
	struct irc_channel_user *user;

	ch = irc_channel_new("#test", NULL, 1);
	GREATEST_ASSERT_STR_EQ("#test", ch->name);
	GREATEST_ASSERT_STR_EQ("", ch->password);
	GREATEST_ASSERT(ch->joined);

	irc_channel_add(ch, "markand", 1);
	user = ch->users;
	GREATEST_ASSERT_EQ(1, user->modes);
	GREATEST_ASSERT_STR_EQ("markand", user->nickname);

	irc_channel_add(ch, "markand", 2);
	user = ch->users;
	GREATEST_ASSERT_EQ(1, user->modes);
	GREATEST_ASSERT_STR_EQ("markand", user->nickname);

	irc_channel_add(ch, "jean", 4);
	user = ch->users;
	GREATEST_ASSERT_EQ(4, user->modes);
	GREATEST_ASSERT_STR_EQ("jean", user->nickname);
	user = user->next;
	GREATEST_ASSERT_EQ(1, user->modes);
	GREATEST_ASSERT_STR_EQ("markand", user->nickname);

	irc_channel_add(ch, "zoe", 0);
	user = ch->users;
	GREATEST_ASSERT_EQ(0, user->modes);
	GREATEST_ASSERT_STR_EQ("zoe", user->nickname);
	user = user->next;
	GREATEST_ASSERT_EQ(4, user->modes);
	GREATEST_ASSERT_STR_EQ("jean", user->nickname);
	user = user->next;
	GREATEST_ASSERT_EQ(1, user->modes);
	GREATEST_ASSERT_STR_EQ("markand", user->nickname);

	irc_channel_finish(ch);

	GREATEST_PASS();
}

GREATEST_TEST
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
	GREATEST_ASSERT_EQ(0, user->modes);
	GREATEST_ASSERT_STR_EQ("zoe", user->nickname);
	user = user->next;
	GREATEST_ASSERT_EQ(1, user->modes);
	GREATEST_ASSERT_STR_EQ("markand", user->nickname);

	irc_channel_remove(ch, "zoe");
	user = ch->users;
	GREATEST_ASSERT_EQ(1, user->modes);
	GREATEST_ASSERT_STR_EQ("markand", user->nickname);

	/* Also test case sensitivity. */
	irc_channel_remove(ch, "MaRKaND");
	GREATEST_ASSERT(!ch->users);

	irc_channel_finish(ch);

	GREATEST_PASS();
}

GREATEST_SUITE(suite_basics)
{
	GREATEST_RUN_TEST(basics_add);
	GREATEST_RUN_TEST(basics_remove);
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
