/*
 * test-subst.c -- test subst.h functions
 *
 * Copyright (c) 2013-2025 David Demelier <markand@malikania.fr>
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

#define GREATEST_USE_ABBREVS 0
#include <greatest.h>

#include <irccd/subst.h>
#include <irccd/util.h>

GREATEST_TEST
basics_test(void)
{
	struct irc_subst params = {0};
	char buf[1024] = {0};

	GREATEST_ASSERT_EQ(irc_subst(buf, sizeof (buf), "hello world!", &params), 12);
	GREATEST_ASSERT_STR_EQ("hello world!", buf);
	GREATEST_PASS();
}

GREATEST_TEST
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
	char buf[1024] = {0};

	GREATEST_ASSERT_EQ(irc_subst(buf, sizeof (buf), "$@#", &params), 3);
	GREATEST_ASSERT_STR_EQ("$@#", buf);

	GREATEST_ASSERT_EQ(irc_subst(buf, sizeof (buf), " $ @ # ", &params), 7);
	GREATEST_ASSERT_STR_EQ(" $ @ # ", buf);

	GREATEST_ASSERT_EQ(irc_subst(buf, sizeof (buf), "#", &params), 1);
	GREATEST_ASSERT_STR_EQ("#", buf);

	GREATEST_ASSERT_EQ(irc_subst(buf, sizeof (buf), " # ", &params), 3);
	GREATEST_ASSERT_STR_EQ(" # ", buf);

	GREATEST_ASSERT_EQ(irc_subst(buf, sizeof (buf), "#@", &params), 2);
	GREATEST_ASSERT_STR_EQ("#@", buf);

	GREATEST_ASSERT_EQ(irc_subst(buf, sizeof (buf), "##", &params), 1);
	GREATEST_ASSERT_STR_EQ("#", buf);

	GREATEST_ASSERT_EQ(irc_subst(buf, sizeof (buf), "#!", &params), 2);
	GREATEST_ASSERT_STR_EQ("#!", buf);

	GREATEST_ASSERT_EQ(irc_subst(buf, sizeof (buf), "##{target}", &params), 9);
	GREATEST_ASSERT_STR_EQ("#{target}", buf);

	GREATEST_ASSERT_EQ(irc_subst(buf, sizeof (buf), "@#{target}", &params), 6);
	GREATEST_ASSERT_STR_EQ("@hello", buf);

	GREATEST_ASSERT_EQ(irc_subst(buf, sizeof (buf), "#{target}#", &params), 6);
	GREATEST_ASSERT_STR_EQ("hello#", buf);

	GREATEST_ASSERT_EQ(irc_subst(buf, sizeof (buf), "abc##xyz", &params), 7);
	GREATEST_ASSERT_STR_EQ("abc#xyz", buf);

	GREATEST_ASSERT_EQ(irc_subst(buf, sizeof (buf), "abc###xyz", &params), 8);
	GREATEST_ASSERT_STR_EQ("abc##xyz", buf);

	GREATEST_ASSERT_EQ(irc_subst(buf, sizeof (buf), "#{failure", &params), -1);
	GREATEST_ASSERT_EQ(errno, EINVAL);
	GREATEST_ASSERT_STR_EQ("", buf);
	GREATEST_PASS();
}

GREATEST_TEST
disable_date(void)
{
	struct irc_subst params = {0};
	char buf[1024] = {0};

	GREATEST_ASSERT_EQ(irc_subst(buf, sizeof (buf), "%H:%M", &params), 5);
	GREATEST_ASSERT_STR_EQ("%H:%M", buf);
	GREATEST_PASS();
}

GREATEST_TEST
disable_keywords(void)
{
	struct irc_subst_keyword kw[] = {
		{ "target", "hello" }
	};
	struct irc_subst params = {
		.keywords = kw,
		.keywordsz = IRC_UTIL_SIZE(kw)
	};
	char buf[1024] = {0};

	GREATEST_ASSERT_EQ(irc_subst(buf, sizeof (buf), "#{target}", &params), 9);
	GREATEST_ASSERT_STR_EQ("#{target}", buf);
	GREATEST_PASS();
}

GREATEST_TEST
disable_env(void)
{
	struct irc_subst params = {0};
	char buf[1024] = {0};

	GREATEST_ASSERT_EQ(irc_subst(buf, sizeof (buf), "${HOME}", &params), 7);
	GREATEST_ASSERT_STR_EQ("${HOME}", buf);
	GREATEST_PASS();
}

GREATEST_TEST
disable_shell(void)
{
	struct irc_subst params = {0};
	char buf[1024] = {0};

	GREATEST_ASSERT_EQ(irc_subst(buf, sizeof (buf), "!{hostname}", &params), 11);
	GREATEST_ASSERT_STR_EQ("!{hostname}", buf);
	GREATEST_PASS();
}

GREATEST_TEST
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
	char buf[1024] = {0};

	GREATEST_ASSERT_EQ(irc_subst(buf, sizeof (buf), "hello #{target}!", &params), 12);
	GREATEST_ASSERT_STR_EQ("hello irccd!", buf);
	GREATEST_PASS();
}

GREATEST_TEST
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
	char buf[1024] = {0};

	GREATEST_ASSERT_EQ(irc_subst(buf, sizeof (buf), "hello #{target} from #{source}!", &params), 27);
	GREATEST_ASSERT_STR_EQ("hello irccd from nightmare!", buf);
	GREATEST_PASS();
}

GREATEST_TEST
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
	char buf[1024] = {0};

	GREATEST_ASSERT_EQ(irc_subst(buf, sizeof (buf), "hello #{target}#{target}!", &params), 17);
	GREATEST_ASSERT_STR_EQ("hello irccdirccd!", buf);
	GREATEST_PASS();
}

GREATEST_TEST
keywords_missing(void)
{
	struct irc_subst params = {
		.flags = IRC_SUBST_KEYWORDS
	};
	char buf[1024] = {0};

	GREATEST_ASSERT_EQ(irc_subst(buf, sizeof (buf), "hello #{target}!", &params), 7);
	GREATEST_ASSERT_STR_EQ("hello !", buf);
	GREATEST_PASS();
}

GREATEST_TEST
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
	char buf[10] = {0};

	GREATEST_ASSERT_EQ(irc_subst(buf, sizeof (buf), "hello #{target}!", &params), -1);
	GREATEST_ASSERT_EQ(ENOMEM, errno);
	GREATEST_ASSERT_STR_EQ("", buf);
	GREATEST_PASS();
}

GREATEST_TEST
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
	char buf[1024] = {0};

	GREATEST_ASSERT_EQ(irc_subst(buf, sizeof (buf), "hello #{target!", &params), -1);
	GREATEST_ASSERT_EQ(EINVAL, errno);
	GREATEST_ASSERT_STR_EQ("", buf);
	GREATEST_PASS();
}

GREATEST_TEST
env_simple(void)
{
	const char *home = getenv("HOME");
	char tmp[1024];

	if (home) {
		struct irc_subst params = {
			.flags = IRC_SUBST_ENV,
		};
		char buf[1024] = {0};

		snprintf(tmp, sizeof (tmp), "my home is %s", home);

		GREATEST_ASSERT_EQ((size_t)irc_subst(buf, sizeof (buf), "my home is ${HOME}", &params), strlen(tmp));
		GREATEST_ASSERT_STR_EQ(tmp, buf);
	}

	GREATEST_PASS();
}

GREATEST_TEST
env_missing(void)
{
	struct irc_subst params = {
		.flags = IRC_SUBST_ENV,
	};
	char buf[1024] = {0};

	GREATEST_ASSERT_EQ(irc_subst(buf, sizeof (buf), "value is ${HOPE_THIS_VAR_NOT_EXIST}", &params), 9);
	GREATEST_ASSERT_STR_EQ("value is ", buf);
	GREATEST_PASS();
}

GREATEST_TEST
env_enomem(void)
{
	const char *home = getenv("HOME");

	if (home) {
		struct irc_subst params = {
			.flags = IRC_SUBST_ENV
		};
		char buf[10];

		GREATEST_ASSERT_EQ(irc_subst(buf, sizeof (buf), "value is ${HOME}", &params), -1);
		GREATEST_ASSERT_EQ(ENOMEM, errno);
		GREATEST_ASSERT_STR_EQ("", buf);
	}

	GREATEST_PASS();
}

GREATEST_TEST
shell_simple(void)
{
	struct irc_subst params = {
		.flags = IRC_SUBST_SHELL
	};
	char buf[1024] = {0};
	char tmp[1024] = {0};
	time_t now = time(NULL);
	struct tm *cal = localtime(&now);

	strftime(tmp, sizeof (tmp), "year: %Y", cal);

	GREATEST_ASSERT_EQ(irc_subst(buf, sizeof (buf), "year: !{date +%Y}", &params), 10);
	GREATEST_ASSERT_STR_EQ(tmp, buf);
	GREATEST_PASS();
}

GREATEST_TEST
shell_no_new_line(void)
{
	struct irc_subst params = {
		.flags = IRC_SUBST_SHELL
	};
	char buf[1024] = {0};

	GREATEST_ASSERT_EQ(irc_subst(buf, sizeof (buf), "hello !{printf world}", &params), 11);
	GREATEST_ASSERT_STR_EQ("hello world", buf);
	GREATEST_PASS();
}

GREATEST_TEST
shattrs_simple(void)
{
	struct irc_subst params = {
		.flags = IRC_SUBST_SHELL_ATTRS
	};
	char buf[1024] = {0};

	/* On shell attributes, all components are optional. */
	GREATEST_ASSERT_EQ(irc_subst(buf, sizeof (buf), "@{red}red@{}", &params), 12);
	GREATEST_ASSERT_STR_EQ("\033[31mred\033[0m", buf);

	GREATEST_ASSERT_EQ(irc_subst(buf, sizeof (buf), "@{red,blue}red on blue@{}", &params), 23);
	GREATEST_ASSERT_STR_EQ("\033[31;44mred on blue\033[0m", buf);

	GREATEST_ASSERT_EQ(irc_subst(buf, sizeof (buf), "@{red,blue,bold}bold red on blue@{}", &params), 30);
	GREATEST_ASSERT_STR_EQ("\033[1;31;44mbold red on blue\033[0m", buf);

	GREATEST_PASS();
}

