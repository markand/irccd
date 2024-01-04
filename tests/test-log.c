/*
 * test-log.c -- test log.h functions
 *
 * Copyright (c) 2013-2024 David Demelier <markand@malikania.fr>
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

#include <irccd/log.h>

/*
 * Note: we can't really test logs to console and syslog, so use files as best effort. Write some
 * data and check that it is correctly written.
 */

static void
clean(void *udata)
{
	(void)udata;

	remove("stdout.txt");
	remove("stderr.txt");
}

GREATEST_TEST
basics_info_verbose_off(void)
{
	FILE *fpout;
	char out[128] = {0}, err[128] = {0};

	/* Default is quiet, should not log. */
	irc_log_to_file("stdout.txt");
	irc_log_info("hello world!");

	if (!(fpout = fopen("stdout.txt", "r")))
		GREATEST_FAIL();

	fgets(out, sizeof (out), fpout);

	GREATEST_ASSERT_STR_EQ("", out);
	GREATEST_ASSERT_STR_EQ("", err);

	GREATEST_PASS();
}

GREATEST_TEST
basics_info_verbose_on(void)
{
	FILE *fpout;
	char out[128] = {0};

	irc_log_set_verbose(1);
	irc_log_to_file("stdout.txt");
	irc_log_info("hello world!");
	irc_log_info("what's up?");

	if (!(fpout = fopen("stdout.txt", "r")))
		GREATEST_FAIL();

	GREATEST_ASSERT(fgets(out, sizeof (out), fpout));
	GREATEST_ASSERT_STR_EQ("hello world!\n", out);
	GREATEST_ASSERT(fgets(out, sizeof (out), fpout));
	GREATEST_ASSERT_STR_EQ("what's up?\n", out);


	GREATEST_PASS();
}

GREATEST_TEST
basics_warn(void)
{
	FILE *fpout;
	char out[128] = {0};

	/* Warning messages are printed even without verbosity. */
	irc_log_set_verbose(0);
	irc_log_to_file("stdout.txt");
	irc_log_info("this is not printed");
	irc_log_warn("error line 1");
	irc_log_warn("error line 2");

	if (!(fpout = fopen("stdout.txt", "r")))
		GREATEST_FAIL();

	GREATEST_ASSERT(fgets(out, sizeof (out), fpout));
	GREATEST_ASSERT_STR_EQ("error line 1\n", out);
	GREATEST_ASSERT(fgets(out, sizeof (out), fpout));
	GREATEST_ASSERT_STR_EQ("error line 2\n", out);

	GREATEST_PASS();
}

GREATEST_TEST
basics_debug(void)
{
#if !defined(NDEBUG)
	FILE *fpout;
	char out[128] = {0};

	/* Debug messages are printed even without verbosity but requires to be built in debug. */
	irc_log_set_verbose(0);
	irc_log_to_file("stdout.txt");
	irc_log_debug("startup!");
	irc_log_debug("shutdown!");

	if (!(fpout = fopen("stdout.txt", "r")))
		GREATEST_FAIL();

	GREATEST_ASSERT(fgets(out, sizeof (out), fpout));
	GREATEST_ASSERT_STR_EQ("startup!\n", out);
	GREATEST_ASSERT(fgets(out, sizeof (out), fpout));
	GREATEST_ASSERT_STR_EQ("shutdown!\n", out);
#endif
	GREATEST_PASS();
}

GREATEST_SUITE(suite_basics)
{
	GREATEST_SET_SETUP_CB(clean, NULL);
	GREATEST_SET_TEARDOWN_CB(clean, NULL);
	GREATEST_RUN_TEST(basics_info_verbose_off);
	GREATEST_RUN_TEST(basics_info_verbose_on);
	GREATEST_RUN_TEST(basics_warn);
	GREATEST_RUN_TEST(basics_debug);
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
