/*
 * test-jsapi-irccd.c -- test Irccd API
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

#include <errno.h>

#define GREATEST_USE_ABBREVS 0
#include <greatest.h>

#include <irccd/config.h>
#include <irccd/js-plugin.h>
#include <irccd/jsapi-system.h>
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

static int
throw(duk_context *ctx)
{
	errno = EINVAL;
	jsapi_system_raise(ctx);

	return 0;
}

GREATEST_TEST
basics_version(void)
{
	const int ret = duk_peval_string(ctx,
		"major = Irccd.Version.Major;"
		"minor = Irccd.Version.Minor;"
		"patch = Irccd.Version.Match;"
	);

	if (ret != 0)
		GREATEST_FAIL();

	GREATEST_ASSERT(duk_get_global_string(ctx, "major"));
	GREATEST_ASSERT_EQ(IRCCD_VERSION_MAJOR, duk_get_int(ctx, -1));
	GREATEST_ASSERT(duk_get_global_string(ctx, "minor"));
	GREATEST_ASSERT_EQ(IRCCD_VERSION_MINOR, duk_get_int(ctx, -1));
	GREATEST_ASSERT(duk_get_global_string(ctx, "patch"));
	GREATEST_ASSERT_EQ(IRCCD_VERSION_PATCH, duk_get_int(ctx, -1));

	GREATEST_PASS();
}

GREATEST_TEST
basics_system_error_from_js(void)
{
	const int ret = duk_peval_string(ctx,
		"try {"
		"  throw new Irccd.SystemError(1, 'test');"
		"} catch (e) {"
		"  errno = e.errno;"
		"  name = e.name;"
		"  message = e.message;"
		"  v1 = (e instanceof Error);"
		"  v2 = (e instanceof Irccd.SystemError);"
		"}"
	);

	if (ret != 0)
		GREATEST_FAIL();

	GREATEST_ASSERT(duk_get_global_string(ctx, "errno"));
	GREATEST_ASSERT_EQ(1, duk_get_int(ctx, -1));
	GREATEST_ASSERT(duk_get_global_string(ctx, "name"));
	GREATEST_ASSERT_STR_EQ("SystemError", duk_get_string(ctx, -1));
	GREATEST_ASSERT(duk_get_global_string(ctx, "message"));
	GREATEST_ASSERT_STR_EQ("test", duk_get_string(ctx, -1));
	GREATEST_ASSERT(duk_get_global_string(ctx, "v1"));
	GREATEST_ASSERT(duk_get_boolean(ctx, -1));
	GREATEST_ASSERT(duk_get_global_string(ctx, "v2"));
	GREATEST_ASSERT(duk_get_boolean(ctx, -1));

	GREATEST_PASS();
}

GREATEST_TEST
basics_system_error_from_c(void)
{
	duk_push_c_function(ctx, throw, 0);
	duk_put_global_string(ctx, "f");

	const int ret = duk_peval_string(ctx,
		"try {"
		"  f();"
		"} catch (e) {"
		"  errno = e.errno;"
		"  name = e.name;"
		"  v1 = (e instanceof Error);"
		"  v2 = (e instanceof Irccd.SystemError);"
		"}"
	);

	if (ret != 0)
		GREATEST_FAIL();

	GREATEST_ASSERT(duk_get_global_string(ctx, "errno"));
	GREATEST_ASSERT_EQ(EINVAL, duk_get_int(ctx, -1));
	GREATEST_ASSERT(duk_get_global_string(ctx, "name"));
	GREATEST_ASSERT_STR_EQ("SystemError", duk_get_string(ctx, -1));
	GREATEST_ASSERT(duk_get_global_string(ctx, "v1"));
	GREATEST_ASSERT(duk_get_boolean(ctx, -1));
	GREATEST_ASSERT(duk_get_global_string(ctx, "v2"));
	GREATEST_ASSERT(duk_get_boolean(ctx, -1));

	GREATEST_PASS();
}

GREATEST_SUITE(suite_basics)
{
	GREATEST_SET_SETUP_CB(setup, NULL);
	GREATEST_SET_TEARDOWN_CB(teardown, NULL);
	GREATEST_RUN_TEST(basics_version);
	GREATEST_RUN_TEST(basics_system_error_from_js);
	GREATEST_RUN_TEST(basics_system_error_from_c);
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
