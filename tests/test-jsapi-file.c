/*
 * test-jsapi-file.c -- test Irccd.File API
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

#include <sys/stat.h>
#include <stdio.h>

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

	duk_push_string(ctx, TOP);
	duk_put_global_string(ctx, "TOP");
}

void
tearDown(void)
{
	irc_plugin_finish(plugin);

	plugin = NULL;
	ctx = NULL;
}

static void
free_basename(void)
{
	if (duk_peval_string(ctx, "result = Irccd.File.basename('/usr/local/etc/irccd.conf');"))
		TEST_FAIL();

	TEST_ASSERT(duk_get_global_string(ctx, "result"));
	TEST_ASSERT_EQUAL_STRING("irccd.conf", duk_get_string(ctx, -1));
}

static void
free_dirname(void)
{
	if (duk_peval_string(ctx, "result = Irccd.File.dirname('/usr/local/etc/irccd.conf');"))
		TEST_FAIL();

	TEST_ASSERT(duk_get_global_string(ctx, "result"));
	TEST_ASSERT_EQUAL_STRING("/usr/local/etc", duk_get_string(ctx, -1));
}

static void
free_exists(void)
{
	if (duk_peval_string(ctx, "result = Irccd.File.exists(TOP + '/tests/data/root/file-1.txt')"))
		TEST_FAIL();

	TEST_ASSERT(duk_get_global_string(ctx, "result"));
	TEST_ASSERT(duk_get_boolean(ctx, -1));
}

static void
free_exists2(void)
{
	if (duk_peval_string(ctx, "result = Irccd.File.exists('file_which_does_not_exist.txt')"))
		TEST_FAIL();

	TEST_ASSERT(duk_get_global_string(ctx, "result"));
	TEST_ASSERT(!duk_get_boolean(ctx, -1));
}

static void
free_remove(void)
{
	FILE *fp;
	struct stat st;

	if (!(fp = fopen(TOP "/tests/test.bin", "w")))
		TEST_FAIL();

	fclose(fp);

	if (duk_peval_string(ctx, "Irccd.File.remove(TOP + '/tests/test.bin')") != 0)
		TEST_FAIL();

	TEST_ASSERT(stat(TOP "/tests/test.bin", &st) < 0);
}

static void
object_basename(void)
{
	const int ret = duk_peval_string(ctx,
		"f = new Irccd.File(TOP + '/tests/data/root/file-1.txt', 'r');"
		"result = f.basename();"
	);

	if (ret != 0)
		TEST_FAIL();

	TEST_ASSERT(duk_get_global_string(ctx, "result"));
	TEST_ASSERT_EQUAL_STRING("file-1.txt", duk_get_string(ctx, -1));
}

static void
object_basename_closed(void)
{
	const int ret = duk_peval_string(ctx,
		"f = new Irccd.File(TOP + '/tests/data/root/file-1.txt', 'r');"
		"f.close();"
		"result = f.basename();"
	);

	if (ret != 0)
		TEST_FAIL();

	TEST_ASSERT(duk_get_global_string(ctx, "result"));
	TEST_ASSERT_EQUAL_STRING("file-1.txt", duk_get_string(ctx, -1));
}

static void
object_dirname(void)
{
	const int ret = duk_peval_string(ctx,
		"f = new Irccd.File(TOP + '/tests/data/root/file-1.txt', 'r');"
		"result = f.dirname();"
	);

	if (ret != 0)
		TEST_FAIL();

	TEST_ASSERT(duk_get_global_string(ctx, "result"));
	TEST_ASSERT_EQUAL_STRING(TOP "/tests/data/root", duk_get_string(ctx, -1));
}

static void
object_dirname_closed(void)
{
	const int ret = duk_peval_string(ctx,
		"f = new Irccd.File(TOP + '/tests/data/root/file-1.txt', 'r');"
		"f.close();"
		"result = f.dirname();"
	);

	if (ret != 0)
		TEST_FAIL();

	TEST_ASSERT(duk_get_global_string(ctx, "result"));
	TEST_ASSERT_EQUAL_STRING(TOP "/tests/data/root", duk_get_string(ctx, -1));
}

static void
object_lines(void)
{
	const int ret = duk_peval_string(ctx,
		"result = new Irccd.File(TOP + '/tests/data/root/lines.txt', 'r').lines();"
	);

	if (ret != 0)
		TEST_FAIL();

	TEST_ASSERT(duk_get_global_string(ctx, "result"));
	TEST_ASSERT_EQUAL_INT(3, duk_get_length(ctx, -1));
	TEST_ASSERT(duk_get_prop_index(ctx, -1, 0));
	TEST_ASSERT_EQUAL_STRING("a", duk_get_string(ctx, -1));
	TEST_ASSERT(duk_get_prop_index(ctx, -2, 1));
	TEST_ASSERT_EQUAL_STRING("b", duk_get_string(ctx, -1));
	TEST_ASSERT(duk_get_prop_index(ctx, -3, 2));
	TEST_ASSERT_EQUAL_STRING("c", duk_get_string(ctx, -1));
}

static void
object_lines_closed(void)
{
	const int ret = duk_peval_string(ctx,
		"try {"
		"  f = new Irccd.File(TOP + '/tests/data/root/lines.txt', 'r');"
		"  f.close();"
		"  f.lines();"
		"} catch (e) {"
		"  name = e.name;"
		"}"
	);

	if (ret != 0)
		TEST_FAIL();

	TEST_ASSERT(duk_get_global_string(ctx, "name"));
	TEST_ASSERT_EQUAL_STRING("SystemError", duk_get_string(ctx, -1));
}

static void
object_seek1(void)
{
	const int ret = duk_peval_string(ctx,
		"f = new Irccd.File(TOP + '/tests/data/root/file-1.txt', 'r');"
		"f.seek(Irccd.File.SeekSet, 6);"
		"result = f.read(1);"
	);

	if (ret != 0)
		TEST_FAIL();

	TEST_ASSERT(duk_get_global_string(ctx, "result"));
	TEST_ASSERT_EQUAL_STRING(".", duk_get_string(ctx, -1));
}

static void
object_seek2(void)
{
	const int ret = duk_peval_string(ctx,
		"f = new Irccd.File(TOP + '/tests/data/root/file-1.txt', 'r');"
		"f.seek(Irccd.File.SeekSet, 2);"
		"f.seek(Irccd.File.SeekCur, 4);"
		"result = f.read(1);"
	);

	if (ret != 0)
		TEST_FAIL();

	TEST_ASSERT(duk_get_global_string(ctx, "result"));
	TEST_ASSERT_EQUAL_STRING(".", duk_get_string(ctx, -1));
}

static void
object_seek3(void)
{
	const int ret = duk_peval_string(ctx,
		"f = new Irccd.File(TOP + '/tests/data/root/file-1.txt', 'r');"
		"f.seek(Irccd.File.SeekEnd, -2);"
		"result = f.read(1);"
	);

	if (ret != 0)
		TEST_FAIL();

	TEST_ASSERT(duk_get_global_string(ctx, "result"));
	TEST_ASSERT_EQUAL_STRING("t", duk_get_string(ctx, -1));
}

static void
object_seek_closed(void)
{
	const int ret = duk_peval_string(ctx,
		"try {"
		"  f = new Irccd.File(TOP + '/tests/data/root/file-1.txt', 'r');"
		"  f.close();"
		"  f.seek(Irccd.File.SeekEnd, -2);"
		"} catch (e) {"
		"  name = e.name"
		"}"
	);

	if (ret != 0)
		TEST_FAIL();

	TEST_ASSERT(duk_get_global_string(ctx, "name"));
	TEST_ASSERT_EQUAL_STRING("SystemError", duk_get_string(ctx, -1));
}

static void
object_read(void)
{
	const int ret = duk_peval_string(ctx,
		"f = new Irccd.File(TOP + '/tests/data/root/file-1.txt', 'r');"
		"result = f.read();"
	);

	if (ret != 0)
		TEST_FAIL();

	TEST_ASSERT(duk_get_global_string(ctx, "result"));
	TEST_ASSERT_EQUAL_STRING("file-1.txt\n", duk_get_string(ctx, -1));
}

static void
object_read_closed(void)
{
	const int ret = duk_peval_string(ctx,
		"try {"
		"  f = new Irccd.File(TOP + '/tests/data/root/file-1.txt', 'r');"
		"  f.close();"
		"  f.read();"
		"} catch (e) {"
		"  name = e.name;"
		"}"
	);

	if (ret != 0)
		TEST_FAIL();

	TEST_ASSERT(duk_get_global_string(ctx, "name"));
	TEST_ASSERT_EQUAL_STRING("SystemError", duk_get_string(ctx, -1));
}

static void
object_readline(void)
{
	const int ret = duk_peval_string(ctx,
		"result = [];"
		"f = new Irccd.File(TOP + '/tests/data/root/lines.txt', 'r');"
		"for (var s; s = f.readline(); ) {"
		"  result.push(s);"
		"}"
	);

	if (ret != 0) {
		puts(duk_to_string(ctx, -1));
		TEST_FAIL();
	}

	TEST_ASSERT(duk_get_global_string(ctx, "result"));
	TEST_ASSERT_EQUAL_INT(3, duk_get_length(ctx, -1));
	TEST_ASSERT(duk_get_prop_index(ctx, -1, 0));
	TEST_ASSERT_EQUAL_STRING("a", duk_get_string(ctx, -1));
	TEST_ASSERT(duk_get_prop_index(ctx, -2, 1));
	TEST_ASSERT_EQUAL_STRING("b", duk_get_string(ctx, -1));
	TEST_ASSERT(duk_get_prop_index(ctx, -3, 2));
	TEST_ASSERT_EQUAL_STRING("c", duk_get_string(ctx, -1));
}

static void
object_readline_closed(void)
{
	const int ret = duk_peval_string(ctx,
		"try {"
		"  result = [];"
		"  f = new Irccd.File(TOP + '/tests/data/root/lines.txt', 'r');"
		"  f.close();"
		"  for (var s; s = f.readline(); ) {"
		"    result.push(s);"
		"  }"
		"} catch (e) {"
		"  name = e.name;"
		"}\n"

	);

	if (ret != 0)
		TEST_FAIL();

	TEST_ASSERT(duk_get_global_string(ctx, "result"));
	TEST_ASSERT_EQUAL_INT(0, duk_get_length(ctx, -1));
	TEST_ASSERT(duk_get_global_string(ctx, "name"));
	TEST_ASSERT_EQUAL_STRING("SystemError", duk_get_string(ctx, -1));
}

int
main(void)
{
	UNITY_BEGIN();

	RUN_TEST(free_basename);
	RUN_TEST(free_dirname);
	RUN_TEST(free_exists);
	RUN_TEST(free_exists2);
	RUN_TEST(free_remove);
	RUN_TEST(object_basename);
	RUN_TEST(object_basename_closed);
	RUN_TEST(object_dirname);
	RUN_TEST(object_dirname_closed);
	RUN_TEST(object_lines);
	RUN_TEST(object_lines_closed);
	RUN_TEST(object_seek1);
	RUN_TEST(object_seek2);
	RUN_TEST(object_seek3);
	RUN_TEST(object_seek_closed);
	RUN_TEST(object_read);
	RUN_TEST(object_read_closed);
	RUN_TEST(object_readline);
	RUN_TEST(object_readline_closed);

	return UNITY_END();
}
