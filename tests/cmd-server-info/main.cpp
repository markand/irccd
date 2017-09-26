/*
 * main.cpp -- test server-cmode remote command
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

nlohmann::json message;

} // !namespace

class ServerInfoCommandTest : public CommandTester {
public:
    ServerInfoCommandTest()
        : CommandTester(std::make_unique<server_info_command>())
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

        server->set_host("example.org");
        server->set_port(8765);
        server->set_password("none");
        server->set_nickname("pascal");
        server->set_username("psc");
        server->set_realname("Pascal le grand frere");
        server->set_ctcp_version("yeah");
        server->set_command_char("@");
        server->set_reconnect_tries(80);
        server->set_ping_timeout(20000);

        m_irccd.servers().add(std::move(server));
        m_irccdctl.client().request({
            { "command",    "server-info"       },
            { "server",     "test"              },
        });

        poll([&] () {
            return message.is_object();
        });

        ASSERT_TRUE(message.is_object());
        ASSERT_EQ("example.org", message["host"].get<std::string>());
        ASSERT_EQ("test", message["name"].get<std::string>());
        ASSERT_EQ("pascal", message["nickname"].get<std::string>());
        ASSERT_EQ(8765, message["port"].get<int>());
        ASSERT_EQ("Pascal le grand frere", message["realname"].get<std::string>());
        ASSERT_EQ("psc", message["username"].get<std::string>());
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
        ASSERT_EQ("server test not found", message["error"].get<std::string>());
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
