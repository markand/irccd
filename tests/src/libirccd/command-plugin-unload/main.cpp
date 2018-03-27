/*
 * main.cpp -- test plugin-unload remote command
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

#define BOOST_TEST_MODULE "plugin-unload"
#include <boost/test/unit_test.hpp>

#include <irccd/daemon/command/plugin_unload_command.hpp>
#include <irccd/daemon/service/plugin_service.hpp>

#include <irccd/test/command_test.hpp>

namespace irccd {

namespace {

class custom_plugin : public plugin {
public:
    bool unloaded{false};

    custom_plugin()
        : plugin("test", "")
    {
    }

    void handle_unload(irccd &) override
    {
        unloaded = true;
    }
};

class broken_plugin : public plugin {
public:
    using plugin::plugin;

    void handle_unload(irccd&) override
    {
        throw std::runtime_error("broken");
    }
};

class plugin_unload_test : public command_test<plugin_unload_command> {
protected:
    std::shared_ptr<custom_plugin> plugin_;

    plugin_unload_test()
        : plugin_(std::make_shared<custom_plugin>())
    {
        daemon_->plugins().add(plugin_);
        daemon_->plugins().add(std::make_unique<broken_plugin>("broken", ""));
    }
};

} // !namespace

BOOST_FIXTURE_TEST_SUITE(plugin_unload_test_suite, plugin_unload_test)

BOOST_AUTO_TEST_CASE(basic)
{
    ctl_->send({
        { "command",    "plugin-unload" },
        { "plugin",     "test"          }
    });

    wait_for([&] () {
        return plugin_->unloaded;
    });

    BOOST_TEST(plugin_->unloaded);
}

BOOST_AUTO_TEST_SUITE(errors)

BOOST_AUTO_TEST_CASE(invalid_identifier)
{
    const auto result = request({
        { "command",    "plugin-unload" }
    });

    BOOST_TEST(result.second == plugin_error::invalid_identifier);
    BOOST_TEST(result.first["error"].template get<int>() == plugin_error::invalid_identifier);
    BOOST_TEST(result.first["errorCategory"].template get<std::string>() == "plugin");
}

BOOST_AUTO_TEST_CASE(not_found)
{
    const auto result = request({
        { "command",    "plugin-unload" },
        { "plugin",     "unknown"       }
    });

    BOOST_TEST(result.second == plugin_error::not_found);
    BOOST_TEST(result.first["error"].template get<int>() == plugin_error::not_found);
    BOOST_TEST(result.first["errorCategory"].template get<std::string>() == "plugin");
}

BOOST_AUTO_TEST_CASE(exec_error)
{
    const auto result = request({
        { "command",    "plugin-unload" },
        { "plugin",     "broken"        }
    });

    BOOST_TEST(result.second == plugin_error::exec_error);
    BOOST_TEST(result.first["error"].template get<int>() == plugin_error::exec_error);
    BOOST_TEST(result.first["errorCategory"].template get<std::string>() == "plugin");
    BOOST_TEST(!daemon_->plugins().has("broken"));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

} // !irccd
