/*
 * main.cpp -- test plugin-config remote command
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

#define BOOST_TEST_MODULE "plugin-config"
#include <boost/test/unit_test.hpp>

#include <irccd/command.hpp>
#include <irccd/plugin_service.hpp>

#include <command_test.hpp>

namespace irccd {

namespace {

class custom_plugin : public plugin {
public:
    plugin_config config_;

    custom_plugin(std::string name = "test")
        : plugin(std::move(name), "")
    {
    }

    plugin_config config() override
    {
        return config_;
    }

    void set_config(plugin_config config) override
    {
        config_ = std::move(config);
    }
};

} // !namespace

BOOST_FIXTURE_TEST_SUITE(plugin_config_test_suite, command_test<plugin_config_command>)

BOOST_AUTO_TEST_CASE(set)
{
    daemon_->plugins().add(std::make_unique<custom_plugin>("test"));
    ctl_->send({
        { "command",    "plugin-config" },
        { "plugin",     "test"          },
        { "variable",   "verbosy"       },
        { "value",      "falsy"         }
    });

    wait_for([&] {
        return !daemon_->plugins().require("test")->config().empty();
    });

    auto config = daemon_->plugins().require("test")->config();

    BOOST_TEST(!config.empty());
    BOOST_TEST(config["verbosy"] == "falsy");
}

BOOST_AUTO_TEST_CASE(get)
{
    auto plugin = std::make_unique<custom_plugin>("test");
    auto json = nlohmann::json();

    plugin->set_config({
        { "x1", "10" },
        { "x2", "20" }
    });

    daemon_->plugins().add(std::move(plugin));
    ctl_->send({
        { "command", "plugin-config" },
        { "plugin", "test" },
        { "variable", "x1" }
    });
    ctl_->recv([&] (auto, auto message) {
        json = std::move(message);
    });

    wait_for([&] {
        return json.is_object();
    });

    BOOST_TEST(json.is_object());
    BOOST_TEST(json["variables"]["x1"].get<std::string>() == "10");
    BOOST_TEST(json["variables"]["x2"].is_null());
}

BOOST_AUTO_TEST_CASE(getall)
{
    auto plugin = std::make_unique<custom_plugin>("test");
    auto json = nlohmann::json();

    plugin->set_config({
        { "x1", "10" },
        { "x2", "20" }
    });

    daemon_->plugins().add(std::move(plugin));
    ctl_->send({
        { "command", "plugin-config" },
        { "plugin", "test" }
    });
    ctl_->recv([&] (auto, auto message) {
        json = std::move(message);
    });

    wait_for([&] {
        return json.is_object();
    });

    BOOST_TEST(json.is_object());
    BOOST_TEST(json["variables"]["x1"].get<std::string>() == "10");
    BOOST_TEST(json["variables"]["x2"].get<std::string>() == "20");
}

BOOST_AUTO_TEST_SUITE_END()

} // !irccd
