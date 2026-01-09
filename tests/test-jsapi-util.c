/*
 * test-jsapi-util.c -- test Irccd.Util API
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

#include <irccd/js-plugin.h>
#include <irccd/plugin.h>

static struct irc_plugin *plugin;
static duk_context *ctx;

void
setUp(void)
{
	plugin = js_plugin_open("example", TOP "/tests/data/example-plugin.js");
	ctx = js_plugin_get_context(plugin);
}

void
tearDown(void)
{
	irc_plugin_finish(plugin);

	plugin = NULL;
	ctx = NULL;
}

static void
basics_splituser(void)
{
	if (duk_peval_string(ctx, "result = Irccd.Util.splituser(\"user!~user@hyper/super/host\");") != 0)
		TEST_FAIL();

	TEST_ASSERT(duk_get_global_string(ctx, "result"));
	TEST_ASSERT_EQUAL_STRING("user", duk_get_string(ctx, -1));
}

static void
basics_splithost(void)
{
	if (duk_peval_string(ctx, "result = Irccd.Util.splithost(\"user!~user@hyper/super/host\");") != 0)
		TEST_FAIL();

	TEST_ASSERT(duk_get_global_string(ctx, "result"));
	TEST_ASSERT_EQUAL_STRING("hyper/super/host", duk_get_string(ctx, -1));
}

static void
format_simple(void)
{
	const int ret = duk_peval_string(ctx,
		"result = Irccd.Util.format(\"#{target}\", { target: \"markand\" })"
	);

	if (ret != 0)
		TEST_FAIL();

	TEST_ASSERT(duk_get_global_string(ctx, "result"));
	TEST_ASSERT_EQUAL_STRING("markand", duk_get_string(ctx, -1));
}

static void
cut_string_simple(void)
{
	const int ret = duk_peval_string(ctx,
		"lines = Irccd.Util.cut('hello world');\n"
		"line0 = lines[0];\n"
	);

	if (ret != 0)
		TEST_FAIL();

	TEST_ASSERT(duk_get_global_string(ctx, "line0"));
	TEST_ASSERT_EQUAL_STRING("hello world", duk_get_string(ctx, -1));
}

static void
cut_string_double(void)
{
	const int ret = duk_peval_string(ctx,
		"lines = Irccd.Util.cut('hello world', 5);\n"
		"line0 = lines[0];\n"
		"line1 = lines[1];\n"
	);

	if (ret != 0)
		TEST_FAIL();

	TEST_ASSERT(duk_get_global_string(ctx, "line0"));
	TEST_ASSERT_EQUAL_STRING("hello", duk_get_string(ctx, -1));
	TEST_ASSERT(duk_get_global_string(ctx, "line1"));
	TEST_ASSERT_EQUAL_STRING("world", duk_get_string(ctx, -1));

}

static void
cut_string_dirty(void)
{
	const int ret = duk_peval_string(ctx,
		"lines = Irccd.Util.cut('	 hello	world	 ', 5);\n"
		"line0 = lines[0];\n"
		"line1 = lines[1];\n"
	);

	if (ret != 0)
		TEST_FAIL();

	TEST_ASSERT(duk_get_global_string(ctx, "line0"));
	TEST_ASSERT_EQUAL_STRING("hello", duk_get_string(ctx, -1));
	TEST_ASSERT(duk_get_global_string(ctx, "line1"));
	TEST_ASSERT_EQUAL_STRING("world", duk_get_string(ctx, -1));
}

static void
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
		TEST_FAIL();

	TEST_ASSERT(duk_get_global_string(ctx, "name"));
	TEST_ASSERT_EQUAL_STRING("RangeError", duk_get_string(ctx, -1));
	TEST_ASSERT(duk_get_global_string(ctx, "message"));
	TEST_ASSERT_EQUAL_STRING("number of lines exceeds maxl (3)", duk_get_string(ctx, -1));
}

static void
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
		TEST_FAIL();

	TEST_ASSERT(duk_get_global_string(ctx, "name"));
	TEST_ASSERT_EQUAL_STRING("RangeError", duk_get_string(ctx, -1));
	TEST_ASSERT(duk_get_global_string(ctx, "message"));
	TEST_ASSERT_EQUAL_STRING("token 'hello' could not fit in maxc limit (3)", duk_get_string(ctx, -1));
}

static void
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
		TEST_FAIL();

	TEST_ASSERT(duk_get_global_string(ctx, "name"));
	TEST_ASSERT_EQUAL_STRING("RangeError", duk_get_string(ctx, -1));
	TEST_ASSERT(duk_get_global_string(ctx, "message"));
	TEST_ASSERT_EQUAL_STRING("argument 1 (maxc) must be positive", duk_get_string(ctx, -1));
}

static void
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
		TEST_FAIL();

	TEST_ASSERT(duk_get_global_string(ctx, "name"));
	TEST_ASSERT_EQUAL_STRING("RangeError", duk_get_string(ctx, -1));
	TEST_ASSERT(duk_get_global_string(ctx, "message"));
	TEST_ASSERT_EQUAL_STRING("argument 2 (maxl) must be positive", duk_get_string(ctx, -1));
}

static void
cut_array_simple(void)
{
	const int ret = duk_peval_string(ctx,
		"lines = Irccd.Util.cut([ 'hello', 'world' ]);\n"
		"line0 = lines[0];\n"
	);

	if (ret != 0)
		TEST_FAIL();

	TEST_ASSERT(duk_get_global_string(ctx, "line0"));
	TEST_ASSERT_EQUAL_STRING("hello world", duk_get_string(ctx, -1));
}

static void
cut_array_double(void)
{
	const int ret = duk_peval_string(ctx,
		"lines = Irccd.Util.cut([ 'hello', 'world' ], 5);\n"
		"line0 = lines[0];\n"
		"line1 = lines[1];\n"
	);

	if (ret != 0)
		TEST_FAIL();

	TEST_ASSERT(duk_get_global_string(ctx, "line0"));
	TEST_ASSERT_EQUAL_STRING("hello", duk_get_string(ctx, -1));
	TEST_ASSERT(duk_get_global_string(ctx, "line1"));
	TEST_ASSERT_EQUAL_STRING("world", duk_get_string(ctx, -1));
}

static void
cut_array_dirty(void)
{
	const int ret = duk_peval_string(ctx,
		"lines = Irccd.Util.cut([ '   ', ' hello  ', '  world ', '	'], 5);\n"
		"line0 = lines[0];\n"
		"line1 = lines[1];\n"
	);

	if (ret != 0)
		TEST_FAIL();

	TEST_ASSERT(duk_get_global_string(ctx, "line0"));
	TEST_ASSERT_EQUAL_STRING("hello", duk_get_string(ctx, -1));
	TEST_ASSERT(duk_get_global_string(ctx, "line1"));
	TEST_ASSERT_EQUAL_STRING("world", duk_get_string(ctx, -1));
}

static void
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
		TEST_FAIL();

	TEST_ASSERT(duk_get_global_string(ctx, "name"));
	TEST_ASSERT_EQUAL_STRING("TypeError", duk_get_string(ctx, -1));
}

int
main(void)
{
	UNITY_BEGIN();

	RUN_TEST(basics_splituser);
	RUN_TEST(basics_splithost);

	RUN_TEST(format_simple);

	RUN_TEST(cut_string_simple);
	RUN_TEST(cut_string_double);
	RUN_TEST(cut_string_dirty);
	RUN_TEST(cut_string_too_much_lines);
	RUN_TEST(cut_string_token_too_big);
	RUN_TEST(cut_string_negative_maxc);
	RUN_TEST(cut_string_negative_maxl);
	RUN_TEST(cut_array_simple);
	RUN_TEST(cut_array_double);
	RUN_TEST(cut_array_dirty);
	RUN_TEST(cut_invalid_data);

	return UNITY_END();
}
