/*
 * test-subst.c -- test subst.h functions
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

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <unity.h>

#include <irccd/subst.h>
#include <irccd/util.h>

void
setUp(void)
{
}

void
tearDown(void)
{
}

static void
basics_test(void)
{
	struct irc_subst params = {};
	char buf[1024] = {};

	TEST_ASSERT_EQUAL_INT(irc_subst(buf, sizeof (buf), "hello world!", &params), 12);
	TEST_ASSERT_EQUAL_STRING("hello world!", buf);
}

static void
basics_escape(void)
{
	struct irc_subst_keyword kw[] = {
		{ "target", "hello" }
	};
	struct irc_subst params = {
		.flags = IRC_SUBST_KEYWORDS,
		.keywords = kw,
		.keywordsz = IRC_UTIL_SIZE(kw)
	};
	char buf[1024] = {};

	TEST_ASSERT_EQUAL_INT(irc_subst(buf, sizeof (buf), "$@#", &params), 3);
	TEST_ASSERT_EQUAL_STRING("$@#", buf);

	TEST_ASSERT_EQUAL_INT(irc_subst(buf, sizeof (buf), " $ @ # ", &params), 7);
	TEST_ASSERT_EQUAL_STRING(" $ @ # ", buf);

	TEST_ASSERT_EQUAL_INT(irc_subst(buf, sizeof (buf), "#", &params), 1);
	TEST_ASSERT_EQUAL_STRING("#", buf);

	TEST_ASSERT_EQUAL_INT(irc_subst(buf, sizeof (buf), " # ", &params), 3);
	TEST_ASSERT_EQUAL_STRING(" # ", buf);

	TEST_ASSERT_EQUAL_INT(irc_subst(buf, sizeof (buf), "#@", &params), 2);
	TEST_ASSERT_EQUAL_STRING("#@", buf);

	TEST_ASSERT_EQUAL_INT(irc_subst(buf, sizeof (buf), "##", &params), 1);
	TEST_ASSERT_EQUAL_STRING("#", buf);

	TEST_ASSERT_EQUAL_INT(irc_subst(buf, sizeof (buf), "#!", &params), 2);
	TEST_ASSERT_EQUAL_STRING("#!", buf);

	TEST_ASSERT_EQUAL_INT(irc_subst(buf, sizeof (buf), "##{target}", &params), 9);
	TEST_ASSERT_EQUAL_STRING("#{target}", buf);

	TEST_ASSERT_EQUAL_INT(irc_subst(buf, sizeof (buf), "@#{target}", &params), 6);
	TEST_ASSERT_EQUAL_STRING("@hello", buf);

	TEST_ASSERT_EQUAL_INT(irc_subst(buf, sizeof (buf), "#{target}#", &params), 6);
	TEST_ASSERT_EQUAL_STRING("hello#", buf);

	TEST_ASSERT_EQUAL_INT(irc_subst(buf, sizeof (buf), "abc##xyz", &params), 7);
	TEST_ASSERT_EQUAL_STRING("abc#xyz", buf);

	TEST_ASSERT_EQUAL_INT(irc_subst(buf, sizeof (buf), "abc###xyz", &params), 8);
	TEST_ASSERT_EQUAL_STRING("abc##xyz", buf);

	TEST_ASSERT_EQUAL_INT(irc_subst(buf, sizeof (buf), "#{failure", &params), -EINVAL);
	TEST_ASSERT_EQUAL_STRING("", buf);
}

static void
disable_date(void)
{
	struct irc_subst params = {};
	char buf[1024] = {};

	TEST_ASSERT_EQUAL_INT(irc_subst(buf, sizeof (buf), "%H:%M", &params), 5);
	TEST_ASSERT_EQUAL_STRING("%H:%M", buf);
}

static void
disable_keywords(void)
{
	struct irc_subst_keyword kw[] = {
		{ "target", "hello" }
	};
	struct irc_subst params = {
		.keywords = kw,
		.keywordsz = IRC_UTIL_SIZE(kw)
	};
	char buf[1024] = {};

	TEST_ASSERT_EQUAL_INT(irc_subst(buf, sizeof (buf), "#{target}", &params), 9);
	TEST_ASSERT_EQUAL_STRING("#{target}", buf);
}

static void
disable_env(void)
{
	struct irc_subst params = {};
	char buf[1024] = {};

	TEST_ASSERT_EQUAL_INT(irc_subst(buf, sizeof (buf), "${HOME}", &params), 7);
	TEST_ASSERT_EQUAL_STRING("${HOME}", buf);
}

static void
disable_shell(void)
{
	struct irc_subst params = {};
	char buf[1024] = {};

	TEST_ASSERT_EQUAL_INT(irc_subst(buf, sizeof (buf), "!{hostname}", &params), 11);
	TEST_ASSERT_EQUAL_STRING("!{hostname}", buf);
}

static void
keywords_simple(void)
{
	struct irc_subst_keyword kw[] = {
		{ "target", "irccd" }
	};
	struct irc_subst params = {
		.flags = IRC_SUBST_KEYWORDS,
		.keywords = kw,
		.keywordsz = IRC_UTIL_SIZE(kw)
	};
	char buf[1024] = {};

	TEST_ASSERT_EQUAL_INT(irc_subst(buf, sizeof (buf), "hello #{target}!", &params), 12);
	TEST_ASSERT_EQUAL_STRING("hello irccd!", buf);
}

static void
keywords_multiple(void)
{
	struct irc_subst_keyword kw[] = {
		{ "target", "irccd" },
		{ "source", "nightmare" }
	};
	struct irc_subst params = {
		.flags = IRC_SUBST_KEYWORDS,
		.keywords = kw,
		.keywordsz = IRC_UTIL_SIZE(kw)
	};
	char buf[1024] = {};

	TEST_ASSERT_EQUAL_INT(irc_subst(buf, sizeof (buf), "hello #{target} from #{source}!", &params), 27);
	TEST_ASSERT_EQUAL_STRING("hello irccd from nightmare!", buf);
}

static void
keywords_adj_twice(void)
{
	struct irc_subst_keyword kw[] = {
		{ "target", "irccd" }
	};
	struct irc_subst params = {
		.flags = IRC_SUBST_KEYWORDS,
		.keywords = kw,
		.keywordsz = IRC_UTIL_SIZE(kw)
	};
	char buf[1024] = {};

	TEST_ASSERT_EQUAL_INT(irc_subst(buf, sizeof (buf), "hello #{target}#{target}!", &params), 17);
	TEST_ASSERT_EQUAL_STRING("hello irccdirccd!", buf);
}

static void
keywords_missing(void)
{
	struct irc_subst params = {
		.flags = IRC_SUBST_KEYWORDS
	};
	char buf[1024] = {};

	TEST_ASSERT_EQUAL_INT(irc_subst(buf, sizeof (buf), "hello #{target}!", &params), 7);
	TEST_ASSERT_EQUAL_STRING("hello !", buf);
}

static void
keywords_enomem(void)
{
	struct irc_subst_keyword kw[] = {
		{ "target", "irccd" }
	};
	struct irc_subst params = {
		.flags = IRC_SUBST_KEYWORDS,
		.keywords = kw,
		.keywordsz = IRC_UTIL_SIZE(kw)
	};
	char buf[10] = {};

	TEST_ASSERT_EQUAL_INT(irc_subst(buf, sizeof (buf), "hello #{target}!", &params), -ENOMEM);
	TEST_ASSERT_EQUAL_STRING("", buf);
}

static void
keywords_einval(void)
{
	struct irc_subst_keyword kw[] = {
		{ "target", "irccd" }
	};
	struct irc_subst params = {
		.flags = IRC_SUBST_KEYWORDS,
		.keywords = kw,
		.keywordsz = IRC_UTIL_SIZE(kw)
	};
	char buf[1024] = {};

	TEST_ASSERT_EQUAL_INT(irc_subst(buf, sizeof (buf), "hello #{target!", &params), -EINVAL);
	TEST_ASSERT_EQUAL_STRING("", buf);
}

static void
env_simple(void)
{
	const char *home = getenv("HOME");
	char tmp[1024];

	if (home) {
		struct irc_subst params = {
			.flags = IRC_SUBST_ENV,
		};
		char buf[1024] = {};

		snprintf(tmp, sizeof (tmp), "my home is %s", home);

		TEST_ASSERT_EQUAL_INT((size_t)irc_subst(buf, sizeof (buf), "my home is ${HOME}", &params), strlen(tmp));
		TEST_ASSERT_EQUAL_STRING(tmp, buf);
	}
}

static void
env_missing(void)
{
	struct irc_subst params = {
		.flags = IRC_SUBST_ENV,
	};
	char buf[1024] = {};

	TEST_ASSERT_EQUAL_INT(irc_subst(buf, sizeof (buf), "value is ${HOPE_THIS_VAR_NOT_EXIST}", &params), 9);
	TEST_ASSERT_EQUAL_STRING("value is ", buf);
}

static void
env_enomem(void)
{
	const char *home = getenv("HOME");

	if (home) {
		struct irc_subst params = {
			.flags = IRC_SUBST_ENV
		};
		char buf[10];

		TEST_ASSERT_EQUAL_INT(irc_subst(buf, sizeof (buf), "value is ${HOME}", &params), -ENOMEM);
		TEST_ASSERT_EQUAL_STRING("", buf);
	}
}

static void
shell_simple(void)
{
	struct irc_subst params = {
		.flags = IRC_SUBST_SHELL
	};
	char buf[1024] = {};
	char tmp[1024] = {};
	time_t now = time(NULL);
	struct tm *cal = localtime(&now);

	strftime(tmp, sizeof (tmp), "year: %Y", cal);

	TEST_ASSERT_EQUAL_INT(irc_subst(buf, sizeof (buf), "year: !{date +%Y}", &params), 10);
	TEST_ASSERT_EQUAL_STRING(tmp, buf);
}

static void
shell_no_new_line(void)
{
	struct irc_subst params = {
		.flags = IRC_SUBST_SHELL
	};
	char buf[1024] = {};

	TEST_ASSERT_EQUAL_INT(irc_subst(buf, sizeof (buf), "hello !{printf world}", &params), 11);
	TEST_ASSERT_EQUAL_STRING("hello world", buf);
}

static void
shattrs_simple(void)
{
	struct irc_subst params = {
		.flags = IRC_SUBST_SHELL_ATTRS
	};
	char buf[1024] = {};

	/* On shell attributes, all components are optional. */
	TEST_ASSERT_EQUAL_INT(irc_subst(buf, sizeof (buf), "@{red}red@{}", &params), 12);
	TEST_ASSERT_EQUAL_STRING("\033[31mred\033[0m", buf);

	TEST_ASSERT_EQUAL_INT(irc_subst(buf, sizeof (buf), "@{red,blue}red on blue@{}", &params), 23);
	TEST_ASSERT_EQUAL_STRING("\033[31;44mred on blue\033[0m", buf);

	TEST_ASSERT_EQUAL_INT(irc_subst(buf, sizeof (buf), "@{red,blue,bold}bold red on blue@{}", &params), 30);
	TEST_ASSERT_EQUAL_STRING("\033[1;31;44mbold red on blue\033[0m", buf);
}