GREATEST_TEST
shattrs_enomem(void)
{
	struct irc_subst params = {
		.flags = IRC_SUBST_SHELL_ATTRS
	};
	char buf[10] = {0};

	GREATEST_ASSERT_EQ(irc_subst(buf, sizeof (buf), "@{red}hello world in red@{}", &params), -1);
	GREATEST_ASSERT_EQ(ENOMEM, errno);
	GREATEST_ASSERT_STR_EQ("", buf);
	GREATEST_PASS();
}

GREATEST_TEST
shattrs_invalid_color(void)
{
	struct irc_subst params = {
		.flags = IRC_SUBST_SHELL_ATTRS
	};
	char buf[1024] = {0};

	GREATEST_ASSERT_EQ(irc_subst(buf, sizeof (buf), "@{invalid}standard@{}", &params), 15);
	GREATEST_ASSERT_STR_EQ("\033[mstandard\033[0m", buf);

	GREATEST_PASS();
}

GREATEST_TEST
ircattrs_simple(void)
{
	struct irc_subst params = {
		.flags = IRC_SUBST_IRC_ATTRS
	};
	char buf[1024] = {0};

	/* In IRC the foreground is required if the background is desired. */
	GREATEST_ASSERT_EQ(irc_subst(buf, sizeof (buf), "@{red}red@{}", &params), 6);
	GREATEST_ASSERT_STR_EQ("\x03""4red\x03", buf);

	GREATEST_ASSERT_EQ(irc_subst(buf, sizeof (buf), "@{red,blue}red on blue@{}", &params), 16);
	GREATEST_ASSERT_STR_EQ("\x03""4,2red on blue\x03", buf);

	GREATEST_ASSERT_EQ(irc_subst(buf, sizeof (buf), "@{red,blue,bold}bold red on blue@{}", &params), 22);
	GREATEST_ASSERT_STR_EQ("\x03" "4,2" "\x02" "bold red on blue\x03", buf);

	GREATEST_PASS();
}

