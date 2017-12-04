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

#define BOOST_TEST_MODULE "History plugin"
#include <boost/test/unit_test.hpp>

#include <irccd/irccd.hpp>
#include <irccd/server.hpp>

#include "plugin_test.hpp"

namespace irccd {

class history_test : public plugin_test {
public:
    history_test()
        : plugin_test(PLUGIN_NAME, PLUGIN_PATH)
    {
        plugin_->set_formats({
            { "error", "error=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}" },
            { "seen", "seen=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}:#{target}:%H:%M" },
            { "said", "said=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}:#{target}:#{message}:%H:%M" },
            { "unknown", "unknown=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}:#{target}" },
        });
    }

    void load(plugin_config config = {})
    {
        // Add file if not there.
        if (config.count("file") == 0)
            config.emplace("file", CMAKE_CURRENT_SOURCE_DIR "/words.conf");

        plugin_->set_config(config);
        plugin_->on_load(irccd_);
    }
};

BOOST_FIXTURE_TEST_SUITE(history_test_suite, history_test)

BOOST_AUTO_TEST_CASE(format_error)
{
    load({{"file", CMAKE_CURRENT_SOURCE_DIR "/broken-conf.json"}});

    plugin_->on_command(irccd_, {server_, "jean!jean@localhost", "#history", "seen francis"});

    auto cmd = server_->cqueue().front();

    BOOST_REQUIRE_EQUAL(cmd["command"].get<std::string>(), "message");
    BOOST_REQUIRE_EQUAL(cmd["target"].get<std::string>(), "#history");
    BOOST_REQUIRE_EQUAL(cmd["message"].get<std::string>(), "error=history:!history:test:#history:jean!jean@localhost:jean");
}

BOOST_AUTO_TEST_CASE(format_seen)
{
    const std::regex rule("seen=history:!history:test:#history:destructor!dst@localhost:destructor:jean:\\d{2}:\\d{2}");

    remove(CMAKE_CURRENT_BINARY_DIR "/seen.json");
    load({{ "file", CMAKE_CURRENT_BINARY_DIR "/seen.json" }});

    plugin_->on_message(irccd_, {server_, "jean!jean@localhost", "#history", "hello"});
    plugin_->on_command(irccd_, {server_, "destructor!dst@localhost", "#history", "seen jean"});

    auto cmd = server_->cqueue().front();

    BOOST_REQUIRE_EQUAL(cmd["command"].get<std::string>(), "message");
    BOOST_REQUIRE_EQUAL(cmd["target"].get<std::string>(), "#history");
    BOOST_REQUIRE(std::regex_match(cmd["message"].get<std::string>(), rule));
}

BOOST_AUTO_TEST_CASE(format_said)
{
    std::regex rule("said=history:!history:test:#history:destructor!dst@localhost:destructor:jean:hello:\\d{2}:\\d{2}");

    remove(CMAKE_CURRENT_BINARY_DIR "/said.json");
    load({{ "file", CMAKE_CURRENT_BINARY_DIR "/said.json" }});

    plugin_->on_message(irccd_, {server_, "jean!jean@localhost", "#history", "hello"});
    plugin_->on_command(irccd_, {server_, "destructor!dst@localhost", "#history", "said jean"});

    auto cmd = server_->cqueue().front();

    BOOST_REQUIRE_EQUAL(cmd["command"].get<std::string>(), "message");
    BOOST_REQUIRE_EQUAL(cmd["target"].get<std::string>(), "#history");
    BOOST_REQUIRE(std::regex_match(cmd["message"].get<std::string>(), rule));
}

BOOST_AUTO_TEST_CASE(format_unknown)
{
    remove(CMAKE_CURRENT_BINARY_DIR "/unknown.json");
    load({{ "file", CMAKE_CURRENT_BINARY_DIR "/unknown.json" }});

    plugin_->on_message(irccd_, {server_, "jean!jean@localhost", "#history", "hello"});
    plugin_->on_command(irccd_, {server_, "destructor!dst@localhost", "#history", "seen nobody"});

    auto cmd = server_->cqueue().front();

    BOOST_REQUIRE_EQUAL(cmd["command"].get<std::string>(), "message");
    BOOST_REQUIRE_EQUAL(cmd["target"].get<std::string>(), "#history");
    BOOST_REQUIRE_EQUAL(cmd["message"].get<std::string>(), "unknown=history:!history:test:#history:destructor!dst@localhost:destructor:nobody");
}

BOOST_AUTO_TEST_CASE(fix_642)
{
    const std::regex rule("said=history:!history:test:#history:destructor!dst@localhost:destructor:jean:hello:\\d{2}:\\d{2}");

    remove(CMAKE_CURRENT_BINARY_DIR "/case.json");
    load({{"file", CMAKE_CURRENT_BINARY_DIR "/case.json"}});

    plugin_->on_message(irccd_, {server_, "JeaN!JeaN@localhost", "#history", "hello"});

    // Full caps.
    plugin_->on_command(irccd_, {server_, "destructor!dst@localhost", "#HISTORY", "said JEAN"});

    auto cmd = server_->cqueue().front();

    BOOST_REQUIRE_EQUAL(cmd["command"].get<std::string>(), "message");
    BOOST_REQUIRE_EQUAL(cmd["target"].get<std::string>(), "#history");
    BOOST_REQUIRE(std::regex_match(cmd["message"].get<std::string>(), rule));

    // Random caps.
    plugin_->on_command(irccd_, {server_, "destructor!dst@localhost", "#HiSToRy", "said JeaN"});

    cmd = server_->cqueue().back();

    BOOST_REQUIRE_EQUAL(cmd["command"].get<std::string>(), "message");
    BOOST_REQUIRE_EQUAL(cmd["target"].get<std::string>(), "#history");
    BOOST_REQUIRE(std::regex_match(cmd["message"].get<std::string>(), rule));
}

BOOST_AUTO_TEST_SUITE_END()

} // !irccd
