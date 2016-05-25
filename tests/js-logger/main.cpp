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

#include <irccd/irccd.hpp>
#include <irccd/logger.hpp>
#include <irccd/mod-irccd.hpp>
#include <irccd/mod-logger.hpp>
#include <irccd/plugin-js.hpp>
#include <irccd/service-module.hpp>
#include <irccd/sysconfig.hpp>

using namespace irccd;

namespace {

std::string lineInfo;
std::string lineWarning;
std::string lineDebug;

} // !namespace

class LoggerIfaceTest : public log::Interface {
public:
	void info(const std::string &line) override
	{
		lineInfo = line;
	}

	void warning(const std::string &line) override
	{
		lineWarning = line;
	}

	void debug(const std::string &line) override
	{
		lineDebug = line;
	}
};

class TestJsLogger : public testing::Test {
protected:
	Irccd m_irccd;
	std::shared_ptr<JsPlugin> m_plugin;

	TestJsLogger()
		: m_plugin(std::make_shared<JsPlugin>("empty", SOURCEDIR "/empty.js"))
	{
		m_irccd.moduleService().get("Irccd")->load(m_irccd, *m_plugin);
		m_irccd.moduleService().get("Irccd.Logger")->load(m_irccd, *m_plugin);
	}
};

TEST_F(TestJsLogger, info)
{
	try {
		duk::putGlobal(m_plugin->context(), "\xff""\xff""name", "test");

		if (duk::pevalString(m_plugin->context(), "Irccd.Logger.info(\"hello!\");") != 0)
			throw duk::error(m_plugin->context(), -1);

		ASSERT_EQ("plugin test: hello!", lineInfo);
	} catch (const std::exception &ex) {
		FAIL() << ex.what();
	}
}

TEST_F(TestJsLogger, warning)
{
	try {
		duk::putGlobal(m_plugin->context(), "\xff""\xff""name", "test");

		if (duk::pevalString(m_plugin->context(), "Irccd.Logger.warning(\"FAIL!\");") != 0)
			throw duk::error(m_plugin->context(), -1);

		ASSERT_EQ("plugin test: FAIL!", lineWarning);
	} catch (const std::exception &ex) {
		FAIL() << ex.what();
	}
}

#if !defined(NDEBUG)

TEST_F(TestJsLogger, debug)
{
	try {
		duk::putGlobal(m_plugin->context(), "\xff""\xff""name", "test");

		if (duk::pevalString(m_plugin->context(), "Irccd.Logger.debug(\"starting\");") != 0)
			throw duk::error(m_plugin->context(), -1);

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
	log::setInterface(std::make_unique<LoggerIfaceTest>());

	return RUN_ALL_TESTS();
}
