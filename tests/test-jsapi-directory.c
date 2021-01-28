/*
 * test-jsapi-directory.c -- test Irccd.Directory API
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

#define GREATEST_USE_ABBREVS 0
#include <greatest.h>

#include <irccd/js-plugin.h>
#include <irccd/plugin.h>

static struct irc_plugin *plugin;
static struct irc_js_plugin_data *data;

static void
setup(void *udata)
{
	(void)udata;

	plugin = irc_js_plugin_open(SOURCE "/data/example-plugin.js");
	data = plugin->data;

	duk_push_string(data->ctx, SOURCE);
	duk_put_global_string(data->ctx, "SOURCE");

	duk_push_string(data->ctx, BINARY);
	duk_put_global_string(data->ctx, "BINARY");
}

static void
teardown(void *udata)
{
	(void)udata;

	irc_plugin_finish(plugin);

	plugin = NULL;
	data = NULL;
}

GREATEST_TEST
object_constructor(void)
{
	const char *script =
		"d = new Irccd.Directory(SOURCE + '/data/root');"
		"p = d.path;"
		"l = d.entries.length;";

	if (duk_peval_string(data->ctx, script) != 0)
		GREATEST_FAIL();

	duk_get_global_string(data->ctx, "l");
	GREATEST_ASSERT_EQ(3U, duk_get_uint(data->ctx, -1));
	duk_get_global_string(data->ctx, "p");
	GREATEST_ASSERT(duk_is_string(data->ctx, -1));

	GREATEST_PASS();
}

GREATEST_TEST
object_find(void)
{
	const char *script = "d = new Irccd.Directory(SOURCE + '/data/root');";

	if (duk_peval_string(data->ctx, script) != 0)
		GREATEST_FAIL();

	/* Find "lines.txt" not recursively. */
	if (duk_peval_string(data->ctx, "p = d.find('lines.txt');") != 0)
		GREATEST_FAIL();

	duk_get_global_string(data->ctx, "p");
	GREATEST_ASSERT_STR_EQ(SOURCE "/data/root/lines.txt", duk_get_string(data->ctx, -1));

	/* Find "unknown.txt" not recursively (not found). */
	if (duk_peval_string(data->ctx, "p = d.find('unknown.txt');") != 0)
		GREATEST_FAIL();

	duk_get_global_string(data->ctx, "p");
	GREATEST_ASSERT(duk_is_null(data->ctx, -1));

	/* Find "file-2.txt" not recursively (exists but in sub directory). */
	if (duk_peval_string(data->ctx, "p = d.find('file-2.txt');") != 0)
		GREATEST_FAIL();

	duk_get_global_string(data->ctx, "p");
	GREATEST_ASSERT(duk_is_null(data->ctx, -1));

	/* Find "file-2.txt" recursively. */
	if (duk_peval_string(data->ctx, "p = d.find('file-2.txt', true);") != 0)
		GREATEST_FAIL();

	duk_get_global_string(data->ctx, "p");
	GREATEST_ASSERT_STR_EQ(SOURCE "/data/root/level-1/level-2/file-2.txt", duk_get_string(data->ctx, -1));

	GREATEST_PASS();
}

GREATEST_TEST
object_remove(void)
{
	struct stat st;

	/* First create an empty directory. */
	mkdir(BINARY "/empty", 0700);

	if (duk_peval_string(data->ctx, "d = new Irccd.Directory(BINARY + '/empty')") != 0)
		GREATEST_FAIL();

	/* Not recursive. */
	if (duk_peval_string(data->ctx, "d.remove()") != 0)
		GREATEST_FAIL();

	GREATEST_ASSERT_EQ(-1, stat(BINARY "/empty", &st));

	mkdir(BINARY "/notempty", 0700);
	mkdir(BINARY "/notempty/empty", 0700);

	if (duk_peval_string(data->ctx, "d = new Irccd.Directory(BINARY + '/notempty')") != 0)
		GREATEST_FAIL();

	/* Not recursive. */
	if (duk_peval_string(data->ctx, "d.remove(true)") != 0)
		GREATEST_FAIL();

	GREATEST_ASSERT_EQ(-1, stat(BINARY "/notempty", &st));

	GREATEST_PASS();
}