static void
shattrs_enomem(void)
{
	struct irc_subst params = {
		.flags = IRC_SUBST_SHELL_ATTRS
	};
	char buf[10] = {};

	TEST_ASSERT_EQUAL_INT(irc_subst(buf, sizeof (buf), "@{red}hello world in red@{}", &params), -ENOMEM);
	TEST_ASSERT_EQUAL_STRING("", buf);
}

static void
shattrs_invalid_color(void)
{
	struct irc_subst params = {
		.flags = IRC_SUBST_SHELL_ATTRS
	};
	char buf[1024] = {};

	TEST_ASSERT_EQUAL_INT(irc_subst(buf, sizeof (buf), "@{invalid}standard@{}", &params), 15);
	TEST_ASSERT_EQUAL_STRING("\033[mstandard\033[0m", buf);
}

static void
ircattrs_simple(void)
{
	struct irc_subst params = {
		.flags = IRC_SUBST_IRC_ATTRS
	};
	char buf[1024] = {};

	/* In IRC the foreground is required if the background is desired. */
	TEST_ASSERT_EQUAL_INT(irc_subst(buf, sizeof (buf), "@{red}red@{}", &params), 6);
	TEST_ASSERT_EQUAL_STRING("\x03""4red\x03", buf);

	TEST_ASSERT_EQUAL_INT(irc_subst(buf, sizeof (buf), "@{red,blue}red on blue@{}", &params), 16);
	TEST_ASSERT_EQUAL_STRING("\x03""4,2red on blue\x03", buf);

	TEST_ASSERT_EQUAL_INT(irc_subst(buf, sizeof (buf), "@{red,blue,bold}bold red on blue@{}", &params), 22);
	TEST_ASSERT_EQUAL_STRING("\x03" "4,2" "\x02" "bold red on blue\x03", buf);
}

