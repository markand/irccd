/*
 * test-rule.c -- test rule.h functions
 *
 * Copyright (c) 2013-2022 David Demelier <markand@malikania.fr>
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

#include <irccd/rule.h>
#include <irccd/irccd.h>

static void
clean(void *udata)
{
	(void)udata;

	irc_bot_rule_clear();
}

GREATEST_TEST
basics_insert(void)
{
	struct irc_rule *r, *r1, *r2;

	r1 = irc_rule_new(IRC_RULE_DROP);
	r2 = irc_rule_new(IRC_RULE_DROP);

	irc_rule_add(r1->servers, "s1");
	irc_rule_add(r2->servers, "s2");

	irc_bot_rule_insert(r1, 0);
	irc_bot_rule_insert(r2, 0);

	r = irc.rules;
	GREATEST_ASSERT_EQ(r2, r);
	r = r->next;
	GREATEST_ASSERT_EQ(r1, r);

	GREATEST_PASS();
}

GREATEST_TEST
basics_remove(void)
{
	struct irc_rule *r, *r1, *r2, *r3;

	r1 = irc_rule_new(IRC_RULE_DROP);
	r2 = irc_rule_new(IRC_RULE_DROP);
	r3 = irc_rule_new(IRC_RULE_DROP);

	irc_rule_add(r1->servers, "s1");
	irc_rule_add(r2->servers, "s2");
	irc_rule_add(r3->servers, "s3");

	irc_bot_rule_insert(r1, -1);
	irc_bot_rule_insert(r2, -1);
	irc_bot_rule_insert(r3, -1);
	irc_bot_rule_remove(1);

	r = irc.rules;
	GREATEST_ASSERT_EQ(r1, r);
	r = r->next;
	GREATEST_ASSERT_EQ(r3, r);

	irc_bot_rule_remove(1);
	r = irc.rules;
	GREATEST_ASSERT_EQ(r1, r);

	irc_bot_rule_remove(0);
	r = irc.rules;
	GREATEST_ASSERT(!r);

	GREATEST_PASS();
}

/*
 * Simulate the following rules configuration:
 *
 * #
 * # On all servers, each channel #staff can't use the onCommand event,
 * # everything else is allowed.
 * #
 * [rule]       #1
 * servers      = ""
 * channels     = "#staff"
 * events       = "onCommand"
 * action       = drop
 *
 * #
 * # However, the same onCommand on #staff is allowed on server "unsafe"
 * #
 * [rule]       #2
 * servers      = "unsafe"
 * channels     = "#staff"
 * events       = "onCommand"
 * action       = accept
 *
 * #
 * # Plugin game is only allowed on server "malikania" and "localhost",
 * # channel "#games" and events "onMessage, onCommand".
 * #
 * # The first rule #3-1 disable the plugin game for every server, it is
 * # reenabled again with the #3-2.
 * #
 * [rule]       #3-1
 * plugins      = "game"
 * action       = drop
 *
 * [rule]       #3-2
 * servers      = "malikania localhost"
 * channels     = "#games"
 * plugins      = "game"
 * events       = "onMessage onCommand"
 * action       = accept
 */

static void
set(void *udata)
{
	(void)udata;

	struct irc_rule *r1, *r2, *r31, *r32;

	irc_bot_rule_clear();

	/* #1 */
	r1 = irc_rule_new(IRC_RULE_DROP);
	irc_rule_add(r1->channels, "#staff");
	irc_rule_add(r1->events, "onCommand");
	irc_bot_rule_insert(r1, -1);

	/* #2 */
	r2 = irc_rule_new(IRC_RULE_ACCEPT);
	irc_rule_add(r2->servers, "unsafe");
	irc_rule_add(r2->channels, "#staff");
	irc_rule_add(r2->events, "onCommand");
	irc_bot_rule_insert(r2, -1);

	/* #3-1 */
	r31 = irc_rule_new(IRC_RULE_DROP);
	irc_rule_add(r31->plugins, "game");
	irc_bot_rule_insert(r31, -1);

	/* #3-2 */
	r32 = irc_rule_new(IRC_RULE_ACCEPT);
	irc_rule_add(r32->servers, "malikania");
	irc_rule_add(r32->servers, "localhost");
	irc_rule_add(r32->channels, "#games");
	irc_rule_add(r32->plugins, "game");
	irc_rule_add(r32->events, "onCommand");
	irc_rule_add(r32->events, "onMessage");
	irc_bot_rule_insert(r32, -1);
}

GREATEST_TEST
solve_match1(void)
{
	struct irc_rule m = {0};

	GREATEST_ASSERT(irc_rule_match(&m, "freenode", "#test", "a", "", ""));
	GREATEST_ASSERT(irc_rule_match(&m, "", "", "", "", ""));
	GREATEST_PASS();
}

GREATEST_TEST
solve_match2(void)
{
	struct irc_rule m = {0};

	irc_rule_add(m.servers, "freenode");

	GREATEST_ASSERT(irc_rule_match(&m, "FreeNode", "#test", "a", "", ""));
	GREATEST_ASSERT(!irc_rule_match(&m, "malikania", "#test", "a", "", ""));
	GREATEST_ASSERT(irc_rule_match(&m, "freenode", "", "jean", "", "onMessage"));
	GREATEST_PASS();
}