GREATEST_SUITE(suite_object)
{
	GREATEST_SET_SETUP_CB(setup, NULL);
	GREATEST_SET_TEARDOWN_CB(teardown, NULL);
	GREATEST_RUN_TEST(object_constructor);
	GREATEST_RUN_TEST(object_find);
	GREATEST_RUN_TEST(object_remove);
}

GREATEST_TEST
free_find(void)
{
	/* Find "lines.txt" not recursively. */
	if (duk_peval_string(data->ctx, "p = Irccd.Directory.find(SOURCE + '/data/root', 'lines.txt');") != 0) {
		puts(duk_to_string(data->ctx, -1));
		GREATEST_FAIL();
	}

	duk_get_global_string(data->ctx, "p");
	GREATEST_ASSERT_STR_EQ(SOURCE "/data/root/lines.txt", duk_get_string(data->ctx, -1));

	/* Find "unknown.txt" not recursively (not found). */
	if (duk_peval_string(data->ctx, "p = Irccd.Directory.find(SOURCE + '/data/root', 'unknown.txt');") != 0)
		GREATEST_FAIL();

	duk_get_global_string(data->ctx, "p");
	GREATEST_ASSERT(duk_is_null(data->ctx, -1));

	/* Find "file-2.txt" not recursively (exists but in sub directory). */
	if (duk_peval_string(data->ctx, "p = Irccd.Directory.find(SOURCE + '/data/root', 'file-2.txt');") != 0)
		GREATEST_FAIL();

	duk_get_global_string(data->ctx, "p");
	GREATEST_ASSERT(duk_is_null(data->ctx, -1));

	/* Find "file-2.txt" recursively. */
	if (duk_peval_string(data->ctx, "p = Irccd.Directory.find(SOURCE + '/data/root', 'file-2.txt', true);") != 0)
		GREATEST_FAIL();

	duk_get_global_string(data->ctx, "p");
	GREATEST_ASSERT_STR_EQ(SOURCE "/data/root/level-1/level-2/file-2.txt", duk_get_string(data->ctx, -1));

	GREATEST_PASS();
}

GREATEST_TEST
free_remove(void)
{
	struct stat st;

	/* First create an empty directory. */
	mkdir(BINARY "/empty", 0700);

	/* Not recursive. */
	if (duk_peval_string(data->ctx, "Irccd.Directory.remove(BINARY + '/empty')") != 0) {
		puts(duk_to_string(data->ctx, -1));
		GREATEST_FAIL();
	}

	GREATEST_ASSERT_EQ(-1, stat(BINARY "/empty", &st));

	mkdir(BINARY "/notempty", 0700);
	mkdir(BINARY "/notempty/empty", 0700);

	/* Not recursive. */
	if (duk_peval_string(data->ctx, "Irccd.Directory.remove(BINARY + '/notempty', true)") != 0)
		GREATEST_FAIL();

	GREATEST_ASSERT_EQ(-1, stat(BINARY "/notempty", &st));

	GREATEST_PASS();
}

GREATEST_TEST
free_mkdir(void)
{
	struct stat st;

	remove(BINARY "/1/2");
	remove(BINARY "/1");

	if (duk_peval_string(data->ctx, "Irccd.Directory.mkdir(BINARY + '/1/2')") != 0) {
		puts(duk_to_string(data->ctx, -1));
		GREATEST_FAIL();
	}

	GREATEST_ASSERT_EQ(0, stat(BINARY "/1/2", &st));

	GREATEST_PASS();
}

GREATEST_SUITE(suite_free)
{
	GREATEST_SET_SETUP_CB(setup, NULL);
	GREATEST_SET_TEARDOWN_CB(teardown, NULL);
	GREATEST_RUN_TEST(free_find);
	GREATEST_RUN_TEST(free_remove);
	GREATEST_RUN_TEST(free_mkdir);
}

GREATEST_MAIN_DEFS();

int
main(int argc, char **argv)
{
	GREATEST_MAIN_BEGIN();
	GREATEST_RUN_SUITE(suite_object);
	GREATEST_RUN_SUITE(suite_free);
	GREATEST_MAIN_END();

	return 0;
}
