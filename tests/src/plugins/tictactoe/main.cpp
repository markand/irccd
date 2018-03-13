/*
 * main.cpp -- test plugin plugin
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

#define BOOST_TEST_MODULE "Plugin tictactoe"
#include <boost/test/unit_test.hpp>

#include <irccd/string_util.hpp>

#include <irccd/daemon/irccd.hpp>
#include <irccd/daemon/server.hpp>
#include <irccd/daemon/service/plugin_service.hpp>

#include <irccd/test/plugin_test.hpp>

namespace irccd {

class test_fixture : public plugin_test {
public:
    test_fixture()
        : plugin_test(PLUGIN_NAME, PLUGIN_PATH)
    {
        plugin_->set_formats({
            { "draw",       "draw=#{channel}:#{command}:#{nickname}:#{plugin}:#{server}"                },
            { "invalid",    "invalid=#{channel}:#{command}:#{nickname}:#{origin}:#{plugin}:#{server}"   },
            { "running",    "running=#{channel}:#{command}:#{nickname}:#{origin}:#{plugin}:#{server}"   },
            { "turn",       "turn=#{channel}:#{command}:#{nickname}:#{plugin}:#{server}"                },
            { "used",       "used=#{channel}:#{command}:#{nickname}:#{origin}:#{plugin}:#{server}"      },
            { "win",        "win=#{channel}:#{command}:#{nickname}:#{plugin}:#{server}"                 }
        });
    }

    auto next_players() const
    {
        if (server_->cqueue().size() == 0)
            throw std::runtime_error("no message");

        const auto cmd = server_->cqueue().back();
        const auto list = string_util::split(cmd["message"].get<std::string>(), ":");

        BOOST_TEST(list.size() == 5U);
        BOOST_TEST(list[0] == "turn=#tictactoe");
        BOOST_TEST(list[1] == "!tictactoe");
        BOOST_TEST(list[3] == "tictactoe");
        BOOST_TEST(list[4] == "test");

        return list[2] == "a" ? std::make_pair("a", "b") : std::make_pair("b", "a");
    }

    auto start()
    {
        plugin_->on_command(irccd_, {server_, "a!a@localhost", "#tictactoe", "b"});
        plugin_->on_names(irccd_, {server_, "#tictactoe", {"a", "b"}});

        return next_players();
    }

    /**
     * Helper to place several tokens on the board and automatically toggling
     * players.
     *
     * This will start the game from "a" with target opponent "b".
     *
     */
    void run(const std::initializer_list<std::string>& points)
    {
        auto players = start();

        for (const auto& p : points) {
            server_->cqueue().clear();
            plugin_->on_message(irccd_, {server_, players.first, "#tictactoe", p});
            players = next_players();
        }
    }
};

BOOST_FIXTURE_TEST_SUITE(test_fixture_suite, test_fixture)

BOOST_AUTO_TEST_CASE(win)
{
    run({"a 1", "b 1", "a 2", "b 2"});

    const auto players = next_players();

    plugin_->on_message(irccd_, {server_, players.first, "#tictactoe", "a 3"});

    const auto cmd = server_->cqueue().back();

    BOOST_TEST(cmd.is_object());
    BOOST_TEST(cmd["command"].get<std::string>() == "message");
    BOOST_TEST(cmd["target"].get<std::string>() == "#tictactoe");

    const auto parts = string_util::split(cmd["message"].get<std::string>(), ":");

    BOOST_TEST(parts.size() == 5U);
    BOOST_TEST(parts[0] == "win=#tictactoe");
    BOOST_TEST(parts[1] == "!tictactoe");
    BOOST_TEST(parts[2] == players.first);
    BOOST_TEST(parts[3] == "tictactoe");
    BOOST_TEST(parts[4] == "test");
}

BOOST_AUTO_TEST_CASE(draw)
{
    /*
     *   a b c
     * 1 o x o
     * 2 o x x
     * 3 x o x
     */
    run({ "b 2", "c 1", "c 3", "b 3", "c 2", "a 2", "a 3", "a 1" });

    const auto players = next_players();

    plugin_->on_message(irccd_, {server_, players.first, "#tictactoe", "b 1"});

    const auto cmd = server_->cqueue().back();

    BOOST_TEST(cmd.is_object());
    BOOST_TEST(cmd["command"].get<std::string>() == "message");
    BOOST_TEST(cmd["target"].get<std::string>() == "#tictactoe");

    const auto parts = string_util::split(cmd["message"].get<std::string>(), ":");

    BOOST_TEST(parts.size() == 5U);
    BOOST_TEST(parts[0] == "draw=#tictactoe");
    BOOST_TEST(parts[1] == "!tictactoe");
    BOOST_TEST(parts[2] == players.first);
    BOOST_TEST(parts[3] == "tictactoe");
    BOOST_TEST(parts[4] == "test");
}

