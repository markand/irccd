/*
 * main.cpp -- test plugin-list remote command
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

#include <command.hpp>
#include <command-tester.hpp>
#include <server-tester.hpp>
#include <service-plugin.hpp>
#include <plugin.hpp>

using namespace irccd;
using namespace irccd::command;

namespace {

class PluginListCommandTest : public CommandTester {
public:
    PluginListCommandTest()
        : CommandTester(std::make_unique<PluginListCommand>())
    {
        m_irccd.plugins().add(std::make_unique<Plugin>("t1", ""));
        m_irccd.plugins().add(std::make_unique<Plugin>("t2", ""));
    }
};

TEST_F(PluginListCommandTest, basic)
{
    try {
        auto response = nlohmann::json();

        m_irccdctl.client().onMessage.connect([&] (auto message) {
            response = message;
        });
        m_irccdctl.client().request({{ "command", "plugin-list" }});

        poll([&] () {
            return response.is_object();
        });

        ASSERT_TRUE(response.is_object());
        ASSERT_EQ("t1", response["list"][0]);
        ASSERT_EQ("t2", response["list"][1]);
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
