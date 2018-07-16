/*
 * main.cpp -- test hangman plugin
 *
 * Copyright (c) 2013-2018 David Demelier <markand@malikania.fr>
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

#define BOOST_TEST_MODULE "Hangman plugin"
#include <boost/test/unit_test.hpp>

#include <irccd/daemon/irccd.hpp>
#include <irccd/daemon/server.hpp>

#include <irccd/test/plugin_test.hpp>

namespace irccd {

class hangman_test : public plugin_test {
public:
    hangman_test()
        : plugin_test(PLUGIN_PATH)
    {
        plugin_->set_formats({
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

    void load(plugin::map config = {})
    {
        // Add file if not there.
        if (config.count("file") == 0)
            config.emplace("file", CMAKE_CURRENT_SOURCE_DIR "/words.conf");

        plugin_->set_options(config);
        plugin_->handle_load(irccd_);
    }
};

BOOST_FIXTURE_TEST_SUITE(hangman_test_suite, hangman_test)

BOOST_AUTO_TEST_CASE(asked)
{
    load({{ "collaborative", "false" }});

    plugin_->handle_command(irccd_, {server_, "jean!jean@localhost", "#hangman", ""});

    auto cmd = server_->cqueue().back();

    BOOST_TEST(cmd["command"].get<std::string>() == "message");
    BOOST_TEST(cmd["target"].get<std::string>() == "#hangman");
    BOOST_TEST(cmd["message"].get<std::string>() == "start=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:_ _ _");

    plugin_->handle_message(irccd_, {server_, "jean!jean@localhost", "#hangman", "s"});
    cmd = server_->cqueue().back();

    BOOST_TEST(cmd["command"].get<std::string>() == "message");
    BOOST_TEST(cmd["target"].get<std::string>() == "#hangman");
    BOOST_TEST(cmd["message"].get<std::string>() == "found=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:s _ _");

    plugin_->handle_message(irccd_, {server_, "jean!jean@localhost", "#hangman", "s"});
    cmd = server_->cqueue().back();

    BOOST_TEST(cmd["command"].get<std::string>() == "message");
    BOOST_TEST(cmd["target"].get<std::string>() == "#hangman");
    BOOST_TEST(cmd["message"].get<std::string>() == "asked=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:s");
}

BOOST_AUTO_TEST_CASE(dead)
{
    load({{ "collaborative", "false" }});

    plugin_->handle_command(irccd_, {server_, "jean!jean@localhost", "#hangman", ""});
    plugin_->handle_message(irccd_, {server_, "jean!jean@localhost", "#hangman", "a"});
    plugin_->handle_message(irccd_, {server_, "jean!jean@localhost", "#hangman", "b"});
    plugin_->handle_message(irccd_, {server_, "jean!jean@localhost", "#hangman", "c"});
    plugin_->handle_message(irccd_, {server_, "jean!jean@localhost", "#hangman", "d"});
    plugin_->handle_message(irccd_, {server_, "jean!jean@localhost", "#hangman", "e"});
    plugin_->handle_message(irccd_, {server_, "jean!jean@localhost", "#hangman", "f"});
    plugin_->handle_message(irccd_, {server_, "jean!jean@localhost", "#hangman", "g"});
    plugin_->handle_message(irccd_, {server_, "jean!jean@localhost", "#hangman", "h"});
    plugin_->handle_message(irccd_, {server_, "jean!jean@localhost", "#hangman", "i"});
    plugin_->handle_message(irccd_, {server_, "jean!jean@localhost", "#hangman", "j"});

    auto cmd = server_->cqueue().back();

    BOOST_TEST(cmd["command"].get<std::string>() == "message");
    BOOST_TEST(cmd["target"].get<std::string>() == "#hangman");
    BOOST_TEST(cmd["message"].get<std::string>() == "dead=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:sky");
}

BOOST_AUTO_TEST_CASE(found)
{
    load({{ "collaborative", "false" }});

    plugin_->handle_command(irccd_, {server_, "jean!jean@localhost", "#hangman", ""});
    plugin_->handle_message(irccd_, {server_, "jean!jean@localhost", "#hangman", "s"});

    auto cmd = server_->cqueue().back();

    BOOST_TEST(cmd["command"].get<std::string>() == "message");
    BOOST_TEST(cmd["target"].get<std::string>() == "#hangman");
    BOOST_TEST(cmd["message"].get<std::string>() == "found=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:s _ _");
}

BOOST_AUTO_TEST_CASE(start)
{
    load();

    plugin_->handle_command(irccd_, {server_, "jean!jean@localhost", "#hangman", ""});

    auto cmd = server_->cqueue().back();

    BOOST_TEST(cmd["command"].get<std::string>() == "message");
    BOOST_TEST(cmd["target"].get<std::string>() == "#hangman");
    BOOST_TEST(cmd["message"].get<std::string>() == "start=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:_ _ _");
}

BOOST_AUTO_TEST_CASE(win1)
{
    load({{ "collaborative", "false" }});

    plugin_->handle_command(irccd_, {server_, "jean!jean@localhost", "#hangman", ""});
    plugin_->handle_message(irccd_, {server_, "jean!jean@localhost", "#hangman", "s"});
    plugin_->handle_message(irccd_, {server_, "jean!jean@localhost", "#hangman", "k"});
    plugin_->handle_message(irccd_, {server_, "jean!jean@localhost", "#hangman", "y"});

    auto cmd = server_->cqueue().back();

    BOOST_TEST(cmd["command"].get<std::string>() == "message");
    BOOST_TEST(cmd["target"].get<std::string>() == "#hangman");
    BOOST_TEST(cmd["message"].get<std::string>() == "win=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:sky");
}

BOOST_AUTO_TEST_CASE(win2)
{
    load({{ "collaborative", "false" }});

    plugin_->handle_command(irccd_, {server_, "jean!jean@localhost", "#hangman", ""});
    plugin_->handle_command(irccd_, {server_, "jean!jean@localhost", "#hangman", "sky"});

    auto cmd = server_->cqueue().back();

    BOOST_TEST(cmd["command"].get<std::string>() == "message");
    BOOST_TEST(cmd["target"].get<std::string>() == "#hangman");
    BOOST_TEST(cmd["message"].get<std::string>() == "win=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:sky");
}

BOOST_AUTO_TEST_CASE(wrong_letter)
{
    load();

    plugin_->handle_command(irccd_, {server_, "jean!jean@localhost", "#hangman", ""});
    plugin_->handle_message(irccd_, {server_, "jean!jean@localhost", "#hangman", "x"});

    auto cmd = server_->cqueue().back();

    BOOST_TEST(cmd["command"].get<std::string>() == "message");
    BOOST_TEST(cmd["target"].get<std::string>() == "#hangman");
    BOOST_TEST(cmd["message"].get<std::string>() == "wrong-letter=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:x");
}

BOOST_AUTO_TEST_CASE(wrongWord)
{
    load();

    plugin_->handle_command(irccd_, {server_, "jean!jean@localhost", "#hangman", ""});
    plugin_->handle_command(irccd_, {server_, "jean!jean@localhost", "#hangman", "cheese"});

    auto cmd = server_->cqueue().back();

    BOOST_TEST(cmd["command"].get<std::string>() == "message");
    BOOST_TEST(cmd["target"].get<std::string>() == "#hangman");
    BOOST_TEST(cmd["message"].get<std::string>() == "wrong-word=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:cheese");
}

BOOST_AUTO_TEST_CASE(collaborative_disabled)
{
    // Disable collaborative mode.
    load({{ "collaborative", "false" }});

    plugin_->handle_command(irccd_, {server_, "jean!jean@localhost", "#hangman", ""});
    plugin_->handle_message(irccd_, {server_, "jean!jean@localhost", "#hangman", "s"});

    auto cmd = server_->cqueue().back();

    BOOST_TEST(cmd["command"].get<std::string>() == "message");
    BOOST_TEST(cmd["target"].get<std::string>() == "#hangman");
    BOOST_TEST(cmd["message"].get<std::string>() == "found=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:s _ _");

    plugin_->handle_message(irccd_, {server_, "jean!jean@localhost", "#hangman", "k"});
    cmd = server_->cqueue().back();

    BOOST_TEST(cmd["command"].get<std::string>() == "message");
    BOOST_TEST(cmd["target"].get<std::string>() == "#hangman");
    BOOST_TEST(cmd["message"].get<std::string>() == "found=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:s k _");
}

BOOST_AUTO_TEST_CASE(collaborative_enabled)
{
    // Enable collaborative mode.
    load({{ "collaborative", "true" }});

    plugin_->handle_command(irccd_, {server_, "jean!jean@localhost", "#hangman", ""});
    plugin_->handle_message(irccd_, {server_, "jean!jean@localhost", "#hangman", "s"});

    auto cmd = server_->cqueue().back();

    BOOST_TEST(cmd["command"].get<std::string>() == "message");
    BOOST_TEST(cmd["target"].get<std::string>() == "#hangman");
    BOOST_TEST(cmd["message"].get<std::string>() == "found=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:s _ _");

    plugin_->handle_message(irccd_, {server_, "jean!jean@localhost", "#hangman", "k"});
    cmd = server_->cqueue().back();

    BOOST_TEST(cmd["command"].get<std::string>() == "message");
    BOOST_TEST(cmd["target"].get<std::string>() == "#hangman");
    BOOST_TEST(cmd["message"].get<std::string>() == "wrong-player=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:k");

    plugin_->handle_message(irccd_, {server_, "francis!francis@localhost", "#hangman", "k"});
    cmd = server_->cqueue().back();

    BOOST_TEST(cmd["command"].get<std::string>() == "message");
    BOOST_TEST(cmd["target"].get<std::string>() == "#hangman");
    BOOST_TEST(cmd["message"].get<std::string>() == "found=hangman:!hangman:test:#hangman:francis!francis@localhost:francis:s k _");
}

BOOST_AUTO_TEST_CASE(case_fix_642)
{
    load();

    plugin_->handle_command(irccd_, {server_, "jean!jean@localhost", "#hangman", ""});
    plugin_->handle_message(irccd_, {server_, "jean!jean@localhost", "#HANGMAN", "s"});

    auto cmd = server_->cqueue().back();

    BOOST_TEST(cmd["command"].get<std::string>() == "message");
    BOOST_TEST(cmd["target"].get<std::string>() == "#hangman");
    BOOST_TEST(cmd["message"].get<std::string>() == "found=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:s _ _");

    plugin_->handle_message(irccd_, {server_, "jean!jean@localhost", "#HaNGMaN", "k"});
    cmd = server_->cqueue().back();

    BOOST_TEST(cmd["command"].get<std::string>() == "message");
    BOOST_TEST(cmd["target"].get<std::string>() == "#hangman");
    BOOST_TEST(cmd["message"].get<std::string>() == "wrong-player=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:k");

    plugin_->handle_message(irccd_, {server_, "francis!francis@localhost", "#hAngmAn", "k"});
    cmd = server_->cqueue().back();

    BOOST_TEST(cmd["command"].get<std::string>() == "message");
    BOOST_TEST(cmd["target"].get<std::string>() == "#hangman");
    BOOST_TEST(cmd["message"].get<std::string>() == "found=hangman:!hangman:test:#hangman:francis!francis@localhost:francis:s k _");
}

BOOST_AUTO_TEST_CASE(query)
{
    load();

    // Query mode is never collaborative.
    plugin_->handle_command(irccd_, {server_, "jean!jean@localhost", "irccd", ""});

    auto cmd = server_->cqueue().back();

    BOOST_TEST(cmd["command"].get<std::string>() == "message");
    BOOST_TEST(cmd["target"].get<std::string>() == "jean!jean@localhost");
    BOOST_TEST(cmd["message"].get<std::string>() == "start=hangman:!hangman:test:jean!jean@localhost:jean!jean@localhost:jean:_ _ _");

    plugin_->handle_message(irccd_, {server_, "jean!jean@localhost", "irccd", "s"});
    cmd = server_->cqueue().back();

    BOOST_TEST(cmd["command"].get<std::string>() == "message");
    BOOST_TEST(cmd["target"].get<std::string>() == "jean!jean@localhost");
    BOOST_TEST(cmd["message"].get<std::string>() == "found=hangman:!hangman:test:jean!jean@localhost:jean!jean@localhost:jean:s _ _");

    plugin_->handle_message(irccd_, {server_, "jean!jean@localhost", "irccd", "k"});
    cmd = server_->cqueue().back();

    BOOST_TEST(cmd["command"].get<std::string>() == "message");
    BOOST_TEST(cmd["target"].get<std::string>() == "jean!jean@localhost");
    BOOST_TEST(cmd["message"].get<std::string>() == "found=hangman:!hangman:test:jean!jean@localhost:jean!jean@localhost:jean:s k _");

    plugin_->handle_command(irccd_, {server_, "jean!jean@localhost", "irccd", "sky"});
    cmd = server_->cqueue().back();

    BOOST_TEST(cmd["command"].get<std::string>() == "message");
    BOOST_TEST(cmd["target"].get<std::string>() == "jean!jean@localhost");
    BOOST_TEST(cmd["message"].get<std::string>() == "win=hangman:!hangman:test:jean!jean@localhost:jean!jean@localhost:jean:sky");
}

BOOST_AUTO_TEST_CASE(running)
{
    load();

    plugin_->handle_command(irccd_, {server_, "jean!jean@localhost", "#hangman", ""});
    plugin_->handle_message(irccd_, {server_, "jean!jean@localhost", "#hangman", "y"});
    plugin_->handle_command(irccd_, {server_, "jean!jean@localhost", "#hangman", ""});

    auto cmd = server_->cqueue().back();

    BOOST_TEST(cmd["command"].get<std::string>() == "message");
    BOOST_TEST(cmd["target"].get<std::string>() == "#hangman");
    BOOST_TEST(cmd["message"].get<std::string>() == "running=hangman:!hangman:test:#hangman:jean!jean@localhost:jean:_ _ y");
}

BOOST_AUTO_TEST_CASE(wordlist_fix_644)
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
    load({{ "file", CMAKE_CURRENT_SOURCE_DIR "/wordlist_fix_644.conf" }});

    std::unordered_map<unsigned, std::string> words{
        { 5, "abc"     },
        { 7, "abcd"    },
        { 9, "abcde"   }
    };
    std::unordered_set<unsigned> found;

    plugin_->set_formats({
        { "start", "#{word}" }
    });

    unsigned last, current;

    // 1. Initial game + finish.
    plugin_->handle_command(irccd_, {server_, "jean!jean@localhost", "#hangman", ""});
    last = server_->cqueue().back()["message"].get<std::string>().length();
    found.insert(last);
    plugin_->handle_command(irccd_, {server_, "jean!jean@localhost", "#hangman", words[last]});

    // 2. Current must not be the last one.
    plugin_->handle_command(irccd_, {server_, "jean!jean@localhost", "#hangman", ""});
    current = server_->cqueue().back()["message"].get<std::string>().length();

    BOOST_TEST(last != current);
    BOOST_TEST(0U == found.count(current));

    found.insert(current);
    last = current;
    plugin_->handle_command(irccd_, {server_, "jean!jean@localhost", "#hangman", words[current]});

    // 3. Last word must be the one that is kept into the map.
    plugin_->handle_command(irccd_, {server_, "jean!jean@localhost", "#hangman", ""});
    current = server_->cqueue().back()["message"].get<std::string>().length();

    BOOST_TEST(last != current);
    BOOST_TEST(0U == found.count(current));
}

BOOST_AUTO_TEST_SUITE_END()

} // !irccd