static void
ircattrs_enomem(void)
{
	struct irc_subst params = {
		.flags = IRC_SUBST_IRC_ATTRS
	};
	char buf[10] = {};

	TEST_ASSERT_EQUAL_INT(irc_subst(buf, sizeof (buf), "@{red}hello world in red@{}", &params), -ENOMEM);
	TEST_ASSERT_EQUAL_STRING("", buf);
}

static void
ircattrs_invalid_color(void)
{
	struct irc_subst params = {
		.flags = IRC_SUBST_IRC_ATTRS
	};
	char buf[1024] = {};

	TEST_ASSERT_EQUAL_INT(irc_subst(buf, sizeof (buf), "@{invalid}standard@{}", &params), 10);
	TEST_ASSERT_EQUAL_STRING("\x03" "standard" "\x03", buf);
}

int
main(void)
{
	UNITY_BEGIN();

	RUN_TEST(basics_test);
	RUN_TEST(basics_escape);
	RUN_TEST(disable_date);
	RUN_TEST(disable_keywords);
	RUN_TEST(disable_env);
	RUN_TEST(disable_shell);
	RUN_TEST(keywords_simple);
	RUN_TEST(keywords_multiple);
	RUN_TEST(keywords_adj_twice);
	RUN_TEST(keywords_missing);
	RUN_TEST(keywords_enomem);
	RUN_TEST(keywords_einval);
	RUN_TEST(env_simple);
	RUN_TEST(env_missing);
	RUN_TEST(env_enomem);
	RUN_TEST(shell_simple);
	RUN_TEST(shell_no_new_line);
	RUN_TEST(shattrs_simple);
	RUN_TEST(shattrs_enomem);
	RUN_TEST(shattrs_invalid_color);
	RUN_TEST(ircattrs_simple);
	RUN_TEST(ircattrs_enomem);
	RUN_TEST(ircattrs_invalid_color);

	return UNITY_END();
}
