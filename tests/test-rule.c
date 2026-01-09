/*
 * test-rule.c -- test rule.h functions
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

#include <irccd/rule.h>
#include <irccd/irccd.h>

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

static int catalog;

void
setUp(void)
{
	struct irc_rule *r1, *r2, *r31, *r32;

	if (!catalog)
		return;

	irc_bot_rule_clear();

	/* #1 */
	r1 = irc_rule_new(IRC_RULE_DROP);
	irc_rule_add_channel(r1, "#staff");
	irc_rule_add_event(r1, "onCommand");
	irc_bot_rule_insert(r1, -1);

	/* #2 */
	r2 = irc_rule_new(IRC_RULE_ACCEPT);
	irc_rule_add_server(r2, "unsafe");
	irc_rule_add_channel(r2, "#staff");
	irc_rule_add_event(r2, "onCommand");
	irc_bot_rule_insert(r2, -1);

	/* #3-1 */
	r31 = irc_rule_new(IRC_RULE_DROP);
	irc_rule_add_plugin(r31, "game");
	irc_bot_rule_insert(r31, -1);

	/* #3-2 */
	r32 = irc_rule_new(IRC_RULE_ACCEPT);
	irc_rule_add_server(r32, "malikania");
	irc_rule_add_server(r32, "localhost");
	irc_rule_add_channel(r32, "#games");
	irc_rule_add_plugin(r32, "game");
	irc_rule_add_event(r32, "onCommand");
	irc_rule_add_event(r32, "onMessage");
	irc_bot_rule_insert(r32, -1);
}

void
tearDown(void)
{
	irc_bot_rule_clear();
}

static void
basics_insert(void)
{
	struct irc_rule *r, *r1, *r2;

	r1 = irc_rule_new(IRC_RULE_DROP);
	r2 = irc_rule_new(IRC_RULE_DROP);

	irc_rule_add_server(r1, "s1");
	irc_rule_add_server(r2, "s2");

	irc_bot_rule_insert(r1, 0);
	irc_bot_rule_insert(r2, 0);

	r = irccd->rules;
	TEST_ASSERT_EQUAL_PTR(r2, r);
	r = r->next;
	TEST_ASSERT_EQUAL_PTR(r1, r);
}

static void
basics_remove(void)
{
	struct irc_rule *r, *r1, *r2, *r3;

	r1 = irc_rule_new(IRC_RULE_DROP);
	r2 = irc_rule_new(IRC_RULE_DROP);
	r3 = irc_rule_new(IRC_RULE_DROP);

	irc_rule_add_server(r1, "s1");
	irc_rule_add_server(r2, "s2");
	irc_rule_add_server(r3, "s3");

	irc_bot_rule_insert(r1, -1);
	irc_bot_rule_insert(r2, -1);
	irc_bot_rule_insert(r3, -1);
	irc_bot_rule_remove(1);

	r = irccd->rules;
	TEST_ASSERT_EQUAL_PTR(r1, r);
	r = r->next;
	TEST_ASSERT_EQUAL_PTR(r3, r);

	irc_bot_rule_remove(1);
	r = irccd->rules;
	TEST_ASSERT_EQUAL_PTR(r1, r);

	irc_bot_rule_remove(0);
	r = irccd->rules;
	TEST_ASSERT(!r);
}

static void
solve_match1(void)
{
	struct irc_rule m = {};

	TEST_ASSERT(irc_rule_match(&m, "freenode", "#test", "a", "", ""));
	TEST_ASSERT(irc_rule_match(&m, "", "", "", "", ""));
}

static void
solve_match2(void)
{
	struct irc_rule m = {};

	irc_rule_add_server(&m, "freenode");

	TEST_ASSERT(irc_rule_match(&m, "FreeNode", "#test", "a", "", ""));
	TEST_ASSERT(!irc_rule_match(&m, "malikania", "#test", "a", "", ""));
	TEST_ASSERT(irc_rule_match(&m, "freenode", "", "jean", "", "onMessage"));
}

static void
solve_match3(void)
{
	struct irc_rule m = {};

	irc_rule_add_server(&m, "freenode");
	irc_rule_add_channel(&m, "#staff");

	TEST_ASSERT(irc_rule_match(&m, "freenode", "#staff", "a", "", ""));
	TEST_ASSERT(!irc_rule_match(&m, "freenode", "#test", "a", "", ""));
	TEST_ASSERT(!irc_rule_match(&m, "malikania", "#staff", "a", "", ""));
}

