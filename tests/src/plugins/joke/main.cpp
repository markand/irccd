/*
 * main.cpp -- test joke plugin
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

#define BOOST_TEST_MODULE "Joke plugin"
#include <boost/test/unit_test.hpp>

#include <irccd/test/plugin_test.hpp>

namespace irccd {

class joke_test : public plugin_test {
public:
    joke_test()
        : plugin_test(PLUGIN_NAME, PLUGIN_PATH)
    {
        plugin_->set_formats({
            { "error", "error=#{server}:#{channel}:#{origin}:#{nickname}" }
        });
    }

    void load(plugin_config config = {})
    {
        // Add file if not there.
        if (config.count("file") == 0)
            config.emplace("file", CMAKE_CURRENT_SOURCE_DIR "/jokes.json");

        plugin_->set_config(config);
        plugin_->on_load(irccd_);
    }
};

BOOST_FIXTURE_TEST_SUITE(joke_test_suite, joke_test)

BOOST_AUTO_TEST_CASE(simple)
{
    /*
     * Jokes.json have two jokes.
     *
     * aaa
     *
     * And
     *
     * bbbb
     * bbbb
     */
    std::unordered_map<std::string, int> said{
        { "aaa", 0 },
        { "bbbb", 0 }
    };

    load();

    auto call = [&] () {
        plugin_->on_command(irccd_, {server_, "jean!jean@localhost", "#joke", ""});

        auto cmd = server_->cqueue().back();

        // "bbbb" is two lines.
        if (cmd["message"] == "bbbb") {
            auto first = server_->cqueue().front();

            BOOST_TEST(first["command"].template get<std::string>() == "message");
            BOOST_TEST(first["target"].template get<std::string>() == "#joke");
            BOOST_TEST(first["message"].template get<std::string>() == "bbbb");
        } else
            BOOST_TEST(cmd["message"].template get<std::string>() == "aaa");

        said[cmd["message"].template get<std::string>()] += 1;
        server_->cqueue().clear();
    };

    call();
    call();

    BOOST_TEST(said.size() == 2U);
    BOOST_TEST(said["aaa"] == 1U);
    BOOST_TEST(said["bbbb"] == 1U);
}

BOOST_AUTO_TEST_CASE(toobig)
{
    // xxx and yyy are both 3-lines which we disallow. only a must be said.
    load({
        { "file", CMAKE_CURRENT_SOURCE_DIR "/jokes-toobig.json" },
        { "max-list-lines", "2" }
    });

    std::unordered_map<std::string, int> said{
        { "a", 0 }
    };

    auto call = [&] () {
        plugin_->on_command(irccd_, {server_, "jean!jean@localhost", "#joke", ""});

        auto cmd = server_->cqueue().back();

        BOOST_TEST(cmd["command"].template get<std::string>() == "message");
        BOOST_TEST(cmd["target"].template get<std::string>() == "#joke");
        BOOST_TEST(cmd["message"].template get<std::string>() == "a");

        said[cmd["message"].template get<std::string>()] += 1;
        server_->cqueue().clear();
    };

    call();
    call();
    call();

    BOOST_TEST(said.size() == 1U);
    BOOST_TEST(said["a"] == 3U);
}

BOOST_AUTO_TEST_CASE(invalid)
{
    // Only a is the valid joke in this file.
    load({
        { "file", CMAKE_CURRENT_SOURCE_DIR "/jokes-invalid.json" },
    });

    std::unordered_map<std::string, int> said{
        { "a", 0 }
    };

    auto call = [&] () {
        plugin_->on_command(irccd_, {server_, "jean!jean@localhost", "#joke", ""});

        auto cmd = server_->cqueue().back();

        BOOST_TEST(cmd["command"].template get<std::string>() == "message");
        BOOST_TEST(cmd["target"].template get<std::string>() == "#joke");
        BOOST_TEST(cmd["message"].template get<std::string>() == "a");

        server_->cqueue().clear();
        said[cmd["message"].template get<std::string>()] += 1;
    };

    call();
    call();
    call();

    BOOST_TEST(said.size() == 1U);
    BOOST_TEST(said["a"] == 3U);
}

BOOST_AUTO_TEST_SUITE(errors)

BOOST_AUTO_TEST_CASE(not_found)
{
    load({{"file", "doesnotexist.json"}});

    plugin_->on_command(irccd_, {server_, "jean!jean@localhost", "#joke", ""});

    auto cmd = server_->cqueue().back();

    BOOST_TEST(cmd["command"].get<std::string>() == "message");
    BOOST_TEST(cmd["target"].get<std::string>() == "#joke");
    BOOST_TEST(cmd["message"].get<std::string>() == "error=test:#joke:jean!jean@localhost:jean");
}

BOOST_AUTO_TEST_CASE(not_array)
{
    load({{"file", CMAKE_CURRENT_SOURCE_DIR "/jokes-not-array.json"}});

    plugin_->on_command(irccd_, {server_, "jean!jean@localhost", "#joke", ""});

    auto cmd = server_->cqueue().back();

    BOOST_TEST(cmd["command"].get<std::string>() == "message");
    BOOST_TEST(cmd["target"].get<std::string>() == "#joke");
    BOOST_TEST(cmd["message"].get<std::string>() == "error=test:#joke:jean!jean@localhost:jean");
}

BOOST_AUTO_TEST_CASE(empty)
{
    load({{"file", CMAKE_CURRENT_SOURCE_DIR "/jokes-empty.json"}});

    plugin_->on_command(irccd_, {server_, "jean!jean@localhost", "#joke", ""});

    auto cmd = server_->cqueue().back();

    BOOST_TEST(cmd["command"].get<std::string>() == "message");
    BOOST_TEST(cmd["target"].get<std::string>() == "#joke");
    BOOST_TEST(cmd["message"].get<std::string>() == "error=test:#joke:jean!jean@localhost:jean");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

} // !irccd
