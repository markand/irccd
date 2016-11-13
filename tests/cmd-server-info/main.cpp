/*
 * main.cpp -- test server-cmode remote command
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
#include <service-server.hpp>

using namespace irccd;
using namespace irccd::command;

namespace {

nlohmann::json message;

} // !namespace

class ServerInfoCommandTest : public CommandTester {
public:
    ServerInfoCommandTest()
        : CommandTester(std::make_unique<ServerInfoCommand>())
    {
        message = nullptr;

        m_irccdctl.client().onMessage.connect([&] (auto message) {
            ::message = message;
        });
    }
};

TEST_F(ServerInfoCommandTest, basic)
{
    try {
        auto server = std::make_unique<ServerTester>();

        server->setHost("example.org");
        server->setPort(8765);
        server->setPassword("none");
        server->setNickname("pascal");
        server->setUsername("psc");
        server->setRealname("Pascal le grand frere");
        server->setCtcpVersion("yeah");
        server->setCommandCharacter("@");
        server->setReconnectTries(80);
        server->setPingTimeout(20000);

        m_irccd.servers().add(std::move(server));
        m_irccdctl.client().request({
            { "command",    "server-info"       },
            { "server",     "test"              },
        });

        poll([&] () {
            return message.is_object();
        });

        ASSERT_TRUE(message.is_object());
        ASSERT_EQ("example.org", message["host"]);
        ASSERT_EQ("test", message["name"]);
        ASSERT_EQ("pascal", message["nickname"]);
        ASSERT_EQ(8765, message["port"]);
        ASSERT_EQ("Pascal le grand frere", message["realname"]);
        ASSERT_EQ("psc", message["username"]);
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

TEST_F(ServerInfoCommandTest, notfound)
{
    try {
        m_irccdctl.client().request({
            { "command",    "server-info"       },
            { "server",     "test"              },
        });

        poll([&] () {
            return message.is_object();
        });

        ASSERT_TRUE(message.is_object());
        ASSERT_FALSE(message["status"]);
        ASSERT_EQ("server test not found", message["error"]);
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