GREATEST_TEST
ircattrs_enomem(void)
{
	struct irc_subst params = {
		.flags = IRC_SUBST_IRC_ATTRS
	};
	char buf[10] = {0};

	GREATEST_ASSERT_EQ(irc_subst(buf, sizeof (buf), "@{red}hello world in red@{}", &params), -1);
	GREATEST_ASSERT_EQ(ENOMEM, errno);
	GREATEST_ASSERT_STR_EQ("", buf);
	GREATEST_PASS();
}

GREATEST_TEST
ircattrs_invalid_color(void)
{
	struct irc_subst params = {
		.flags = IRC_SUBST_IRC_ATTRS
	};
	char buf[1024] = {0};

	GREATEST_ASSERT_EQ(irc_subst(buf, sizeof (buf), "@{invalid}standard@{}", &params), 10);
	GREATEST_ASSERT_STR_EQ("\x03" "standard" "\x03", buf);

	GREATEST_PASS();
}


GREATEST_SUITE(suite_basics)
{
	GREATEST_RUN_TEST(basics_test);
	GREATEST_RUN_TEST(basics_escape);
}

GREATEST_SUITE(suite_disable)
{
	GREATEST_RUN_TEST(disable_date);
	GREATEST_RUN_TEST(disable_keywords);
	GREATEST_RUN_TEST(disable_env);
	GREATEST_RUN_TEST(disable_shell);
}

