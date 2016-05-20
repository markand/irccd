/*
 * main.cpp -- test ask plugin
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
#include <irccd/server.hpp>
#include <irccd/service-plugin.hpp>

using namespace irccd;

class ServerTest : public Server {
private:
	std::string m_last;

public:
	inline ServerTest()
		: Server("test", ServerInfo())
	{
	}

	inline const std::string &last() const noexcept
	{
		return m_last;
	}

	void message(std::string target, std::string message) override
	{
		m_last = util::join({target, message});
	}
};

class AskTest : public testing::Test {
protected:
	Irccd m_irccd;
	PluginService &m_ps;

	std::shared_ptr<ServerTest> m_server;
	std::shared_ptr<Plugin> m_plugin;

public:
	AskTest()
		: m_ps(m_irccd.pluginService())
		, m_server(std::make_shared<ServerTest>())
	{
		m_ps.configure("ask", {{"file", SOURCEDIR "/answers.conf"}});
		m_ps.load("ask", PLUGINDIR "/ask.js");
		m_plugin = m_ps.require("ask");
	}
};

TEST_F(AskTest, basic)
{
	bool no = false;
	bool yes = false;

	// Invoke the plugin 1000 times, it will be very unlucky to not have both answers in that amount of tries.
	for (int i = 0; i < 1000; ++i) {
		m_plugin->onCommand(m_irccd, m_server, "tester", "#dummy", "");

		if (m_server->last() == "#dummy:tester, YES")
			yes = true;
		if (m_server->last() == "#dummy:tester, NO")
			no = true;
	}

	ASSERT_TRUE(no);
	ASSERT_TRUE(yes);
}

int main(int argc, char **argv)
{
	path::setApplicationPath(argv[0]);
	testing::InitGoogleTest(&argc, argv);

	return RUN_ALL_TESTS();
}
