/*
 * test-jsapi-file.c -- test Irccd.File API
 *
 * Copyright (c) 2013-2021 David Demelier <markand@malikania.fr>
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

	plugin = js_plugin_open("example", SOURCE "/data/example-plugin.js");
	ctx = js_plugin_get_context(plugin);

	duk_push_string(ctx, SOURCE);
	duk_put_global_string(ctx, "SOURCE");

	duk_push_string(ctx, BINARY);
	duk_put_global_string(ctx, "BINARY");
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
free_basename(void)
{
	if (duk_peval_string(ctx, "result = Irccd.File.basename('/usr/local/etc/irccd.conf');"))
		GREATEST_FAIL();

	GREATEST_ASSERT(duk_get_global_string(ctx, "result"));
	GREATEST_ASSERT_STR_EQ("irccd.conf", duk_get_string(ctx, -1));

	GREATEST_PASS();
}

GREATEST_TEST
free_dirname(void)
{
	if (duk_peval_string(ctx, "result = Irccd.File.dirname('/usr/local/etc/irccd.conf');"))
		GREATEST_FAIL();

	GREATEST_ASSERT(duk_get_global_string(ctx, "result"));
	GREATEST_ASSERT_STR_EQ("/usr/local/etc", duk_get_string(ctx, -1));
	
	GREATEST_PASS();
}

GREATEST_TEST
free_exists(void)
{
	if (duk_peval_string(ctx, "result = Irccd.File.exists(SOURCE + '/data/root/file-1.txt')"))
		GREATEST_FAIL();

	GREATEST_ASSERT(duk_get_global_string(ctx, "result"));
	GREATEST_ASSERT(duk_get_boolean(ctx, -1));

	GREATEST_PASS();
}

GREATEST_TEST
free_exists2(void)
{
	if (duk_peval_string(ctx, "result = Irccd.File.exists('file_which_does_not_exist.txt')"))
		GREATEST_FAIL();

	GREATEST_ASSERT(duk_get_global_string(ctx, "result"));
	GREATEST_ASSERT(!duk_get_boolean(ctx, -1));

	GREATEST_PASS();
}

GREATEST_TEST
free_remove(void)
{
	FILE *fp;
	struct stat st;

	if (!(fp = fopen(BINARY "/test.bin", "w")))
		GREATEST_FAIL();

	fclose(fp);

	if (duk_peval_string(ctx, "Irccd.File.remove(BINARY + '/test.bin')") != 0)
		GREATEST_FAIL();

	GREATEST_ASSERT(stat(BINARY "/test.bin", &st) < 0);

	GREATEST_PASS();
}

GREATEST_SUITE(suite_free)
{
	GREATEST_SET_SETUP_CB(setup, NULL);
	GREATEST_SET_TEARDOWN_CB(teardown, NULL);
	GREATEST_RUN_TEST(free_basename);
	GREATEST_RUN_TEST(free_dirname);
	GREATEST_RUN_TEST(free_exists);
	GREATEST_RUN_TEST(free_exists2);
	GREATEST_RUN_TEST(free_remove);
}

GREATEST_TEST
object_basename(void)
{
	const int ret = duk_peval_string(ctx,
		"f = new Irccd.File(SOURCE + '/data/root/file-1.txt', 'r');"
		"result = f.basename();"
	);

	if (ret != 0)
		GREATEST_FAIL();

	GREATEST_ASSERT(duk_get_global_string(ctx, "result"));
	GREATEST_ASSERT_STR_EQ("file-1.txt", duk_get_string(ctx, -1));

	GREATEST_PASS();
}

GREATEST_TEST
object_basename_closed(void)
{
	const int ret = duk_peval_string(ctx,
		"f = new Irccd.File(SOURCE + '/data/root/file-1.txt', 'r');"
		"f.close();"
		"result = f.basename();"
	);

	if (ret != 0)
		GREATEST_FAIL();

	GREATEST_ASSERT(duk_get_global_string(ctx, "result"));
	GREATEST_ASSERT_STR_EQ("file-1.txt", duk_get_string(ctx, -1));

	GREATEST_PASS();
}

GREATEST_TEST
object_dirname(void)
{
	const int ret = duk_peval_string(ctx,
		"f = new Irccd.File(SOURCE + '/data/root/file-1.txt', 'r');"
		"result = f.dirname();"
	);

	if (ret != 0)
		GREATEST_FAIL();

	GREATEST_ASSERT(duk_get_global_string(ctx, "result"));
	GREATEST_ASSERT_STR_EQ(SOURCE "/data/root", duk_get_string(ctx, -1));

	GREATEST_PASS();
}

GREATEST_TEST
object_dirname_closed(void)
{
	const int ret = duk_peval_string(ctx,
		"f = new Irccd.File(SOURCE + '/data/root/file-1.txt', 'r');"
		"f.close();"
		"result = f.dirname();"
	);

	if (ret != 0)
		GREATEST_FAIL();

	GREATEST_ASSERT(duk_get_global_string(ctx, "result"));
	GREATEST_ASSERT_STR_EQ(SOURCE "/data/root", duk_get_string(ctx, -1));

	GREATEST_PASS();
}

GREATEST_TEST
object_lines(void)
{
	const int ret = duk_peval_string(ctx,
		"result = new Irccd.File(SOURCE + '/data/root/lines.txt', 'r').lines();"
	);

	if (ret != 0)
		GREATEST_FAIL();

	GREATEST_ASSERT(duk_get_global_string(ctx, "result"));
	GREATEST_ASSERT_EQ(3, duk_get_length(ctx, -1));
	GREATEST_ASSERT(duk_get_prop_index(ctx, -1, 0));
	GREATEST_ASSERT_STR_EQ("a", duk_get_string(ctx, -1));
	GREATEST_ASSERT(duk_get_prop_index(ctx, -2, 1));
	GREATEST_ASSERT_STR_EQ("b", duk_get_string(ctx, -1));
	GREATEST_ASSERT(duk_get_prop_index(ctx, -3, 2));
	GREATEST_ASSERT_STR_EQ("c", duk_get_string(ctx, -1));

	GREATEST_PASS();
}

GREATEST_TEST
object_lines_closed(void)
{
	const int ret = duk_peval_string(ctx,
		"try {"
		"  f = new Irccd.File(SOURCE + '/data/root/lines.txt', 'r');"
		"  f.close();"
		"  f.lines();"
		"} catch (e) {"
		"  name = e.name;"
		"}"
	);

	if (ret != 0)
		GREATEST_FAIL();

	GREATEST_ASSERT(duk_get_global_string(ctx, "name"));
	GREATEST_ASSERT_STR_EQ("SystemError", duk_get_string(ctx, -1));

	GREATEST_PASS();
}

GREATEST_TEST
object_seek1(void)
{
	const int ret = duk_peval_string(ctx,
		"f = new Irccd.File(SOURCE + '/data/root/file-1.txt', 'r');"
		"f.seek(Irccd.File.SeekSet, 6);"
		"result = f.read(1);"
	);

	if (ret != 0)
		GREATEST_FAIL();

	GREATEST_ASSERT(duk_get_global_string(ctx, "result"));
	GREATEST_ASSERT_STR_EQ(".", duk_get_string(ctx, -1));

	GREATEST_PASS();
}

GREATEST_TEST
object_seek2(void)
{
	const int ret = duk_peval_string(ctx,
		"f = new Irccd.File(SOURCE + '/data/root/file-1.txt', 'r');"
		"f.seek(Irccd.File.SeekSet, 2);"
		"f.seek(Irccd.File.SeekCur, 4);"
		"result = f.read(1);"
	);

	if (ret != 0)
		GREATEST_FAIL();

	GREATEST_ASSERT(duk_get_global_string(ctx, "result"));
	GREATEST_ASSERT_STR_EQ(".", duk_get_string(ctx, -1));

	GREATEST_PASS();
}

GREATEST_TEST
object_seek3(void)
{
	const int ret = duk_peval_string(ctx,
		"f = new Irccd.File(SOURCE + '/data/root/file-1.txt', 'r');"
		"f.seek(Irccd.File.SeekEnd, -2);"
		"result = f.read(1);"
	);

	if (ret != 0)
		GREATEST_FAIL();

	GREATEST_ASSERT(duk_get_global_string(ctx, "result"));
	GREATEST_ASSERT_STR_EQ("t", duk_get_string(ctx, -1));

	GREATEST_PASS();
}

GREATEST_TEST
object_seek_closed(void)
{
	const int ret = duk_peval_string(ctx,
		"try {"
		"  f = new Irccd.File(SOURCE + '/data/root/file-1.txt', 'r');"
		"  f.close();"
		"  f.seek(Irccd.File.SeekEnd, -2);"
		"} catch (e) {"
		"  name = e.name"
		"}"
	);

	if (ret != 0)
		GREATEST_FAIL();

	GREATEST_ASSERT(duk_get_global_string(ctx, "name"));
	GREATEST_ASSERT_STR_EQ("SystemError", duk_get_string(ctx, -1));

	GREATEST_PASS();
}

GREATEST_TEST
object_read(void)
{
	const int ret = duk_peval_string(ctx,
		"f = new Irccd.File(SOURCE + '/data/root/file-1.txt', 'r');"
		"result = f.read();"
	);

	if (ret != 0)
		GREATEST_FAIL();

	GREATEST_ASSERT(duk_get_global_string(ctx, "result"));
	GREATEST_ASSERT_STR_EQ("file-1.txt\n", duk_get_string(ctx, -1));

	GREATEST_PASS();
}

GREATEST_TEST
object_read_closed(void)
{
	const int ret = duk_peval_string(ctx,
		"try {"
		"  f = new Irccd.File(SOURCE + '/data/root/file-1.txt', 'r');"
		"  f.close();"
		"  f.read();"
		"} catch (e) {"
		"  name = e.name;"
		"}"
	);

	if (ret != 0)
		GREATEST_FAIL();

	GREATEST_ASSERT(duk_get_global_string(ctx, "name"));
	GREATEST_ASSERT_STR_EQ("SystemError", duk_get_string(ctx, -1));

	GREATEST_PASS();
}

GREATEST_TEST
object_readline(void)
{
	const int ret = duk_peval_string(ctx,
		"result = [];"
		"f = new Irccd.File(SOURCE + '/data/root/lines.txt', 'r');"
		"for (var s; s = f.readline(); ) {"
		"  result.push(s);"
		"}"
	);

	if (ret != 0) {
		puts(duk_to_string(ctx, -1));
		GREATEST_FAIL();
	}

	GREATEST_ASSERT(duk_get_global_string(ctx, "result"));
	GREATEST_ASSERT_EQ(3, duk_get_length(ctx, -1));
	GREATEST_ASSERT(duk_get_prop_index(ctx, -1, 0));
	GREATEST_ASSERT_STR_EQ("a", duk_get_string(ctx, -1));
	GREATEST_ASSERT(duk_get_prop_index(ctx, -2, 1));
	GREATEST_ASSERT_STR_EQ("b", duk_get_string(ctx, -1));
	GREATEST_ASSERT(duk_get_prop_index(ctx, -3, 2));
	GREATEST_ASSERT_STR_EQ("c", duk_get_string(ctx, -1));

	GREATEST_PASS();
}

GREATEST_TEST
object_readline_closed(void)
{
	const int ret = duk_peval_string(ctx,
		"try {"
		"  result = [];"
		"  f = new Irccd.File(SOURCE + '/data/root/lines.txt', 'r');"
		"  f.close();"
		"  for (var s; s = f.readline(); ) {"
		"    result.push(s);"
		"  }"
		"} catch (e) {"
		"  name = e.name;"
		"}\n"

	);

	if (ret != 0)
		GREATEST_FAIL();

	GREATEST_ASSERT(duk_get_global_string(ctx, "result"));
	GREATEST_ASSERT_EQ(0, duk_get_length(ctx, -1));
	GREATEST_ASSERT(duk_get_global_string(ctx, "name"));
	GREATEST_ASSERT_STR_EQ("SystemError", duk_get_string(ctx, -1));

	GREATEST_PASS();
}

GREATEST_SUITE(suite_object)
{
	GREATEST_SET_SETUP_CB(setup, NULL);
	GREATEST_SET_TEARDOWN_CB(teardown, NULL);
	GREATEST_RUN_TEST(object_basename);
	GREATEST_RUN_TEST(object_basename_closed);
	GREATEST_RUN_TEST(object_dirname);
	GREATEST_RUN_TEST(object_dirname_closed);
	GREATEST_RUN_TEST(object_lines);
	GREATEST_RUN_TEST(object_lines_closed);
	GREATEST_RUN_TEST(object_seek1);
	GREATEST_RUN_TEST(object_seek2);
	GREATEST_RUN_TEST(object_seek3);
	GREATEST_RUN_TEST(object_seek_closed);
	GREATEST_RUN_TEST(object_read);
	GREATEST_RUN_TEST(object_read_closed);
	GREATEST_RUN_TEST(object_readline);
	GREATEST_RUN_TEST(object_readline_closed);
}

GREATEST_MAIN_DEFS();

int
main(int argc, char **argv)
{
	GREATEST_MAIN_BEGIN();
	GREATEST_RUN_SUITE(suite_free);
	GREATEST_RUN_SUITE(suite_object);
	GREATEST_MAIN_END();

	return 0;
}
