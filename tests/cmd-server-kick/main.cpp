/*
 * main.cpp -- test server-kick remote command
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

#include <cmd-server-kick.hpp>
#include <command-tester.hpp>
#include <server-tester.hpp>

using namespace irccd;
using namespace irccd::command;

namespace {

std::string target;
std::string channel;
std::string reason;

} // !namespace

class ServerKickTest : public ServerTester {
public:
    void kick(std::string target, std::string channel, std::string reason) override
    {
        ::target = target;
        ::channel = channel;
        ::reason = reason;
    }
};

class ServerKickCommandTest : public CommandTester {
public:
    ServerKickCommandTest()
        : CommandTester(std::make_unique<ServerKickCommand>(),
                        std::make_unique<ServerKickTest>())
    {
        target.clear();
        channel.clear();
        reason.clear();
    }
};

TEST_F(ServerKickCommandTest, basic)
{
    try {
        m_irccdctl.client().request({
            { "command",    "server-kick"       },
            { "server",     "test"              },
            { "target",     "francis"           },
            { "channel",    "#staff"            },
            { "reason",     "too noisy"         }
        });

        poll([&] () {
            return !target.empty() && !channel.empty();
        });

        ASSERT_EQ("francis", target);
        ASSERT_EQ("#staff", channel);
        ASSERT_EQ("too noisy", reason);
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

TEST_F(ServerKickCommandTest, noreason)
{
    try {
        m_irccdctl.client().request({
            { "command",    "server-kick"       },
            { "server",     "test"              },
            { "target",     "francis"           },
            { "channel",    "#staff"            }
        });

        poll([&] () {
            return !target.empty() && !channel.empty();
        });

        ASSERT_EQ("francis", target);
        ASSERT_EQ("#staff", channel);
        ASSERT_EQ("", reason);
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
