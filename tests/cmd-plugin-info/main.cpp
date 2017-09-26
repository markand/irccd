/*
 * main.cpp -- test plugin-info remote command
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

#include <command.hpp>
#include <command-tester.hpp>
#include <server-tester.hpp>
#include <service.hpp>
#include <plugin.hpp>

using namespace irccd;

namespace {

class PluginInfoCommandTest : public CommandTester {
public:
    PluginInfoCommandTest()
        : CommandTester(std::make_unique<plugin_info_command>())
    {
    }
};

TEST_F(PluginInfoCommandTest, basic)
{
    try {
        auto plg = std::make_unique<plugin>("test", "");
        auto response = nlohmann::json();

        plg->set_author("Francis Beaugrand");
        plg->set_license("GPL");
        plg->set_summary("Completely useless plugin");
        plg->set_version("0.0.0.0.0.0.0.0.1-beta5");

        m_irccd.plugins().add(std::move(plg));
        m_irccdctl.client().onMessage.connect([&] (auto msg) {
            response = std::move(msg);
        });
        m_irccdctl.client().request({
            { "command",    "plugin-info"       },
            { "plugin",     "test"              },
        });

        poll([&] () {
            return response.is_object();
        });

        ASSERT_TRUE(response.is_object());
        ASSERT_EQ("Francis Beaugrand", response["author"]);
        ASSERT_EQ("GPL", response["license"]);
        ASSERT_EQ("Completely useless plugin", response["summary"]);
        ASSERT_EQ("0.0.0.0.0.0.0.0.1-beta5", response["version"]);
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

TEST_F(PluginInfoCommandTest, notfound)
{
    try {
        auto response = nlohmann::json();

        m_irccdctl.client().onMessage.connect([&] (auto msg) {
            response = std::move(msg);
        });
        m_irccdctl.client().request({
            { "command",    "plugin-info"       },
            { "plugin",     "test"              },
        });

        poll([&] () {
            return response.is_object();
        });

        ASSERT_TRUE(response.is_object());
        ASSERT_FALSE(response["status"]);
        ASSERT_EQ("plugin test not found", response["error"]);
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
