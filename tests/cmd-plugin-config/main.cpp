/*
 * main.cpp -- test plugin-config remote command
 *
 * Copyright (c) 2013-2016 David Demelier <markand@malikania.fr>
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

#include <cmd-plugin-config.hpp>
#include <command-tester.hpp>
#include <server-tester.hpp>
#include <service-plugin.hpp>

using namespace irccd;
using namespace irccd::command;

namespace {

struct CustomPlugin : public Plugin {
    PluginConfig m_config;

    CustomPlugin(std::string name = "test")
        : Plugin(std::move(name), "")
    {
    }

    PluginConfig config() override
    {
        return m_config;
    }

    void setConfig(PluginConfig config) override
    {
        m_config = std::move(config);
    }
};



class PluginConfigCommandTest : public CommandTester {
public:
    PluginConfigCommandTest()
        : CommandTester(std::make_unique<PluginConfigCommand>())
    {
    }
};

TEST_F(PluginConfigCommandTest, set)
{
    try {
        m_irccd.plugins().add(std::make_unique<CustomPlugin>("test"));
        m_irccdctl.client().request({
            { "command", "plugin-config" },
            { "plugin", "test" },
            { "variable", "verbosy" },
            { "value", "falsy" }
        });

        poll([&] {
            return !m_irccd.plugins().require("test")->config().empty();
        });

        auto config = m_irccd.plugins().require("test")->config();

        ASSERT_FALSE(config.empty());
        ASSERT_EQ("falsy", config["verbosy"]);
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

TEST_F(PluginConfigCommandTest, get)
{
    try {
        auto plugin = std::make_unique<CustomPlugin>("test");
        auto json = nlohmann::json();

        plugin->setConfig({
            { "x1", "10" },
            { "x2", "20" }
        });

        m_irccd.plugins().add(std::move(plugin));
        m_irccdctl.client().request({
            { "command", "plugin-config" },
            { "plugin", "test" },
            { "variable", "x1" }
        });
        m_irccdctl.client().onMessage.connect([&] (auto message) {
            json = std::move(message);
        });

        poll([&] {
            return json.is_object();
        });

        ASSERT_TRUE(json.is_object());
        ASSERT_EQ("10", json["variables"]["x1"]);
        ASSERT_TRUE(json["variables"]["x2"].is_null());
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

TEST_F(PluginConfigCommandTest, getAll)
{
    try {
        auto plugin = std::make_unique<CustomPlugin>("test");
        auto json = nlohmann::json();

        plugin->setConfig({
            { "x1", "10" },
            { "x2", "20" }
        });

        m_irccd.plugins().add(std::move(plugin));
        m_irccdctl.client().request({
            { "command", "plugin-config" },
            { "plugin", "test" }
        });
        m_irccdctl.client().onMessage.connect([&] (auto message) {
            json = std::move(message);
        });

        poll([&] {
            return json.is_object();
        });

        ASSERT_TRUE(json.is_object());
        ASSERT_EQ("10", json["variables"]["x1"]);
        ASSERT_EQ("20", json["variables"]["x2"]);
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

} // !namespace

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
