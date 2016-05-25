/*
 * main.cpp -- test plugin plugin
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

#include <format.h>

#include <irccd/irccd.hpp>
#include <irccd/logger.hpp>
#include <irccd/server.hpp>
#include <irccd/service-plugin.hpp>

using namespace fmt::literals;

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

class FakePlugin : public Plugin {
public:
	FakePlugin()
		: Plugin("fake", "")
	{
		setAuthor("jean");
		setVersion("0.0.0.0.0.1");
		setLicense("BEER");
		setSummary("Fake White Beer 2000");
	}
};

class PluginTest : public testing::Test {
protected:
	Irccd m_irccd;
	PluginService &m_ps;

	std::shared_ptr<ServerTest> m_server;
	std::shared_ptr<Plugin> m_plugin;

public:
	PluginTest()
		: m_ps(m_irccd.pluginService())
		, m_server(std::make_shared<ServerTest>())
	{
		m_ps.add(std::make_shared<FakePlugin>());
		m_ps.setFormats("plugin", {
			{ "usage", "usage=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}" },
			{ "info", "info=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}:#{author}:#{license}:#{name}:#{summary}:#{version}" },
			{ "not-found", "not-found=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}:#{name}" },
			{ "too-long", "too-long=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}" }
		});
		m_ps.load("plugin", PLUGINDIR "/plugin.js");
		m_plugin = m_ps.require("plugin");
	}
};

TEST_F(PluginTest, formatUsage)
{
	m_plugin->onCommand(m_irccd, m_server, "jean!jean@localhost", "#staff", "");
	ASSERT_EQ("#staff:usage=plugin:!plugin:test:#staff:jean!jean@localhost:jean", m_server->last());

	m_plugin->onCommand(m_irccd, m_server, "jean!jean@localhost", "#staff", "fail");
	ASSERT_EQ("#staff:usage=plugin:!plugin:test:#staff:jean!jean@localhost:jean", m_server->last());

	m_plugin->onCommand(m_irccd, m_server, "jean!jean@localhost", "#staff", "info");
	ASSERT_EQ("#staff:usage=plugin:!plugin:test:#staff:jean!jean@localhost:jean", m_server->last());
}

TEST_F(PluginTest, formatInfo)
{
	m_plugin->onCommand(m_irccd, m_server, "jean!jean@localhost", "#staff", "info fake");

	ASSERT_EQ("#staff:info=plugin:!plugin:test:#staff:jean!jean@localhost:jean:jean:BEER:fake:Fake White Beer 2000:0.0.0.0.0.1", m_server->last());
}

TEST_F(PluginTest, formatNotFound)
{
	m_plugin->onCommand(m_irccd, m_server, "jean!jean@localhost", "#staff", "info doesnotexistsihope");

	ASSERT_EQ("#staff:not-found=plugin:!plugin:test:#staff:jean!jean@localhost:jean:doesnotexistsihope", m_server->last());
}

TEST_F(PluginTest, formatTooLong)
{
	for (int i = 0; i < 100; ++i)
		m_ps.add(std::make_shared<Plugin>("plugin-n-{}"_format(i), ""));

	m_plugin->onCommand(m_irccd, m_server, "jean!jean@localhost", "#staff", "list");

	ASSERT_EQ("#staff:too-long=plugin:!plugin:test:#staff:jean!jean@localhost:jean", m_server->last());
}

int main(int argc, char **argv)
{
	path::setApplicationPath(argv[0]);
	testing::InitGoogleTest(&argc, argv);
	log::setInterface(std::make_unique<log::Silent>());

	return RUN_ALL_TESTS();
}
