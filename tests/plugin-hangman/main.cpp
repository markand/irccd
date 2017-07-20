/*
 * main.cpp -- test hangman plugin
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

#include <unordered_map>
#include <unordered_set>

#include <gtest/gtest.h>

#include <irccd/irccd.hpp>
#include <irccd/server.hpp>
#include <irccd/service.hpp>

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

class HangmanTest : public PluginTester {
protected:
    std::shared_ptr<ServerTest> m_server;
    std::shared_ptr<Plugin> m_plugin;

public:
    HangmanTest()
        : m_server(std::make_shared<ServerTest>())
    {
        m_irccd.plugins().setFormats("hangman", {
            { "asked", "asked=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}:#{letter}" },
            { "dead", "dead=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}:#{word}" },
            { "found", "found=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}:#{word}" },
            { "start", "start=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}:#{word}" },
            { "running", "running=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}:#{word}" },
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

        m_irccd.plugins().setConfig("hangman", config);
        m_irccd.plugins().load("hangman", PLUGINDIR "/hangman.js");
        m_plugin = m_irccd.plugins().require("hangman");
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

TEST_F(HangmanTest, case_fix_642)
{
    load();

    m_plugin->onCommand(m_irccd, MessageEvent{m_server, "jean!jean@localhost", "#hangman", ""});
    m_plugin->onMessage(m_irccd, MessageEvent{m_server, "jean!jean@localhost", "#HANGMAN", "s"});
    ASSERT_EQ("#hangman:found=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:s _ _", m_server->last());
    m_plugin->onMessage(m_irccd, MessageEvent{m_server, "jean!jean@localhost", "#HaNGMaN", "k"});
    ASSERT_EQ("#hangman:wrong-player=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:k", m_server->last());
    m_plugin->onMessage(m_irccd, MessageEvent{m_server, "francis!francis@localhost", "#hAngmAn", "k"});
    ASSERT_EQ("#hangman:found=hangman:!hangman:test:#hangman:francis!francis@localhost:francis:s k _", m_server->last());
}

TEST_F(HangmanTest, query)
{
    load();

    // Query mode is never collaborative.
    m_plugin->onQueryCommand(m_irccd, QueryEvent{m_server, "jean!jean@localhost", ""});
    ASSERT_EQ("jean:start=hangman:!hangman:test:jean:jean!jean@localhost:jean:_ _ _", m_server->last());
    m_plugin->onQuery(m_irccd, QueryEvent{m_server, "jean!jean@localhost", "s"});
    ASSERT_EQ("jean:found=hangman:!hangman:test:jean:jean!jean@localhost:jean:s _ _", m_server->last());
    m_plugin->onQuery(m_irccd, QueryEvent{m_server, "jean!jean@localhost", "k"});
    ASSERT_EQ("jean:found=hangman:!hangman:test:jean:jean!jean@localhost:jean:s k _", m_server->last());
    m_plugin->onQueryCommand(m_irccd, QueryEvent{m_server, "jean!jean@localhost", "sky"});
    ASSERT_EQ("jean:win=hangman:!hangman:test:jean:jean!jean@localhost:jean:sky", m_server->last());
}

TEST_F(HangmanTest, running)
{
    load();

    m_plugin->onCommand(m_irccd, MessageEvent{m_server, "jean!jean@localhost", "#hangman", ""});
    m_plugin->onMessage(m_irccd, MessageEvent{m_server, "jean!jean@localhost", "#hangman", "y"});
    m_plugin->onCommand(m_irccd, MessageEvent{m_server, "jean!jean@localhost", "#hangman", ""});
    ASSERT_EQ("#hangman:running=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:_ _ y", m_server->last());
}

TEST_F(HangmanTest, wordlist_fix_644)
{
    /*
     * To be sure that the selection use the same list, we create a list of
     * three words that has different size to determine which one was selected.
     *
     * Then we run 3 games and verify that the old selection is not the same
     * as the current.
     *
     * This is not very accurate but it's better than nothing.
     */
    load({{ "file", SOURCEDIR "/wordlist_fix_644.conf" }});

    std::unordered_map<unsigned, std::string> words{
        { 14, "abc"     },
        { 16, "abcd"    },
        { 18, "abcde"   }
    };
    std::unordered_set<unsigned> found;

    m_plugin->setFormats({
        { "start", "#{word}" }
    });

    unsigned last, current;

    // 1. Initial game + finish.
    m_plugin->onCommand(m_irccd, MessageEvent{m_server, "jean!jean@localhost", "#hangman", ""});
    last = m_server->last().length();
    found.insert(last);
    m_plugin->onCommand(m_irccd, MessageEvent{m_server, "jean!jean@localhost", "#hangman", words[last]});

    // 2. Current must not be the last one.
    m_plugin->onCommand(m_irccd, MessageEvent{m_server, "jean!jean@localhost", "#hangman", ""});
    current = m_server->last().length();

    ASSERT_NE(last, current);
    ASSERT_EQ(0U, found.count(current));

    found.insert(current);
    last = current;
    m_plugin->onCommand(m_irccd, MessageEvent{m_server, "jean!jean@localhost", "#hangman", words[current]});

    // 3. Last word must be the one that is kept into the map.
    m_plugin->onCommand(m_irccd, MessageEvent{m_server, "jean!jean@localhost", "#hangman", ""});
    current = m_server->last().length();

    ASSERT_NE(last, current);
    ASSERT_EQ(0U, found.count(current));
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
