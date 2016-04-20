/*
 * main.cpp -- test Irccd API
 *
 * Copyright (c) 2013-2016 David Demelier <markand@malikania.fr>
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

#include <gtest/gtest.h>

#include <irccd/sysconfig.hpp>

#include <irccd/js-irccd.hpp>
#include <irccd/logger.hpp>

using namespace irccd;

TEST(TestJsIrccd, version)
{
	duk::Context ctx;

	loadJsIrccd(ctx);

	try {
		auto ret = duk::pevalString(ctx,
			"major = Irccd.version.major;"
			"minor = Irccd.version.minor;"
			"patch = Irccd.version.patch;"
		);

		if (ret != 0)
			throw duk::error(ctx, -1);

		ASSERT_EQ(IRCCD_VERSION_MAJOR, duk::getGlobal<int>(ctx, "major"));
		ASSERT_EQ(IRCCD_VERSION_MINOR, duk::getGlobal<int>(ctx, "minor"));
		ASSERT_EQ(IRCCD_VERSION_PATCH, duk::getGlobal<int>(ctx, "patch"));
	} catch (const std::exception &ex) {
		FAIL() << ex.what();
	}
}

TEST(SystemError, fromJavascript)
{
	duk::Context ctx;

	loadJsIrccd(ctx);

	try {
		auto ret = duk::pevalString(ctx,
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
			throw duk::error(ctx, -1);

		ASSERT_EQ(1, duk::getGlobal<int>(ctx, "errno"));
		ASSERT_EQ("SystemError", duk::getGlobal<std::string>(ctx, "name"));
		ASSERT_EQ("test", duk::getGlobal<std::string>(ctx, "message"));
		ASSERT_TRUE(duk::getGlobal<bool>(ctx, "v1"));
		ASSERT_TRUE(duk::getGlobal<bool>(ctx, "v2"));
	} catch (const std::exception &ex) {
		FAIL() << ex.what();
	}
}

TEST(SystemError, fromNative)
{
	duk::Context ctx;

	loadJsIrccd(ctx);

	try {
		duk::push(ctx, duk::Function{[] (duk::ContextPtr ctx) -> duk::Ret {
			duk::raise(ctx, SystemError{EINVAL, "hey"});

			return 0;
		}});

		duk::putGlobal(ctx, "f");

		auto ret = duk::pevalString(ctx,
			"try {"
			"  f();"
			"} catch (e) {"
			"  errno = e.errno;"
			"  name = e.name;"
			"  message = e.message;"
			"  v1 = (e instanceof Error);"
			"  v2 = (e instanceof Irccd.SystemError);"
			"}"
		);

		if (ret != 0)
			throw duk::error(ctx, -1);

		ASSERT_EQ(EINVAL, duk::getGlobal<int>(ctx, "errno"));
		ASSERT_EQ("SystemError", duk::getGlobal<std::string>(ctx, "name"));
		ASSERT_EQ("hey", duk::getGlobal<std::string>(ctx, "message"));
		ASSERT_TRUE(duk::getGlobal<bool>(ctx, "v1"));
		ASSERT_TRUE(duk::getGlobal<bool>(ctx, "v2"));
	} catch (const std::exception &ex) {
		FAIL() << ex.what();
	}
}

int main(int argc, char **argv)
{
	testing::InitGoogleTest(&argc, argv);

	return RUN_ALL_TESTS();
}
