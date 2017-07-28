/*
 * main.cpp -- test history plugin
 *
 * Copyright (c) 2013-2017 David Demelier <markand@malikania.fr>
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

#include <regex>

#include <gtest/gtest.h>

#include <irccd/irccd.hpp>
#include <irccd/server.hpp>
#include <irccd/service.hpp>
#include <irccd/path.hpp>

#include "plugin-tester.hpp"

using namespace irccd;

class ServerTest : public Server {
private:
    std::string m_last;

public:
    inline ServerTest()
        : Server("test")
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

class HistoryTest : public PluginTester {
protected:
    std::shared_ptr<ServerTest> m_server;
    std::shared_ptr<Plugin> m_plugin;

public:
    HistoryTest()
        : m_server(std::make_shared<ServerTest>())
    {
        m_irccd.plugins().setFormats("history", {
            { "error", "error=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}" },
            { "seen", "seen=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}:#{target}:%H:%M" },
            { "said", "said=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}:#{target}:#{message}:%H:%M" },
            { "unknown", "unknown=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}:#{target}" },
        });
    }

    void load(PluginConfig config = PluginConfig())
    {
        // Add file if not there.
        if (config.count("file") == 0)
            config.emplace("file", SOURCEDIR "/words.conf");

        m_irccd.plugins().setConfig("history", config);
        m_irccd.plugins().load("history", PLUGINDIR "/history.js");
        m_plugin = m_irccd.plugins().require("history");
    }
};

TEST_F(HistoryTest, formatError)
{
    load({{ "file", SOURCEDIR "/broken-conf.json" }});

    m_plugin->onCommand(m_irccd, MessageEvent{m_server, "jean!jean@localhost", "#history", "seen francis"});
    ASSERT_EQ("#history:error=history:!history:test:#history:jean!jean@localhost:jean", m_server->last());
}

TEST_F(HistoryTest, formatSeen)
{
    std::regex rule("#history:seen=history:!history:test:#history:destructor!dst@localhost:destructor:jean:\\d{2}:\\d{2}");

    remove(BINARYDIR "/seen.json");
    load({{ "file", BINARYDIR "/seen.json" }});

    m_plugin->onMessage(m_irccd, MessageEvent{m_server, "jean!jean@localhost", "#history", "hello"});
    m_plugin->onCommand(m_irccd, MessageEvent{m_server, "destructor!dst@localhost", "#history", "seen jean"});

    ASSERT_TRUE(std::regex_match(m_server->last(), rule));
}

TEST_F(HistoryTest, formatSaid)
{
    std::regex rule("#history:said=history:!history:test:#history:destructor!dst@localhost:destructor:jean:hello:\\d{2}:\\d{2}");

    remove(BINARYDIR "/said.json");
    load({{ "file", BINARYDIR "/said.json" }});

    m_plugin->onMessage(m_irccd, MessageEvent{m_server, "jean!jean@localhost", "#history", "hello"});
    m_plugin->onCommand(m_irccd, MessageEvent{m_server, "destructor!dst@localhost", "#history", "said jean"});

    ASSERT_TRUE(std::regex_match(m_server->last(), rule));
}

TEST_F(HistoryTest, formatUnknown)
{
    remove(BINARYDIR "/unknown.json");
    load({{ "file", BINARYDIR "/unknown.json" }});

    m_plugin->onMessage(m_irccd, MessageEvent{m_server, "jean!jean@localhost", "#history", "hello"});
    m_plugin->onCommand(m_irccd, MessageEvent{m_server, "destructor!dst@localhost", "#history", "seen nobody"});

    ASSERT_EQ("#history:unknown=history:!history:test:#history:destructor!dst@localhost:destructor:nobody", m_server->last());
}

TEST_F(HistoryTest, case_fix_642)
{
    std::regex rule("#history:said=history:!history:test:#history:destructor!dst@localhost:destructor:jean:hello:\\d{2}:\\d{2}");

    remove(BINARYDIR "/case.json");
    load({{"file", BINARYDIR "/case.json"}});

    m_plugin->onMessage(m_irccd, MessageEvent{m_server, "JeaN!JeaN@localhost", "#history", "hello"});

    m_plugin->onCommand(m_irccd, MessageEvent{m_server, "destructor!dst@localhost", "#HISTORY", "said JEAN"});
    ASSERT_TRUE(std::regex_match(m_server->last(), rule));
    m_plugin->onCommand(m_irccd, MessageEvent{m_server, "destructor!dst@localhost", "#HiSToRy", "said JeaN"});
    ASSERT_TRUE(std::regex_match(m_server->last(), rule));
}

int main(int argc, char **argv)
{
    path::setApplicationPath(argv[0]);
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
