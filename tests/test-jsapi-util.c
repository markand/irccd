/*
 * test-jsapi-util.c -- test Irccd.Util API
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

#define GREATEST_USE_ABBREVS 0
#include <greatest.h>

#include <irccd/js-plugin.h>
#include <irccd/plugin.h>

static struct irc_plugin *plugin;
static duk_context *ctx;

static void
setup(void *udata)
{
	(void)udata;

	plugin = js_plugin_open("example", TOP "/tests/data/example-plugin.js");
	ctx = js_plugin_get_context(plugin);
}

static void
teardown(void *udata)
{
	(void)udata;

	irc_plugin_finish(plugin);

	plugin = NULL;
	ctx = NULL;
}

GREATEST_TEST
basics_splituser(void)
{
	if (duk_peval_string(ctx, "result = Irccd.Util.splituser(\"user!~user@hyper/super/host\");") != 0)
		GREATEST_FAIL();

	GREATEST_ASSERT(duk_get_global_string(ctx, "result"));
	GREATEST_ASSERT_STR_EQ("user", duk_get_string(ctx, -1));

	GREATEST_PASS();
}

GREATEST_TEST
basics_splithost(void)
{
	if (duk_peval_string(ctx, "result = Irccd.Util.splithost(\"user!~user@hyper/super/host\");") != 0)
		GREATEST_FAIL();

	GREATEST_ASSERT(duk_get_global_string(ctx, "result"));
	GREATEST_ASSERT_STR_EQ("hyper/super/host", duk_get_string(ctx, -1));

	GREATEST_PASS();
}

GREATEST_SUITE(suite_basics)
{
	GREATEST_SET_SETUP_CB(setup, NULL);
	GREATEST_SET_TEARDOWN_CB(teardown, NULL);
	GREATEST_RUN_TEST(basics_splituser);
	GREATEST_RUN_TEST(basics_splithost);
}

GREATEST_TEST
format_simple(void)
{
	const int ret = duk_peval_string(ctx,
		"result = Irccd.Util.format(\"#{target}\", { target: \"markand\" })"
	);

	if (ret != 0)
		GREATEST_FAIL();

	GREATEST_ASSERT(duk_get_global_string(ctx, "result"));
	GREATEST_ASSERT_STR_EQ("markand", duk_get_string(ctx, -1));

	GREATEST_PASS();
}

GREATEST_SUITE(suite_format)
{
	GREATEST_RUN_TEST(format_simple);
}

GREATEST_TEST
cut_string_simple(void)
{
	const int ret = duk_peval_string(ctx,
		"lines = Irccd.Util.cut('hello world');\n"
		"line0 = lines[0];\n"
	);

	if (ret != 0)
		GREATEST_FAIL();

	GREATEST_ASSERT(duk_get_global_string(ctx, "line0"));
	GREATEST_ASSERT_STR_EQ("hello world", duk_get_string(ctx, -1));

	GREATEST_PASS();
}

GREATEST_TEST
cut_string_double(void)
{
	const int ret = duk_peval_string(ctx,
		"lines = Irccd.Util.cut('hello world', 5);\n"
		"line0 = lines[0];\n"
		"line1 = lines[1];\n"
	);

	if (ret != 0)
		GREATEST_FAIL();

	GREATEST_ASSERT(duk_get_global_string(ctx, "line0"));
	GREATEST_ASSERT_STR_EQ("hello", duk_get_string(ctx, -1));
	GREATEST_ASSERT(duk_get_global_string(ctx, "line1"));
	GREATEST_ASSERT_STR_EQ("world", duk_get_string(ctx, -1));

	GREATEST_PASS();
}

GREATEST_TEST
cut_string_dirty(void)
{
	const int ret = duk_peval_string(ctx,
		"lines = Irccd.Util.cut('	 hello	world	 ', 5);\n"
		"line0 = lines[0];\n"
		"line1 = lines[1];\n"
	);

	if (ret != 0)
		GREATEST_FAIL();

	GREATEST_ASSERT(duk_get_global_string(ctx, "line0"));
	GREATEST_ASSERT_STR_EQ("hello", duk_get_string(ctx, -1));
	GREATEST_ASSERT(duk_get_global_string(ctx, "line1"));
	GREATEST_ASSERT_STR_EQ("world", duk_get_string(ctx, -1));

	GREATEST_PASS();
}

GREATEST_TEST
cut_string_too_much_lines(void)
{
	const int ret = duk_peval_string(ctx,
		"try {"
		"  lines = Irccd.Util.cut('abc def ghi jkl', 3, 3);"
		"} catch (e) {\n"
		"  name = e.name;\n"
		"  message = e.message;\n"
		"}\n"
	);

	if (ret != 0)
		GREATEST_FAIL();

	GREATEST_ASSERT(duk_get_global_string(ctx, "name"));
	GREATEST_ASSERT_STR_EQ("RangeError", duk_get_string(ctx, -1));
	GREATEST_ASSERT(duk_get_global_string(ctx, "message"));
	GREATEST_ASSERT_STR_EQ("number of lines exceeds maxl (3)", duk_get_string(ctx, -1));
	GREATEST_PASS();
}

GREATEST_TEST
cut_string_token_too_big(void)
{
	const int ret = duk_peval_string(ctx,
		"try {\n"
		"  lines = Irccd.Util.cut('hello world', 3);\n"
		"} catch (e) {\n"
		"  name = e.name;\n"
		"  message = e.message;\n"
		"}\n"
	);

	if (ret != 0)
		GREATEST_FAIL();

	GREATEST_ASSERT(duk_get_global_string(ctx, "name"));
	GREATEST_ASSERT_STR_EQ("RangeError", duk_get_string(ctx, -1));
	GREATEST_ASSERT(duk_get_global_string(ctx, "message"));
	GREATEST_ASSERT_STR_EQ("token 'hello' could not fit in maxc limit (3)", duk_get_string(ctx, -1));

	GREATEST_PASS();
}

GREATEST_TEST
cut_string_negative_maxc(void)
{
	const int ret = duk_peval_string(ctx,
		"try {\n"
		"  lines = Irccd.Util.cut('hello world', -3);\n"
		"} catch (e) {\n"
		"  name = e.name;\n"
		"  message = e.message;\n"
		"}\n"
	);

	if (ret != 0)
		GREATEST_FAIL();

	GREATEST_ASSERT(duk_get_global_string(ctx, "name"));
	GREATEST_ASSERT_STR_EQ("RangeError", duk_get_string(ctx, -1));
	GREATEST_ASSERT(duk_get_global_string(ctx, "message"));
	GREATEST_ASSERT_STR_EQ("argument 1 (maxc) must be positive", duk_get_string(ctx, -1));

	GREATEST_PASS();
}

GREATEST_TEST
cut_string_negative_maxl(void)
{
	const int ret = duk_peval_string(ctx,
		"try {\n"
		"  lines = Irccd.Util.cut('hello world', undefined, -1);\n"
		"} catch (e) {\n"
		"  name = e.name;\n"
		"  message = e.message;\n"
		"}\n"
	);

	if (ret != 0)
		GREATEST_FAIL();

	GREATEST_ASSERT(duk_get_global_string(ctx, "name"));
	GREATEST_ASSERT_STR_EQ("RangeError", duk_get_string(ctx, -1));
	GREATEST_ASSERT(duk_get_global_string(ctx, "message"));
	GREATEST_ASSERT_STR_EQ("argument 2 (maxl) must be positive", duk_get_string(ctx, -1));

	GREATEST_PASS();
}

GREATEST_TEST
cut_array_simple(void)
{
	const int ret = duk_peval_string(ctx,
		"lines = Irccd.Util.cut([ 'hello', 'world' ]);\n"
		"line0 = lines[0];\n"
	);

	if (ret != 0)
		GREATEST_FAIL();

	GREATEST_ASSERT(duk_get_global_string(ctx, "line0"));
	GREATEST_ASSERT_STR_EQ("hello world", duk_get_string(ctx, -1));

	GREATEST_PASS();
}

GREATEST_TEST
cut_array_double(void)
{
	const int ret = duk_peval_string(ctx,
		"lines = Irccd.Util.cut([ 'hello', 'world' ], 5);\n"
		"line0 = lines[0];\n"
		"line1 = lines[1];\n"
	);

	if (ret != 0)
		GREATEST_FAIL();

	GREATEST_ASSERT(duk_get_global_string(ctx, "line0"));
	GREATEST_ASSERT_STR_EQ("hello", duk_get_string(ctx, -1));
	GREATEST_ASSERT(duk_get_global_string(ctx, "line1"));
	GREATEST_ASSERT_STR_EQ("world", duk_get_string(ctx, -1));

	GREATEST_PASS();
}

GREATEST_TEST
cut_array_dirty(void)
{
	const int ret = duk_peval_string(ctx,
		"lines = Irccd.Util.cut([ '   ', ' hello  ', '  world ', '	'], 5);\n"
		"line0 = lines[0];\n"
		"line1 = lines[1];\n"
	);

	if (ret != 0)
		GREATEST_FAIL();

	GREATEST_ASSERT(duk_get_global_string(ctx, "line0"));
	GREATEST_ASSERT_STR_EQ("hello", duk_get_string(ctx, -1));
	GREATEST_ASSERT(duk_get_global_string(ctx, "line1"));
	GREATEST_ASSERT_STR_EQ("world", duk_get_string(ctx, -1));

	GREATEST_PASS();
}

GREATEST_TEST
cut_invalid_data(void)
{
	const int ret = duk_peval_string(ctx,
		"try {\n"
		"  lines = Irccd.Util.cut(123);\n"
		"} catch (e) {\n"
		"  name = e.name;\n"
		"  message = e.message;\n"
		"}\n"
	);

	if (ret != 0)
		GREATEST_FAIL();

	GREATEST_ASSERT(duk_get_global_string(ctx, "name"));
	GREATEST_ASSERT_STR_EQ("TypeError", duk_get_string(ctx, -1));

	GREATEST_PASS();
}

GREATEST_SUITE(suite_cut)
{
	GREATEST_SET_SETUP_CB(setup, NULL);
	GREATEST_SET_TEARDOWN_CB(teardown, NULL);
	GREATEST_RUN_TEST(cut_string_simple);
	GREATEST_RUN_TEST(cut_string_double);
	GREATEST_RUN_TEST(cut_string_dirty);
	GREATEST_RUN_TEST(cut_string_too_much_lines);
	GREATEST_RUN_TEST(cut_string_token_too_big);
	GREATEST_RUN_TEST(cut_string_negative_maxc);
	GREATEST_RUN_TEST(cut_string_negative_maxl);
	GREATEST_RUN_TEST(cut_array_simple);
	GREATEST_RUN_TEST(cut_array_double);
	GREATEST_RUN_TEST(cut_array_dirty);
	GREATEST_RUN_TEST(cut_invalid_data);
}

GREATEST_MAIN_DEFS();

int
main(int argc, char **argv)
{
	GREATEST_MAIN_BEGIN();
	GREATEST_RUN_SUITE(suite_basics);
	GREATEST_RUN_SUITE(suite_cut);
	GREATEST_MAIN_END();

	return 0;
}
