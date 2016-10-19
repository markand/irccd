/*
 * main.cpp -- test server-invite remote command
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

#include <cmd-server-invite.hpp>
#include <command-tester.hpp>
#include <server-tester.hpp>

using namespace irccd;
using namespace irccd::command;

namespace {

std::string target;
std::string channel;

} // !namespace

class ServerInviteTest : public ServerTester {
public:
    void invite(std::string target, std::string channel) override
    {
        ::target = target;
        ::channel = channel;
    }
};

class ServerInviteCommandTest : public CommandTester {
public:
    ServerInviteCommandTest()
        : CommandTester(std::make_unique<ServerInviteCommand>(),
                        std::make_unique<ServerInviteTest>())
    {
        m_irccdctl.client().request({
            { "command",    "server-invite"     },
            { "server",     "test"              },
            { "target",     "francis"           },
            { "channel",    "#music"            }
        });
    }
};

TEST_F(ServerInviteCommandTest, basic)
{
    try {
        poll([&] () {
            return !target.empty() && !channel.empty();
        });

        ASSERT_EQ("francis", target);
        ASSERT_EQ("#music", channel);
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