static void
solve_match4(void)
{
	struct irc_rule m = {};

	irc_rule_add_server(&m, "malikania");
	irc_rule_add_channel(&m, "#staff");
	irc_rule_add_origin(&m, "a");

	TEST_ASSERT(irc_rule_match(&m, "malikania", "#staff", "a", "",""));
	TEST_ASSERT(!irc_rule_match(&m, "malikania", "#staff", "b", "", ""));
	TEST_ASSERT(!irc_rule_match(&m, "freenode", "#staff", "a", "", ""));
}

static void
solve_match5(void)
{
	struct irc_rule m = {};

	irc_rule_add_server(&m, "malikania");
	irc_rule_add_server(&m, "freenode");

	TEST_ASSERT(irc_rule_match(&m, "malikania", "", "", "", ""));
	TEST_ASSERT(irc_rule_match(&m, "freenode", "", "", "", ""));
	TEST_ASSERT(!irc_rule_match(&m, "no", "", "", "", ""));
}

static void
solve_match6(void)
{
	struct irc_rule m = {};

	irc_rule_add_server(&m, "malikania");
	irc_rule_add_origin(&m, "markand");

	TEST_ASSERT(irc_rule_match(&m, "malikania", "#staff", "markand", "system", "onCommand"));
	TEST_ASSERT(!irc_rule_match(&m, "malikania", "#staff", "", "system", "onNames"));
	TEST_ASSERT(!irc_rule_match(&m, "malikania", "#staff", "jean", "system", "onMessage"));
}

static void
solve_match7(void)
{
	/* Allowed */
	TEST_ASSERT(irc_rule_matchlist(irccd->rules, "malikania", "#staff", "", "a", "onMessage"));

	/* Allowed */
	TEST_ASSERT(irc_rule_matchlist(irccd->rules, "freenode", "#staff", "", "b", "onTopic"));

	/* Not allowed */
	TEST_ASSERT(!irc_rule_matchlist(irccd->rules, "malikania", "#staff", "", "", "onCommand"));

	/* Not allowed */
	TEST_ASSERT(!irc_rule_matchlist(irccd->rules, "freenode", "#staff", "", "c", "onCommand"));

	/* Allowed */
	TEST_ASSERT(irc_rule_matchlist(irccd->rules, "unsafe", "#staff", "", "c", "onCommand"));
}

static void
solve_match8(void)
{
	/* Allowed */
	TEST_ASSERT(irc_rule_matchlist(irccd->rules, "malikania", "#games", "", "game", "onMessage"));

	/* Allowed */
	TEST_ASSERT(irc_rule_matchlist(irccd->rules, "localhost", "#games", "", "game", "onMessage"));

	/* Allowed */
	TEST_ASSERT(irc_rule_matchlist(irccd->rules, "malikania", "#games", "", "game", "onCommand"));

	/* Not allowed */
	TEST_ASSERT(!irc_rule_matchlist(irccd->rules, "malikania", "#games", "", "game", "onQuery"));

	/* Not allowed */
	TEST_ASSERT(!irc_rule_matchlist(irccd->rules, "freenode", "#no", "", "game", "onMessage"));

	/* Not allowed */
	TEST_ASSERT(!irc_rule_matchlist(irccd->rules, "malikania", "#test", "", "game", "onMessage"));
}

static void
solve_match9(void)
{
	TEST_ASSERT(!irc_rule_matchlist(irccd->rules, "MALIKANIA", "#STAFF", "", "SYSTEM", "onCommand"));
}

int
main(void)
{
	UNITY_BEGIN();

	RUN_TEST(basics_insert);
	RUN_TEST(basics_remove);

	catalog = 1;

	RUN_TEST(solve_match1);
	RUN_TEST(solve_match2);
	RUN_TEST(solve_match3);
	RUN_TEST(solve_match4);
	RUN_TEST(solve_match5);
	RUN_TEST(solve_match6);
	RUN_TEST(solve_match7);
	RUN_TEST(solve_match8);
	RUN_TEST(solve_match9);

	return UNITY_END();
}
