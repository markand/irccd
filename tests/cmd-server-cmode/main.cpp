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

using namespace irccd;

namespace {

std::string cmd_channel;
std::string cmd_mode;

} // !namespace

class ServerChannelModeTest : public ServerTester {
public:
    void cmode(std::string channel, std::string mode)
    {
        ::cmd_channel = channel;
        ::cmd_mode = mode;
    }
};

class ServerChannelModeCommandTest : public CommandTester {
public:
    ServerChannelModeCommandTest()
        : CommandTester(std::make_unique<server_channel_mode_command>(),
                        std::make_unique<ServerChannelModeTest>())
    {
        m_irccdctl.client().request({
            { "command",    "server-cmode"      },
            { "server",     "test"              },
            { "channel",    "#staff"            },
            { "mode",       "+c"                }
        });
    }
};

TEST_F(ServerChannelModeCommandTest, basic)
{
    try {
        poll([&] () {
            return !cmd_channel.empty() && !cmd_mode.empty();
        });

        ASSERT_EQ("#staff", cmd_channel);
        ASSERT_EQ("+c", cmd_mode);
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
