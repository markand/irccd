/*
 * test-jsapi-unicode.c -- test Irccd.Unicode API
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

/*
 * /!\ Be sure that this file is kept saved in UTF-8 /!\
 */

#include <unity.h>

#include <irccd/config.h>
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
basics_is_letter(void)
{
	duk_peval_string_noresult(ctx, "result = Irccd.Unicode.isLetter(String('é').charCodeAt(0));");
	TEST_ASSERT(duk_get_global_string(ctx, "result"));
	TEST_ASSERT(duk_get_boolean(ctx, -1));

	duk_peval_string_noresult(ctx, "result = Irccd.Unicode.isLetter(String('€').charCodeAt(0));");
	TEST_ASSERT(duk_get_global_string(ctx, "result"));
	TEST_ASSERT(!duk_get_boolean(ctx, -1));
}

static void
basics_is_lower(void)
{
	duk_peval_string_noresult(ctx, "result = Irccd.Unicode.isLower(String('é').charCodeAt(0));");
	TEST_ASSERT(duk_get_global_string(ctx, "result"));
	TEST_ASSERT(duk_get_boolean(ctx, -1));

	duk_peval_string_noresult(ctx, "result = Irccd.Unicode.isLower(String('É').charCodeAt(0));");
	TEST_ASSERT(duk_get_global_string(ctx, "result"));
	TEST_ASSERT(!duk_get_boolean(ctx, -1));
}

static void
basics_is_upper(void)
{
	duk_peval_string_noresult(ctx, "result = Irccd.Unicode.isUpper(String('É').charCodeAt(0));");
	TEST_ASSERT(duk_get_global_string(ctx, "result"));
	TEST_ASSERT(duk_get_boolean(ctx, -1));

	duk_peval_string_noresult(ctx, "result = Irccd.Unicode.isUpper(String('é').charCodeAt(0));");
	TEST_ASSERT(duk_get_global_string(ctx, "result"));
	TEST_ASSERT(!duk_get_boolean(ctx, -1));
}

int
main(void)
{
	UNITY_BEGIN();

	RUN_TEST(basics_is_letter);
	RUN_TEST(basics_is_lower);
	RUN_TEST(basics_is_upper);

	return UNITY_END();
}
