/*
 * main.cpp -- test server-disconnect remote command
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
#include <service.hpp>
#include <server.hpp>

using namespace irccd;
using namespace irccd::command;

class ServerDisconnectCommandTest : public CommandTester {
public:
    ServerDisconnectCommandTest()
        : CommandTester(std::make_unique<ServerDisconnectCommand>())
    {
    }
};

TEST_F(ServerDisconnectCommandTest, one)
{
    bool response = false;

    try {
        m_irccd.servers().add(std::make_unique<ServerTester>("s1"));
        m_irccd.servers().add(std::make_unique<ServerTester>("s2"));
        m_irccdctl.client().onMessage.connect([&] (const auto &msg) {
            auto it = msg.find("command");

            if (it != msg.end())
                response = it->is_string() && *it == "server-disconnect";
        });
        m_irccdctl.client().request({
            { "command",    "server-disconnect" },
            { "server",     "s1"                }
        });

        poll([&] () { return response; });

        ASSERT_TRUE(response);
        ASSERT_FALSE(m_irccd.servers().has("s1"));
        ASSERT_TRUE(m_irccd.servers().has("s2"));
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

TEST_F(ServerDisconnectCommandTest, all)
{
    bool response = false;

    try {
        m_irccd.servers().add(std::make_unique<ServerTester>("s1"));
        m_irccd.servers().add(std::make_unique<ServerTester>("s2"));
        m_irccdctl.client().onMessage.connect([&] (const auto &msg) {
            auto it = msg.find("command");

            if (it != msg.end())
                response = it->is_string() && *it == "server-disconnect";
        });
        m_irccdctl.client().request({
            { "command",    "server-disconnect" }
        });

        poll([&] () { return response; });

        ASSERT_TRUE(response);
        ASSERT_FALSE(m_irccd.servers().has("s1"));
        ASSERT_FALSE(m_irccd.servers().has("s2"));
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
