/*
 * test-util.c -- test util.h functions
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

#include <string.h>

#include <unity.h>

#include <irccd/util.h>

/* Make sure to run this test with sanitizers enabled. */
void
setUp(void)
{
}

void
tearDown(void)
{
}

static void
basics_size(void)
{
	int array[10] = {0};

	TEST_ASSERT_EQUAL_INT(10, IRC_UTIL_SIZE(array));
}

static void
basics_malloc(void)
{
	int *array;

	array = irc_util_malloc(sizeof (*array) * 10);
	memset(array, 0, sizeof (*array) * 10);
}

static void
basics_calloc(void)
{
	int *array;

	array = irc_util_calloc(10, sizeof (*array));
	memset(array, 0, sizeof (*array) * 10);
}

static void
basics_realloc(void)
{
	int *array = NULL;

	array = irc_util_realloc(array, sizeof (*array) * 10);
	memset(array, 0, sizeof (*array) * 10);
}

static void
basics_reallocarray(void)
{
	int *array = NULL;

	array = irc_util_calloc(10, sizeof (*array));
	array = irc_util_reallocarray(array, 20, sizeof (*array));
	memset(array, 0, sizeof (*array) * 20);
}

static void
basics_memdup(void)
{
	int tab[] = { 1, 2, 3, 4 };
	int *copy;

	copy = irc_util_memdup(tab, sizeof (tab));
	TEST_ASSERT(memcmp(tab, copy, sizeof (tab)) == 0);
}

static void
basics_strdup(void)
{
	char *out;

	out = irc_util_strdup("hello");
	TEST_ASSERT_EQUAL_STRING("hello", out);
}

static void
basics_basename(void)
{
	TEST_ASSERT_EQUAL_STRING("irccd", irc_util_basename("/usr/local/bin/irccd"));
	TEST_ASSERT_EQUAL_STRING("irccd", irc_util_basename("irccd"));
}

static void
basics_dirname(void)
{
	TEST_ASSERT_EQUAL_STRING("/usr/local/bin", irc_util_dirname("/usr/local/bin/irccd"));
	TEST_ASSERT_EQUAL_STRING(".", irc_util_dirname("irccd"));
}

int
main(void)
{
	UNITY_BEGIN();

	RUN_TEST(basics_size);
	RUN_TEST(basics_malloc);
	RUN_TEST(basics_calloc);
	RUN_TEST(basics_realloc);
	RUN_TEST(basics_reallocarray);
	RUN_TEST(basics_memdup);
	RUN_TEST(basics_strdup);
	RUN_TEST(basics_dirname);
	RUN_TEST(basics_basename);

	return UNITY_END();
}
