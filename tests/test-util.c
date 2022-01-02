/*
 * test-util.c -- test util.h functions
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

#include <irccd/util.h>

/* Make sure to run this test with sanitizers enabled. */

GREATEST_TEST
basics_size(void)
{
	int array[10] = {0};

	GREATEST_ASSERT_EQ(10, IRC_UTIL_SIZE(array));
	GREATEST_PASS();
}

GREATEST_TEST
basics_malloc(void)
{
	int *array;

	array = irc_util_malloc(sizeof (*array) * 10);
	memset(array, 0, sizeof (*array) * 10);

	GREATEST_PASS();
}

GREATEST_TEST
basics_calloc(void)
{
	int *array;

	array = irc_util_calloc(10, sizeof (*array));
	memset(array, 0, sizeof (*array) * 10);

	GREATEST_PASS();
}

GREATEST_TEST
basics_realloc(void)
{
	int *array = NULL;

	array = irc_util_realloc(array, sizeof (*array) * 10);
	memset(array, 0, sizeof (*array) * 10);

	GREATEST_PASS();
}

GREATEST_TEST
basics_reallocarray(void)
{
	int *array = NULL;

	array = irc_util_calloc(10, sizeof (*array));
	array = irc_util_reallocarray(array, 20, sizeof (*array));
	memset(array, 0, sizeof (*array) * 20);

	GREATEST_PASS();
}

GREATEST_TEST
basics_memdup(void)
{
	int tab[] = { 1, 2, 3, 4 };
	int *copy;

	copy = irc_util_memdup(tab, sizeof (tab));
	GREATEST_ASSERT(memcmp(tab, copy, sizeof (tab)) == 0);
	GREATEST_PASS();
}

GREATEST_TEST
basics_strdup(void)
{
	char *out;

	out = irc_util_strdup("hello");
	GREATEST_ASSERT_STR_EQ("hello", out);
	GREATEST_PASS();
}

GREATEST_TEST
basics_basename(void)
{
	GREATEST_ASSERT_STR_EQ("irccd", irc_util_basename("/usr/local/bin/irccd"));
	GREATEST_ASSERT_STR_EQ("irccd", irc_util_basename("irccd"));
	GREATEST_PASS();
}

GREATEST_TEST
basics_dirname(void)
{
	GREATEST_ASSERT_STR_EQ("/usr/local/bin", irc_util_dirname("/usr/local/bin/irccd"));
	GREATEST_ASSERT_STR_EQ(".", irc_util_dirname("irccd"));
	GREATEST_PASS();
}

GREATEST_SUITE(suite_basics)
{
	GREATEST_RUN_TEST(basics_size);
	GREATEST_RUN_TEST(basics_malloc);
	GREATEST_RUN_TEST(basics_calloc);
	GREATEST_RUN_TEST(basics_realloc);
	GREATEST_RUN_TEST(basics_reallocarray);
	GREATEST_RUN_TEST(basics_memdup);
	GREATEST_RUN_TEST(basics_strdup);
	GREATEST_RUN_TEST(basics_dirname);
	GREATEST_RUN_TEST(basics_basename);
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
