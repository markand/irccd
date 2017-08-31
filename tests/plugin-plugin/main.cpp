/*
 * main.cpp -- test plugin plugin
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

#define BOOST_TEST_MODULE "Plugin plugin"
#include <boost/test/unit_test.hpp>

#include <irccd/irccd.hpp>
#include <irccd/logger.hpp>
#include <irccd/server.hpp>
#include <irccd/service.hpp>

#include "plugin_test.hpp"

namespace irccd {

class fake_plugin : public plugin {
public:
    fake_plugin()
        : plugin("fake", "")
    {
        set_author("jean");
        set_version("0.0.0.0.0.1");
        set_license("BEER");
        set_summary("Fake White Beer 2000");
    }
};

class test_fixture : public plugin_test {
public:
    test_fixture()
        : plugin_test(PLUGIN_NAME, PLUGIN_PATH)
    {
        irccd_.plugins().add(std::make_shared<fake_plugin>());

        plugin_->set_formats({
            { "usage", "usage=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}" },
            { "info", "info=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}:#{author}:#{license}:#{name}:#{summary}:#{version}" },
            { "not-found", "not-found=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}:#{name}" },
            { "too-long", "too-long=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}" }
        });
        plugin_->on_load(irccd_);
    }
};

BOOST_FIXTURE_TEST_SUITE(test_fixture_suite, test_fixture)

BOOST_AUTO_TEST_CASE(format_usage)
{
    nlohmann::json cmd;

    plugin_->on_command(irccd_, {server_, "jean!jean@localhost", "#staff", ""});
    cmd = server_->cqueue().front();

    BOOST_REQUIRE_EQUAL(cmd["command"].get<std::string>(), "message");
    BOOST_REQUIRE_EQUAL(cmd["target"].get<std::string>(), "#staff");
    BOOST_REQUIRE_EQUAL(cmd["message"].get<std::string>(), "usage=plugin:!plugin:test:#staff:jean!jean@localhost:jean");

    plugin_->on_command(irccd_, {server_, "jean!jean@localhost", "#staff", "fail"});
    cmd = server_->cqueue().front();

    BOOST_REQUIRE_EQUAL(cmd["command"].get<std::string>(), "message");
    BOOST_REQUIRE_EQUAL(cmd["target"].get<std::string>(), "#staff");
    BOOST_REQUIRE_EQUAL(cmd["message"].get<std::string>(), "usage=plugin:!plugin:test:#staff:jean!jean@localhost:jean");

    plugin_->on_command(irccd_, {server_, "jean!jean@localhost", "#staff", "info"});
    cmd = server_->cqueue().front();

    BOOST_REQUIRE_EQUAL(cmd["command"].get<std::string>(), "message");
    BOOST_REQUIRE_EQUAL(cmd["target"].get<std::string>(), "#staff");
    BOOST_REQUIRE_EQUAL(cmd["message"].get<std::string>(), "usage=plugin:!plugin:test:#staff:jean!jean@localhost:jean");
}

BOOST_AUTO_TEST_CASE(format_info)
{
    plugin_->on_command(irccd_, {server_, "jean!jean@localhost", "#staff", "info fake"});

    auto cmd = server_->cqueue().front();

    BOOST_REQUIRE_EQUAL(cmd["command"].get<std::string>(), "message");
    BOOST_REQUIRE_EQUAL(cmd["target"].get<std::string>(), "#staff");
    BOOST_REQUIRE_EQUAL(cmd["message"].get<std::string>(), "info=plugin:!plugin:test:#staff:jean!jean@localhost:jean:jean:BEER:fake:Fake White Beer 2000:0.0.0.0.0.1");
}

BOOST_AUTO_TEST_CASE(format_not_found)
{
    plugin_->on_command(irccd_, {server_, "jean!jean@localhost", "#staff", "info doesnotexistsihope"});

    auto cmd = server_->cqueue().front();

    BOOST_REQUIRE_EQUAL(cmd["command"].get<std::string>(), "message");
    BOOST_REQUIRE_EQUAL(cmd["target"].get<std::string>(), "#staff");
    BOOST_REQUIRE_EQUAL(cmd["message"].get<std::string>(), "not-found=plugin:!plugin:test:#staff:jean!jean@localhost:jean:doesnotexistsihope");
}

BOOST_AUTO_TEST_CASE(format_too_long)
{
    for (int i = 0; i < 100; ++i)
        irccd_.plugins().add(std::make_shared<plugin>(util::sprintf("plugin-n-%d", i), ""));

    plugin_->on_command(irccd_, {server_, "jean!jean@localhost", "#staff", "list"});

    auto cmd = server_->cqueue().front();

    BOOST_REQUIRE_EQUAL(cmd["command"].get<std::string>(), "message");
    BOOST_REQUIRE_EQUAL(cmd["target"].get<std::string>(), "#staff");
    BOOST_REQUIRE_EQUAL(cmd["message"].get<std::string>(), "too-long=plugin:!plugin:test:#staff:jean!jean@localhost:jean");
}

BOOST_AUTO_TEST_SUITE_END()

} // !irccd
