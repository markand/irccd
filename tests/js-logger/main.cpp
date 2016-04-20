/*
 * main.cpp -- test Irccd.Logger API
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
#include <irccd/js-logger.hpp>
#include <irccd/logger.hpp>

using namespace irccd;

namespace {

std::string lineInfo;
std::string lineWarning;
std::string lineDebug;

} // !namespace

class TestLogger : public log::Interface {
public:
	void write(log::Level level, const std::string &line) noexcept override
	{
		switch (level) {
		case log::Level::Info:
			lineInfo = line;
			break;
		case log::Level::Warning:
			lineWarning = line;
			break;
		case log::Level::Debug:
			lineDebug = line;
			break;
		default:
			break;
		}
	}
};

TEST(TestJsLogger, info)
{
	duk::Context ctx;

	loadJsIrccd(ctx);
	loadJsLogger(ctx);

	try {
		duk::putGlobal(ctx, "\xff""\xff""name", "test");

		if (duk::pevalString(ctx, "Irccd.Logger.info(\"hello!\");") != 0)
			throw duk::error(ctx, -1);

		ASSERT_EQ("plugin test: hello!", lineInfo);
	} catch (const std::exception &ex) {
		FAIL() << ex.what();
	}
}

TEST(TestJsLogger, warning)
{
	duk::Context ctx;

	loadJsIrccd(ctx);
	loadJsLogger(ctx);

	try {
		duk::putGlobal(ctx, "\xff""\xff""name", "test");

		if (duk::pevalString(ctx, "Irccd.Logger.warning(\"FAIL!\");") != 0)
			throw duk::error(ctx, -1);

		ASSERT_EQ("plugin test: FAIL!", lineWarning);
	} catch (const std::exception &ex) {
		FAIL() << ex.what();
	}
}

#if !defined(NDEBUG)

TEST(TestJsLogger, debug)
{
	duk::Context ctx;

	loadJsIrccd(ctx);
	loadJsLogger(ctx);

	try {
		duk::putGlobal(ctx, "\xff""\xff""name", "test");

		if (duk::pevalString(ctx, "Irccd.Logger.debug(\"starting\");") != 0)
			throw duk::error(ctx, -1);

		ASSERT_EQ("plugin test: starting", lineDebug);
	} catch (const std::exception &ex) {
		FAIL() << ex.what();
	}
}

#endif

int main(int argc, char **argv)
{
	testing::InitGoogleTest(&argc, argv);

	log::setVerbose(true);
	log::setInterface(std::make_unique<TestLogger>());

	return RUN_ALL_TESTS();
}
