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

#define BOOST_TEST_MODULE "Plugin plugin"
#include <boost/test/unit_test.hpp>
#include <boost/format.hpp>

#include <irccd/string_util.hpp>

#include <irccd/daemon/irccd.hpp>
#include <irccd/daemon/server.hpp>
#include <irccd/daemon/service/plugin_service.hpp>

#include <irccd/test/plugin_test.hpp>

using boost::format;
using boost::str;

namespace irccd {

namespace {

class fake_plugin : public plugin {
public:
    auto get_name() const noexcept -> std::string_view override
    {
        return "fake";
    }

    auto get_author() const noexcept -> std::string_view override
    {
        return "jean";
    }

    auto get_version() const noexcept -> std::string_view override
    {
        return "0.0.0.0.0.1";
    }

    auto get_license() const noexcept -> std::string_view override
    {
        return "BEER";
    }

    auto get_summary() const noexcept -> std::string_view override
    {
        return "Fake White Beer 2000";
    }
};

class test_fixture : public plugin_test {
public:
    test_fixture()
        : plugin_test(PLUGIN_PATH)
    {
        irccd_.plugins().add("fake", std::make_shared<fake_plugin>());

        plugin_->set_formats({
            { "usage", "usage=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}" },
            { "info", "info=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}:#{author}:#{license}:#{name}:#{summary}:#{version}" },
            { "not-found", "not-found=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}:#{name}" },
            { "too-long", "too-long=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}" }
        });
        plugin_->handle_load(irccd_);
    }
};

BOOST_FIXTURE_TEST_SUITE(test_fixture_suite, test_fixture)

BOOST_AUTO_TEST_CASE(format_usage)
{
    nlohmann::json cmd;

    plugin_->handle_command(irccd_, {server_, "jean!jean@localhost", "#staff", ""});
    cmd = server_->cqueue().front();

    BOOST_REQUIRE_EQUAL(cmd["command"].get<std::string>(), "message");
    BOOST_REQUIRE_EQUAL(cmd["target"].get<std::string>(), "#staff");
    BOOST_REQUIRE_EQUAL(cmd["message"].get<std::string>(), "usage=plugin:!plugin:test:#staff:jean!jean@localhost:jean");

    plugin_->handle_command(irccd_, {server_, "jean!jean@localhost", "#staff", "fail"});
    cmd = server_->cqueue().front();

    BOOST_REQUIRE_EQUAL(cmd["command"].get<std::string>(), "message");
    BOOST_REQUIRE_EQUAL(cmd["target"].get<std::string>(), "#staff");
    BOOST_REQUIRE_EQUAL(cmd["message"].get<std::string>(), "usage=plugin:!plugin:test:#staff:jean!jean@localhost:jean");

    plugin_->handle_command(irccd_, {server_, "jean!jean@localhost", "#staff", "info"});
    cmd = server_->cqueue().front();

    BOOST_REQUIRE_EQUAL(cmd["command"].get<std::string>(), "message");
    BOOST_REQUIRE_EQUAL(cmd["target"].get<std::string>(), "#staff");
    BOOST_REQUIRE_EQUAL(cmd["message"].get<std::string>(), "usage=plugin:!plugin:test:#staff:jean!jean@localhost:jean");
}

BOOST_AUTO_TEST_CASE(format_info)
{
    plugin_->handle_command(irccd_, {server_, "jean!jean@localhost", "#staff", "info fake"});

    auto cmd = server_->cqueue().front();

    BOOST_REQUIRE_EQUAL(cmd["command"].get<std::string>(), "message");
    BOOST_REQUIRE_EQUAL(cmd["target"].get<std::string>(), "#staff");
    BOOST_REQUIRE_EQUAL(cmd["message"].get<std::string>(), "info=plugin:!plugin:test:#staff:jean!jean@localhost:jean:jean:BEER:fake:Fake White Beer 2000:0.0.0.0.0.1");
}

BOOST_AUTO_TEST_CASE(format_not_found)
{
    plugin_->handle_command(irccd_, {server_, "jean!jean@localhost", "#staff", "info doesnotexistsihope"});

    auto cmd = server_->cqueue().front();

    BOOST_REQUIRE_EQUAL(cmd["command"].get<std::string>(), "message");
    BOOST_REQUIRE_EQUAL(cmd["target"].get<std::string>(), "#staff");
    BOOST_REQUIRE_EQUAL(cmd["message"].get<std::string>(), "not-found=plugin:!plugin:test:#staff:jean!jean@localhost:jean:doesnotexistsihope");
}

BOOST_AUTO_TEST_CASE(format_too_long)
{
    for (int i = 0; i < 100; ++i)
        irccd_.plugins().add(str(format("plugin-n-%1%") % i), std::make_shared<fake_plugin>());

    plugin_->handle_command(irccd_, {server_, "jean!jean@localhost", "#staff", "list"});

    auto cmd = server_->cqueue().front();

    BOOST_REQUIRE_EQUAL(cmd["command"].get<std::string>(), "message");
    BOOST_REQUIRE_EQUAL(cmd["target"].get<std::string>(), "#staff");
    BOOST_REQUIRE_EQUAL(cmd["message"].get<std::string>(), "too-long=plugin:!plugin:test:#staff:jean!jean@localhost:jean");
}

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
