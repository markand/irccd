/*
 * main.cpp -- test server-connect remote command
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

using namespace irccd;
using namespace irccd::command;

namespace {

nlohmann::json message;

} // !namespace

class ServerConnectCommandTest : public CommandTester {
public:
    ServerConnectCommandTest()
        : CommandTester(std::make_unique<ServerConnectCommand>())
    {
        message = nullptr;

        m_irccdctl.client().onMessage.connect([&] (auto message) {
            ::message = message;
        });
    }
};

TEST_F(ServerConnectCommandTest, minimal)
{
    try {
        m_irccdctl.client().request({
            { "command",    "server-connect"    },
            { "name",       "local"             },
            { "host",       "irc.example.org"   }
        });

        poll([&] () {
            return message.is_object();
        });

        auto s = m_irccd.servers().get("local");

        ASSERT_TRUE(s != nullptr);
        ASSERT_EQ("local", s->name());
        ASSERT_EQ("irc.example.org", s->host());
        ASSERT_EQ(6667U, s->port());
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

TEST_F(ServerConnectCommandTest, full)
{
    try {
        m_irccdctl.client().request({
            { "command",    "server-connect"    },
            { "name",       "local2"            },
            { "host",       "irc.example2.org"  },
            { "password",   "nonono"            },
            { "nickname",   "francis"           },
            { "realname",   "the_francis"       },
            { "username",   "frc"               },
            { "ctcpVersion", "ultra bot"        },
            { "commandChar", "::"               },
            { "port",       18000               },
            { "ssl",        true                },
            { "sslVerify",  true                },
            { "autoRejoin", true                },
            { "joinInvite", true                }
        });

        poll([&] () {
            return message.is_object();
        });

        auto s = m_irccd.servers().get("local2");

        ASSERT_TRUE(s != nullptr);
        ASSERT_EQ("local2", s->name());
        ASSERT_EQ("irc.example2.org", s->host());
        ASSERT_EQ(18000U, s->port());
        ASSERT_EQ("nonono", s->password());
        ASSERT_EQ("francis", s->nickname());
        ASSERT_EQ("the_francis", s->realname());
        ASSERT_EQ("frc", s->username());
        ASSERT_EQ("::", s->commandCharacter());
        ASSERT_EQ("ultra bot", s->ctcpVersion());
        ASSERT_TRUE(s->flags() & Server::Ssl);
        ASSERT_TRUE(s->flags() & Server::SslVerify);
        ASSERT_TRUE(s->flags() & Server::AutoRejoin);
        ASSERT_TRUE(s->flags() & Server::JoinInvite);
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
