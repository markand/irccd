/*
 * main.cpp -- test logger plugin
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

#include <fstream>
#include <iterator>

#include <gtest/gtest.h>

#include <irccd/irccd.hpp>
#include <irccd/logger.hpp>
#include <irccd/server.hpp>
#include <irccd/service.hpp>
#include <irccd/path.hpp>

#include "plugin-tester.hpp"

using namespace irccd;

class ServerTest : public Server {
public:
    inline ServerTest()
        : Server("test")
    {
    }
};

class LoggerTest : public PluginTester {
protected:
    std::shared_ptr<ServerTest> m_server;
    std::shared_ptr<Plugin> m_plugin;

    std::string last() const
    {
        std::ifstream file(BINARYDIR "/log.txt");

        return std::string(std::istreambuf_iterator<char>(file.rdbuf()), {});
    }

public:
    LoggerTest()
        : m_server(std::make_shared<ServerTest>())
    {
        remove(BINARYDIR "/log.txt");

        m_irccd.plugins().setFormats("logger", {
            { "cmode", "cmode=#{server}:#{channel}:#{origin}:#{nickname}:#{mode}:#{arg}" },
            { "cnotice", "cnotice=#{server}:#{channel}:#{origin}:#{nickname}:#{message}" },
            { "join", "join=#{server}:#{channel}:#{origin}:#{nickname}" },
            { "kick", "kick=#{server}:#{channel}:#{origin}:#{nickname}:#{target}:#{reason}" },
            { "me", "me=#{server}:#{channel}:#{origin}:#{nickname}:#{message}" },
            { "message", "message=#{server}:#{channel}:#{origin}:#{nickname}:#{message}" },
            { "mode", "mode=#{server}:#{origin}:#{nickname}:#{mode}:#{arg}" },
            { "notice", "notice=#{server}:#{origin}:#{nickname}:#{message}" },
            { "part", "part=#{server}:#{channel}:#{origin}:#{nickname}:#{reason}" },
            { "query", "query=#{server}:#{origin}:#{nickname}:#{message}" },
            { "topic", "topic=#{server}:#{channel}:#{origin}:#{nickname}:#{topic}" },
        });
    }

    void load(PluginConfig config = PluginConfig())
    {
        if (config.count("path") == 0)
            config.emplace("path", BINARYDIR "/log.txt");

        m_irccd.plugins().setConfig("logger", config);
        m_irccd.plugins().load("logger", PLUGINDIR "/logger.js");
        m_plugin = m_irccd.plugins().require("logger");
    }
};

TEST_F(LoggerTest, formatChannelMode)
{
    load();

    m_plugin->onChannelMode(m_irccd, ChannelModeEvent{m_server, "jean!jean@localhost", "#staff", "+o", "jean"});

    ASSERT_EQ("cmode=test:#staff:jean!jean@localhost:jean:+o:jean\n", last());
}

TEST_F(LoggerTest, formatChannelNotice)
{
    load();

    m_plugin->onChannelNotice(m_irccd, ChannelNoticeEvent{m_server, "jean!jean@localhost", "#staff", "bonjour!"});

    ASSERT_EQ("cnotice=test:#staff:jean!jean@localhost:jean:bonjour!\n", last());
}

TEST_F(LoggerTest, formatJoin)
{
    load();

    m_plugin->onJoin(m_irccd, JoinEvent{m_server, "jean!jean@localhost", "#staff"});

    ASSERT_EQ("join=test:#staff:jean!jean@localhost:jean\n", last());
}

TEST_F(LoggerTest, formatKick)
{
    load();

    m_plugin->onKick(m_irccd, KickEvent{m_server, "jean!jean@localhost", "#staff", "badboy", "please do not flood"});

    ASSERT_EQ("kick=test:#staff:jean!jean@localhost:jean:badboy:please do not flood\n", last());
}

TEST_F(LoggerTest, formatMe)
{
    load();

    m_plugin->onMe(m_irccd, MeEvent{m_server, "jean!jean@localhost", "#staff", "is drinking water"});

    ASSERT_EQ("me=test:#staff:jean!jean@localhost:jean:is drinking water\n", last());
}

TEST_F(LoggerTest, formatMessage)
{
    load();

    m_plugin->onMessage(m_irccd, MessageEvent{m_server, "jean!jean@localhost", "#staff", "hello guys"});

    ASSERT_EQ("message=test:#staff:jean!jean@localhost:jean:hello guys\n", last());
}

TEST_F(LoggerTest, formatMode)
{
    load();

    m_plugin->onMode(m_irccd, ModeEvent{m_server, "jean!jean@localhost", "+i"});

    ASSERT_EQ("mode=test:jean!jean@localhost:jean:+i:\n", last());
}

TEST_F(LoggerTest, formatNotice)
{
    load();

    m_plugin->onNotice(m_irccd, NoticeEvent{m_server, "jean!jean@localhost", "tu veux voir mon chat ?"});

    ASSERT_EQ("notice=test:jean!jean@localhost:jean:tu veux voir mon chat ?\n", last());
}

TEST_F(LoggerTest, formatPart)
{
    load();

    m_plugin->onPart(m_irccd, PartEvent{m_server, "jean!jean@localhost", "#staff", "too noisy here"});

    ASSERT_EQ("part=test:#staff:jean!jean@localhost:jean:too noisy here\n", last());
}

TEST_F(LoggerTest, formatQuery)
{
    load();

    m_plugin->onQuery(m_irccd, QueryEvent{m_server, "jean!jean@localhost", "much irccd, wow"});

    ASSERT_EQ("query=test:jean!jean@localhost:jean:much irccd, wow\n", last());
}

TEST_F(LoggerTest, formatTopic)
{
    load();

    m_plugin->onTopic(m_irccd, TopicEvent{m_server, "jean!jean@localhost", "#staff", "oh yeah yeaaaaaaaah"});

    ASSERT_EQ("topic=test:#staff:jean!jean@localhost:jean:oh yeah yeaaaaaaaah\n", last());
}

int main(int argc, char **argv)
{
    path::setApplicationPath(argv[0]);
    testing::InitGoogleTest(&argc, argv);
    log::setLogger(std::make_unique<log::SilentLogger>());

    return RUN_ALL_TESTS();
}
