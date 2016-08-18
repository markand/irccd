/*
 * main.cpp -- test hangman plugin
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

class HangmanTest : public testing::Test {
protected:
    Irccd m_irccd;
    PluginService &m_ps;

    std::shared_ptr<ServerTest> m_server;
    std::shared_ptr<Plugin> m_plugin;

public:
    HangmanTest()
        : m_ps(m_irccd.plugins())
        , m_server(std::make_shared<ServerTest>())
    {
        m_ps.setFormats("hangman", {
            { "asked", "asked=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}:#{letter}" },
            { "dead", "dead=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}:#{word}" },
            { "found", "found=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}:#{word}" },
            { "start", "start=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}:#{word}" },
            { "win", "win=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}:#{word}" },
            { "wrong-letter", "wrong-letter=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}:#{letter}" },
            { "wrong-player", "wrong-player=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}:#{letter}" },
            { "wrong-word", "wrong-word=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}:#{word}" }
        });
    }

    void load(PluginConfig config = PluginConfig())
    {
        // Add file if not there.
        if (config.count("file") == 0)
            config.emplace("file", SOURCEDIR "/words.conf");

        m_ps.setConfig("hangman", config);
        m_ps.load("hangman", PLUGINDIR "/hangman.js");
        m_plugin = m_ps.require("hangman");
    }
};

TEST_F(HangmanTest, asked)
{
    load({{ "collaborative", "false" }});

    m_plugin->onCommand(m_irccd, MessageEvent{m_server, "jean!jean@localhost", "#hangman", ""});
    m_plugin->onMessage(m_irccd, MessageEvent{m_server, "jean!jean@localhost", "#hangman", "s"});
    m_plugin->onMessage(m_irccd, MessageEvent{m_server, "jean!jean@localhost", "#hangman", "s"});

    ASSERT_EQ("#hangman:asked=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:s", m_server->last());
}

TEST_F(HangmanTest, dead)
{
    load({{ "collaborative", "false" }});

    m_plugin->onCommand(m_irccd, MessageEvent{m_server, "jean!jean@localhost", "#hangman", ""});
    m_plugin->onMessage(m_irccd, MessageEvent{m_server, "jean!jean@localhost", "#hangman", "a"});
    m_plugin->onMessage(m_irccd, MessageEvent{m_server, "jean!jean@localhost", "#hangman", "b"});
    m_plugin->onMessage(m_irccd, MessageEvent{m_server, "jean!jean@localhost", "#hangman", "c"});
    m_plugin->onMessage(m_irccd, MessageEvent{m_server, "jean!jean@localhost", "#hangman", "d"});
    m_plugin->onMessage(m_irccd, MessageEvent{m_server, "jean!jean@localhost", "#hangman", "e"});
    m_plugin->onMessage(m_irccd, MessageEvent{m_server, "jean!jean@localhost", "#hangman", "f"});
    m_plugin->onMessage(m_irccd, MessageEvent{m_server, "jean!jean@localhost", "#hangman", "g"});
    m_plugin->onMessage(m_irccd, MessageEvent{m_server, "jean!jean@localhost", "#hangman", "h"});
    m_plugin->onMessage(m_irccd, MessageEvent{m_server, "jean!jean@localhost", "#hangman", "i"});
    m_plugin->onMessage(m_irccd, MessageEvent{m_server, "jean!jean@localhost", "#hangman", "j"});

    ASSERT_EQ("#hangman:dead=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:sky", m_server->last());
}

TEST_F(HangmanTest, found)
{
    load({{ "collaborative", "false" }});

    m_plugin->onCommand(m_irccd, MessageEvent{m_server, "jean!jean@localhost", "#hangman", ""});
    m_plugin->onMessage(m_irccd, MessageEvent{m_server, "jean!jean@localhost", "#hangman", "s"});

    ASSERT_EQ("#hangman:found=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:s _ _", m_server->last());
}

TEST_F(HangmanTest, start)
{
    load();

    m_plugin->onCommand(m_irccd, MessageEvent{m_server, "jean!jean@localhost", "#hangman", ""});

    ASSERT_EQ("#hangman:start=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:_ _ _", m_server->last());
}

TEST_F(HangmanTest, win1)
{
    load({{ "collaborative", "false" }});

    m_plugin->onCommand(m_irccd, MessageEvent{m_server, "jean!jean@localhost", "#hangman", ""});
    m_plugin->onMessage(m_irccd, MessageEvent{m_server, "jean!jean@localhost", "#hangman", "s"});
    m_plugin->onMessage(m_irccd, MessageEvent{m_server, "jean!jean@localhost", "#hangman", "k"});
    m_plugin->onMessage(m_irccd, MessageEvent{m_server, "jean!jean@localhost", "#hangman", "y"});

    ASSERT_EQ("#hangman:win=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:sky", m_server->last());
}

TEST_F(HangmanTest, win2)
{
    load({{ "collaborative", "false" }});

    m_plugin->onCommand(m_irccd, MessageEvent{m_server, "jean!jean@localhost", "#hangman", ""});
    m_plugin->onCommand(m_irccd, MessageEvent{m_server, "jean!jean@localhost", "#hangman", "sky"});

    ASSERT_EQ("#hangman:win=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:sky", m_server->last());
}

TEST_F(HangmanTest, wrongLetter)
{
    load();

    m_plugin->onCommand(m_irccd, MessageEvent{m_server, "jean!jean@localhost", "#hangman", ""});
    m_plugin->onMessage(m_irccd, MessageEvent{m_server, "jean!jean@localhost", "#hangman", "x"});

    ASSERT_EQ("#hangman:wrong-letter=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:x", m_server->last());
}

TEST_F(HangmanTest, wrongWord)
{
    load();

    m_plugin->onCommand(m_irccd, MessageEvent{m_server, "jean!jean@localhost", "#hangman", ""});
    m_plugin->onCommand(m_irccd, MessageEvent{m_server, "jean!jean@localhost", "#hangman", "cheese"});

    ASSERT_EQ("#hangman:wrong-word=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:cheese", m_server->last());
}

TEST_F(HangmanTest, collaborativeDisabled)
{
    // Disable collaborative mode.
    load({{ "collaborative", "false" }});

    m_plugin->onCommand(m_irccd, MessageEvent{m_server, "jean!jean@localhost", "#hangman", ""});
    m_plugin->onMessage(m_irccd, MessageEvent{m_server, "jean!jean@localhost", "#hangman", "s"});
    ASSERT_EQ("#hangman:found=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:s _ _", m_server->last());
    m_plugin->onMessage(m_irccd, MessageEvent{m_server, "jean!jean@localhost", "#hangman", "k"});
    ASSERT_EQ("#hangman:found=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:s k _", m_server->last());
}

TEST_F(HangmanTest, collaborativeEnabled)
{
    // Enable collaborative mode.
    load({{ "collaborative", "true" }});

    m_plugin->onCommand(m_irccd, MessageEvent{m_server, "jean!jean@localhost", "#hangman", ""});
    m_plugin->onMessage(m_irccd, MessageEvent{m_server, "jean!jean@localhost", "#hangman", "s"});
    ASSERT_EQ("#hangman:found=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:s _ _", m_server->last());
    m_plugin->onMessage(m_irccd, MessageEvent{m_server, "jean!jean@localhost", "#hangman", "k"});
    ASSERT_EQ("#hangman:wrong-player=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:k", m_server->last());
    m_plugin->onMessage(m_irccd, MessageEvent{m_server, "francis!francis@localhost", "#hangman", "k"});
    ASSERT_EQ("#hangman:found=hangman:!hangman:test:#hangman:francis!francis@localhost:francis:s k _", m_server->last());
}

int main(int argc, char **argv)
{
    path::setApplicationPath(argv[0]);
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