BOOST_AUTO_TEST_CASE(used)
{
    auto players = start();

    plugin_->on_message(irccd_, {server_, players.first, "#tictactoe", "a 1"});
    plugin_->on_message(irccd_, {server_, players.second, "#tictactoe", "a 1"});

    const auto cmd = server_->cqueue().back();

    BOOST_TEST(cmd.is_object());
    BOOST_TEST(cmd["command"].get<std::string>() == "message");
    BOOST_TEST(cmd["target"].get<std::string>() == "#tictactoe");

    const auto parts = string_util::split(cmd["message"].get<std::string>(), ":");

    BOOST_TEST(parts[0] == "used=#tictactoe");
    BOOST_TEST(parts[1] == "!tictactoe");
    BOOST_TEST(parts[2] == players.second);
    BOOST_TEST(parts[3] == players.second);
    BOOST_TEST(parts[4] == "tictactoe");
    BOOST_TEST(parts[5] == "test");
}

BOOST_AUTO_TEST_CASE(invalid)
{
    nlohmann::json cmd;

    // empty name (no names)
    plugin_->on_command(irccd_, {server_, "jean", "#tictactoe", ""});
    cmd = server_->cqueue().back();

    BOOST_TEST(cmd["command"].get<std::string>() == "message");
    BOOST_TEST(cmd["target"].get<std::string>() == "#tictactoe");
    BOOST_TEST(cmd["message"].get<std::string>() == "invalid=#tictactoe:!tictactoe:jean:jean:tictactoe:test");

    // bot name (no names)
    plugin_->on_command(irccd_, {server_, "jean", "#tictactoe", "irccd"});
    cmd = server_->cqueue().back();

    BOOST_TEST(cmd["command"].get<std::string>() == "message");
    BOOST_TEST(cmd["target"].get<std::string>() == "#tictactoe");
    BOOST_TEST(cmd["message"].get<std::string>() == "invalid=#tictactoe:!tictactoe:jean:jean:tictactoe:test");

    // target is origin (no names)
    plugin_->on_command(irccd_, {server_, server_->nickname(), "#tictactoe", server_->nickname()});
    cmd = server_->cqueue().back();

    BOOST_TEST(cmd["command"].get<std::string>() == "message");
    BOOST_TEST(cmd["target"].get<std::string>() == "#tictactoe");
    BOOST_TEST(cmd["message"].get<std::string>() == "invalid=#tictactoe:!tictactoe:irccd:irccd:tictactoe:test");

    // not existing (names)
    plugin_->on_command(irccd_, {server_, server_->nickname(), "#tictactoe", server_->nickname()});
    plugin_->on_names(irccd_, {server_, "#tictactoe", {"a", "b", "c"}});
    cmd = server_->cqueue().back();

    BOOST_TEST(cmd["command"].get<std::string>() == "message");
    BOOST_TEST(cmd["target"].get<std::string>() == "#tictactoe");
    BOOST_TEST(cmd["message"].get<std::string>() == "invalid=#tictactoe:!tictactoe:irccd:irccd:tictactoe:test");
}

BOOST_AUTO_TEST_CASE(random)
{
    /*
     * Ensure that the first player is not always the originator, start the game
     * for at most 1'000'000 times to avoid forever loop.
     */
    unsigned count = 0;
    bool a = false;
    bool b = false;

    // Last player turn is the winner.
    while (!a && !b && count++ < 1000000U) {
        run({"a 1", "b 1", "a 2", "b 2"});

        const auto players = next_players();

        if (players.first == std::string("a"))
            a = true;
        else
            b = true;

        plugin_->on_message(irccd_, {server_, players.first, "#tictactoe", "a 3"});
    }
}

BOOST_AUTO_TEST_CASE(kick)
{
    auto players = start();

    server_->cqueue().clear();
    plugin_->on_kick(irccd_, {server_, "kefka", "#tictactoe", players.first, ""});
    plugin_->on_message(irccd_, {server_, players.first, "#tictactoe", "a 1"});

    BOOST_TEST(server_->cqueue().empty());
}

BOOST_AUTO_TEST_CASE(part)
{
    auto players = start();

    server_->cqueue().clear();
    plugin_->on_part(irccd_, {server_, players.first, "#tictactoe", ""});
    plugin_->on_message(irccd_, {server_, players.first, "#tictactoe", "a 1"});

    BOOST_TEST(server_->cqueue().empty());
}

BOOST_AUTO_TEST_SUITE_END()

} // !irccd
