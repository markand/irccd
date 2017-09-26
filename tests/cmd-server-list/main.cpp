/*
 * main.cpp -- test server-list remote command
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

using namespace irccd;

namespace {

nlohmann::json result;

} // !namespace

class ServerListCommandTest : public CommandTester {
public:
    ServerListCommandTest()
        : CommandTester(std::make_unique<server_list_command>())
    {
        m_irccd.servers().add(std::make_unique<ServerTester>("s1"));
        m_irccd.servers().add(std::make_unique<ServerTester>("s2"));
        m_irccdctl.client().request({{ "command", "server-list" }});
        m_irccdctl.client().onMessage.connect([&] (auto result) {
            ::result = result;
        });
    }
};

TEST_F(ServerListCommandTest, basic)
{
    try {
        poll([&] () {
            return result.is_object();
        });

        ASSERT_TRUE(result.is_object());
        ASSERT_TRUE(result["list"].is_array());
        ASSERT_EQ(2U, result["list"].size());
        ASSERT_EQ("s1", result["list"][0]);
        ASSERT_EQ("s2", result["list"][1]);
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
