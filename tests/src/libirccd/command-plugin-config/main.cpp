/*
 * main.cpp -- test plugin-config remote command
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

#define BOOST_TEST_MODULE "plugin-config"
#include <boost/test/unit_test.hpp>

#include <irccd/daemon/command/plugin_config_command.hpp>
#include <irccd/daemon/service/plugin_service.hpp>

#include <irccd/test/command_test.hpp>
#include <irccd/test/mock_plugin.hpp>

namespace irccd {

namespace {

BOOST_FIXTURE_TEST_SUITE(plugin_config_test_suite, command_test<plugin_config_command>)

BOOST_AUTO_TEST_CASE(set)
{
    daemon_->plugins().add(std::make_unique<mock_plugin>("test"));

    const auto [json, code] = request({
        { "command",    "plugin-config" },
        { "plugin",     "test"          },
        { "variable",   "verbosy"       },
        { "value",      "falsy"         }
    });

    const auto config = daemon_->plugins().require("test")->get_options();

    BOOST_TEST(!code);
    BOOST_TEST(!config.empty());
    BOOST_TEST(config.at("verbosy") == "falsy");
}

BOOST_AUTO_TEST_CASE(get)
{
    auto plugin = std::make_unique<mock_plugin>("test");

    plugin->set_options({
        { "x1", "10" },
        { "x2", "20" }
    });
    daemon_->plugins().add(std::move(plugin));

    const auto [json, code] = request({
        { "command",    "plugin-config" },
        { "plugin",     "test"          },
        { "variable",   "x1"            }
    });

    BOOST_TEST(!code);
    BOOST_TEST(json["variables"]["x1"].get<std::string>() == "10");
    BOOST_TEST(json["variables"].count("x2") == 0U);
}

BOOST_AUTO_TEST_CASE(getall)
{
    auto plugin = std::make_unique<mock_plugin>("test");

    plugin->set_options({
        { "x1", "10" },
        { "x2", "20" }
    });
    daemon_->plugins().add(std::move(plugin));

    const auto [json, code] = request({
        { "command",    "plugin-config" },
        { "plugin",     "test"          }
    });

    BOOST_TEST(!code);
    BOOST_TEST(json["variables"]["x1"].get<std::string>() == "10");
    BOOST_TEST(json["variables"]["x2"].get<std::string>() == "20");
}

BOOST_AUTO_TEST_SUITE(errors)

BOOST_AUTO_TEST_CASE(invalid_identifier)
{
    const auto [json, code] = request({
        { "command",    "plugin-config" }
    });

    BOOST_TEST(code == plugin_error::invalid_identifier);
    BOOST_TEST(json["error"].get<int>() == plugin_error::invalid_identifier);
    BOOST_TEST(json["errorCategory"].get<std::string>() == "plugin");
}

BOOST_AUTO_TEST_CASE(not_found)
{
    const auto [json, code] = request({
        { "command",    "plugin-config" },
        { "plugin",     "unknown"       }
    });

    BOOST_TEST(code == plugin_error::not_found);
    BOOST_TEST(json["error"].get<int>() == plugin_error::not_found);
    BOOST_TEST(json["errorCategory"].get<std::string>() == "plugin");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
