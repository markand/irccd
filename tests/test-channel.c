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

	irc_channel_add(ch, "markand", 'o', '@');
	user = LIST_FIRST(&ch->users);
	GREATEST_ASSERT_EQ('o', user->mode);
	GREATEST_ASSERT_EQ('@', user->symbol);
	GREATEST_ASSERT_STR_EQ("markand", user->nickname);

	irc_channel_add(ch, "markand", '+', '@');
	user = LIST_FIRST(&ch->users);
	GREATEST_ASSERT_EQ('o', user->mode);
	GREATEST_ASSERT_EQ('@', user->symbol);
	GREATEST_ASSERT_STR_EQ("markand", user->nickname);

	irc_channel_add(ch, "jean", 'h', '+');
	user = LIST_FIRST(&ch->users);
	GREATEST_ASSERT_EQ('h', user->mode);
	GREATEST_ASSERT_EQ('+', user->symbol);
	GREATEST_ASSERT_STR_EQ("jean", user->nickname);
	user = LIST_NEXT(user, link);
	GREATEST_ASSERT_EQ('o', user->mode);
	GREATEST_ASSERT_EQ('@', user->symbol);
	GREATEST_ASSERT_STR_EQ("markand", user->nickname);

	irc_channel_add(ch, "zoe", 0, 0);
	user = LIST_FIRST(&ch->users);
	GREATEST_ASSERT_EQ(0, user->mode);
	GREATEST_ASSERT_EQ(0, user->symbol);
	GREATEST_ASSERT_STR_EQ("zoe", user->nickname);
	user = LIST_NEXT(user, link);
	GREATEST_ASSERT_EQ('h', user->mode);
	GREATEST_ASSERT_EQ('+', user->symbol);
	GREATEST_ASSERT_STR_EQ("jean", user->nickname);
	user = LIST_NEXT(user, link);
	GREATEST_ASSERT_EQ('o', user->mode);
	GREATEST_ASSERT_EQ('@', user->symbol);
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

	irc_channel_add(ch, "markand", 'o', '@');
	irc_channel_add(ch, "jean", 0, 0);
	irc_channel_add(ch, "zoe", 0, 0);

	irc_channel_remove(ch, "jean");
	user = LIST_FIRST(&ch->users);
	GREATEST_ASSERT_EQ(0, user->mode);
	GREATEST_ASSERT_EQ(0, user->symbol);
	GREATEST_ASSERT_STR_EQ("zoe", user->nickname);
	user = LIST_NEXT(user, link);
	GREATEST_ASSERT_EQ('o', user->mode);
	GREATEST_ASSERT_EQ('@', user->symbol);
	GREATEST_ASSERT_STR_EQ("markand", user->nickname);

	irc_channel_remove(ch, "zoe");
	user = LIST_FIRST(&ch->users);
	GREATEST_ASSERT_EQ('o', user->mode);
	GREATEST_ASSERT_EQ('@', user->symbol);
	GREATEST_ASSERT_STR_EQ("markand", user->nickname);

	irc_channel_remove(ch, "markand");
	GREATEST_ASSERT(!LIST_FIRST(&ch->users));

	irc_channel_finish(ch);

	GREATEST_PASS();
}

GREATEST_TEST
basics_update(void)
{
	struct irc_channel *ch;
	struct irc_channel_user *user;

	ch = irc_channel_new("#test", NULL, 1);

	irc_channel_add(ch, "markand", 'o', '@');
	irc_channel_add(ch, "jean", 0, 0);
	irc_channel_add(ch, "zoe", 0, 0);
	
	irc_channel_update(ch, "zoe", NULL, 'o', '@');
	user = LIST_FIRST(&ch->users);
	GREATEST_ASSERT_EQ('o', user->mode);
	GREATEST_ASSERT_EQ('@', user->symbol);
	GREATEST_ASSERT_STR_EQ("zoe", user->nickname);

	irc_channel_update(ch, "zoe", "eoz", -1, -1);
	user = LIST_FIRST(&ch->users);
	GREATEST_ASSERT_EQ('o', user->mode);
	GREATEST_ASSERT_EQ('@', user->symbol);
	GREATEST_ASSERT_STR_EQ("eoz", user->nickname);

	irc_channel_finish(ch);

	GREATEST_PASS();
}

GREATEST_SUITE(suite_basics)
{
	GREATEST_RUN_TEST(basics_add);
	GREATEST_RUN_TEST(basics_remove);
	GREATEST_RUN_TEST(basics_update);
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