GREATEST_TEST
solve_match3(void)
{
	struct irc_rule m = {0};

	irc_rule_add(m.servers, "freenode");
	irc_rule_add(m.channels, "#staff");

	GREATEST_ASSERT(irc_rule_match(&m, "freenode", "#staff", "a", "", ""));
	GREATEST_ASSERT(!irc_rule_match(&m, "freenode", "#test", "a", "", ""));
	GREATEST_ASSERT(!irc_rule_match(&m, "malikania", "#staff", "a", "", ""));
	GREATEST_PASS();
}

GREATEST_TEST
solve_match4(void)
{
	struct irc_rule m = {0};

	irc_rule_add(m.servers, "malikania");
	irc_rule_add(m.channels, "#staff");
	irc_rule_add(m.origins, "a");

	GREATEST_ASSERT(irc_rule_match(&m, "malikania", "#staff", "a", "",""));
	GREATEST_ASSERT(!irc_rule_match(&m, "malikania", "#staff", "b", "", ""));
	GREATEST_ASSERT(!irc_rule_match(&m, "freenode", "#staff", "a", "", ""));
	GREATEST_PASS();
}

GREATEST_TEST
solve_match5(void)
{
	struct irc_rule m = {0};

	irc_rule_add(m.servers, "malikania");
	irc_rule_add(m.servers, "freenode");

	GREATEST_ASSERT(irc_rule_match(&m, "malikania", "", "", "", ""));
	GREATEST_ASSERT(irc_rule_match(&m, "freenode", "", "", "", ""));
	GREATEST_ASSERT(!irc_rule_match(&m, "no", "", "", "", ""));
	GREATEST_PASS();
}

GREATEST_TEST
solve_match6(void)
{
	struct irc_rule m = {0};

	irc_rule_add(m.servers, "malikania");
	irc_rule_add(m.origins, "markand");

	GREATEST_ASSERT(irc_rule_match(&m, "malikania", "#staff", "markand", "system", "onCommand"));
	GREATEST_ASSERT(!irc_rule_match(&m, "malikania", "#staff", "", "system", "onNames"));
	GREATEST_ASSERT(!irc_rule_match(&m, "malikania", "#staff", "jean", "system", "onMessage"));
	GREATEST_PASS();
}

GREATEST_TEST
solve_match7(void)
{
	/* Allowed */
	GREATEST_ASSERT(irc_rule_matchlist(irc.rules, "malikania", "#staff", "", "a", "onMessage"));

	/* Allowed */
	GREATEST_ASSERT(irc_rule_matchlist(irc.rules, "freenode", "#staff", "", "b", "onTopic"));

	/* Not allowed */
	GREATEST_ASSERT(!irc_rule_matchlist(irc.rules, "malikania", "#staff", "", "", "onCommand"));

	/* Not allowed */
	GREATEST_ASSERT(!irc_rule_matchlist(irc.rules, "freenode", "#staff", "", "c", "onCommand"));

	/* Allowed */
	GREATEST_ASSERT(irc_rule_matchlist(irc.rules, "unsafe", "#staff", "", "c", "onCommand"));

	GREATEST_PASS();
}

GREATEST_TEST
solve_match8(void)
{
	/* Allowed */
	GREATEST_ASSERT(irc_rule_matchlist(irc.rules, "malikania", "#games", "", "game", "onMessage"));

	/* Allowed */
	GREATEST_ASSERT(irc_rule_matchlist(irc.rules, "localhost", "#games", "", "game", "onMessage"));

	/* Allowed */
	GREATEST_ASSERT(irc_rule_matchlist(irc.rules, "malikania", "#games", "", "game", "onCommand"));

	/* Not allowed */
	GREATEST_ASSERT(!irc_rule_matchlist(irc.rules, "malikania", "#games", "", "game", "onQuery"));

	/* Not allowed */
	GREATEST_ASSERT(!irc_rule_matchlist(irc.rules, "freenode", "#no", "", "game", "onMessage"));

	/* Not allowed */
	GREATEST_ASSERT(!irc_rule_matchlist(irc.rules, "malikania", "#test", "", "game", "onMessage"));
	GREATEST_PASS();
}

GREATEST_TEST
solve_match9(void)
{
	GREATEST_ASSERT(!irc_rule_matchlist(irc.rules, "MALIKANIA", "#STAFF", "", "SYSTEM", "onCommand"));
	GREATEST_PASS();
}

GREATEST_SUITE(suite_basics)
{
	GREATEST_SET_SETUP_CB(clean, NULL);
	GREATEST_RUN_TEST(basics_insert);
	GREATEST_RUN_TEST(basics_remove);
}

GREATEST_SUITE(suite_solve)
{
	GREATEST_SET_SETUP_CB(set, NULL);
	GREATEST_RUN_TEST(solve_match1);
	GREATEST_RUN_TEST(solve_match2);
	GREATEST_RUN_TEST(solve_match3);
	GREATEST_RUN_TEST(solve_match4);
	GREATEST_RUN_TEST(solve_match5);
	GREATEST_RUN_TEST(solve_match6);
	GREATEST_RUN_TEST(solve_match7);
	GREATEST_RUN_TEST(solve_match8);
	GREATEST_RUN_TEST(solve_match9);
}

GREATEST_MAIN_DEFS();

int
main(int argc, char **argv)
{
	GREATEST_MAIN_BEGIN();
	GREATEST_RUN_SUITE(suite_basics);
	GREATEST_RUN_SUITE(suite_solve);
	GREATEST_MAIN_END();

	return 0;
}
