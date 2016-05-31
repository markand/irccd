/*
 * main.cpp -- test Irccd.System API
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
#include <irccd/mod-irccd.hpp>
#include <irccd/mod-system.hpp>
#include <irccd/plugin-js.hpp>
#include <irccd/service-module.hpp>
#include <irccd/sysconfig.hpp>
#include <irccd/system.hpp>

using namespace irccd;

class TestJsSystem : public testing::Test {
protected:
	Irccd m_irccd;
	std::shared_ptr<JsPlugin> m_plugin;

	TestJsSystem()
		: m_plugin(std::make_shared<JsPlugin>("empty", SOURCEDIR "/empty.js"))
	{
		m_irccd.moduleService().get("Irccd")->load(m_irccd, *m_plugin);
		m_irccd.moduleService().get("Irccd.File")->load(m_irccd, *m_plugin);
		m_irccd.moduleService().get("Irccd.System")->load(m_irccd, *m_plugin);
	}
};

TEST_F(TestJsSystem, home)
{
	try {
		duk::pevalString(m_plugin->context(), "result = Irccd.System.home();");

		ASSERT_EQ(sys::home(), duk::getGlobal<std::string>(m_plugin->context(), "result"));
	} catch (const std::exception &ex) {
		FAIL() << ex.what();
	}
}

#if defined(HAVE_POPEN)

TEST_F(TestJsSystem, popen)
{
	try {
		auto ret = duk::pevalString(m_plugin->context(),
			"f = Irccd.System.popen(\"" IRCCD_EXECUTABLE " --version\", \"r\");"
			"r = f.readline();"
		);

		if (ret != 0)
			throw duk::exception(m_plugin->context(), -1);

		ASSERT_EQ(IRCCD_VERSION, duk::getGlobal<std::string>(m_plugin->context(), "r"));
	} catch (const std::exception &ex) {
		FAIL() << ex.what();
	}
}

#endif

int main(int argc, char **argv)
{
	testing::InitGoogleTest(&argc, argv);

	return RUN_ALL_TESTS();
}