GREATEST_SUITE(suite_keywords)
{
	GREATEST_RUN_TEST(keywords_simple);
	GREATEST_RUN_TEST(keywords_multiple);
	GREATEST_RUN_TEST(keywords_adj_twice);
	GREATEST_RUN_TEST(keywords_missing);
	GREATEST_RUN_TEST(keywords_enomem);
	GREATEST_RUN_TEST(keywords_einval);
}

GREATEST_SUITE(suite_env)
{
	GREATEST_RUN_TEST(env_simple);
	GREATEST_RUN_TEST(env_missing);
	GREATEST_RUN_TEST(env_enomem);
}

GREATEST_SUITE(suite_shell)
{
	GREATEST_RUN_TEST(shell_simple);
	GREATEST_RUN_TEST(shell_no_new_line);
}

GREATEST_SUITE(suite_shattrs)
{
	GREATEST_RUN_TEST(shattrs_simple);
	GREATEST_RUN_TEST(shattrs_enomem);
	GREATEST_RUN_TEST(shattrs_invalid_color);
}

GREATEST_SUITE(suite_ircattrs)
{
	GREATEST_RUN_TEST(ircattrs_simple);
	GREATEST_RUN_TEST(ircattrs_enomem);
	GREATEST_RUN_TEST(ircattrs_invalid_color);
}

GREATEST_MAIN_DEFS();

int
main(int argc, char **argv)
{
	GREATEST_MAIN_BEGIN();
	GREATEST_RUN_SUITE(suite_basics);
	GREATEST_RUN_SUITE(suite_disable);
	GREATEST_RUN_SUITE(suite_keywords);
	GREATEST_RUN_SUITE(suite_env);
	GREATEST_RUN_SUITE(suite_shell);
	GREATEST_RUN_SUITE(suite_shattrs);
	GREATEST_RUN_SUITE(suite_ircattrs);
	GREATEST_MAIN_END();

	return 0;
}
