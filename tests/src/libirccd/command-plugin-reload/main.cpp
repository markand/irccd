/*
 * main.cpp -- test plugin-reload remote command
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

#define BOOST_TEST_MODULE "plugin-reload"
#include <boost/test/unit_test.hpp>

#include <irccd/daemon/command/plugin_reload_command.hpp>
#include <irccd/daemon/service/plugin_service.hpp>

#include <irccd/test/command_test.hpp>

namespace irccd {

namespace {

class reloadable_plugin : public plugin {
public:
    bool reloaded{false};

    reloadable_plugin()
        : plugin("test")
    {
    }

    auto get_name() const noexcept -> std::string_view override
    {
        return "reload";
    }

    void handle_reload(irccd&) override
    {
        reloaded = true;
    }
};

class broken_plugin : public plugin {
public:
    broken_plugin()
        : plugin("broken")
    {
    }

    auto get_name() const noexcept -> std::string_view override
    {
        return "broken";
    }

    void handle_reload(irccd&) override
    {
        throw std::runtime_error("broken");
    }
};

class plugin_reload_test : public command_test<plugin_reload_command> {
protected:
    std::shared_ptr<reloadable_plugin> plugin_;

    plugin_reload_test()
        : plugin_(std::make_shared<reloadable_plugin>())
    {
        daemon_->plugins().add(plugin_);
        daemon_->plugins().add(std::make_unique<broken_plugin>());
    }
};

} // !namespace

BOOST_FIXTURE_TEST_SUITE(plugin_reload_test_suite, plugin_reload_test)

BOOST_AUTO_TEST_CASE(basic)
{
    ctl_->write({
        { "command",    "plugin-reload" },
        { "plugin",     "test"          }
    });

    wait_for([&] () {
        return plugin_->reloaded;
    });

    BOOST_TEST(plugin_->reloaded);
}

BOOST_AUTO_TEST_SUITE(errors)

BOOST_AUTO_TEST_CASE(invalid_identifier)
{
    const auto result = request({
        { "command",    "plugin-reload" }
    });

    BOOST_TEST(result.second == plugin_error::invalid_identifier);
    BOOST_TEST(result.first["error"].template get<int>() == plugin_error::invalid_identifier);
    BOOST_TEST(result.first["errorCategory"].template get<std::string>() == "plugin");
}

BOOST_AUTO_TEST_CASE(not_found)
{
    const auto result = request({
        { "command",    "plugin-reload" },
        { "plugin",     "unknown"       }
    });

    BOOST_TEST(result.second == plugin_error::not_found);
    BOOST_TEST(result.first["error"].template get<int>() == plugin_error::not_found);
    BOOST_TEST(result.first["errorCategory"].template get<std::string>() == "plugin");
}

BOOST_AUTO_TEST_CASE(exec_error)
{
    const auto result = request({
        { "command",    "plugin-reload" },
        { "plugin",     "broken"        }
    });

    BOOST_TEST(result.second == plugin_error::exec_error);
    BOOST_TEST(result.first["error"].template get<int>() == plugin_error::exec_error);
    BOOST_TEST(result.first["errorCategory"].template get<std::string>() == "plugin");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

} // !irccd
