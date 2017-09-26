/*
 * main.cpp -- test server-topic remote command
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
std::string cmd_topic;

} // !namespace

class ServerTopicTest : public ServerTester {
public:
    void topic(std::string channel, std::string topic) override
    {
        ::cmd_channel = channel;
        ::cmd_topic = topic;
    }
};

class ServerTopicCommandTest : public CommandTester {
public:
    ServerTopicCommandTest()
        : CommandTester(std::make_unique<server_topic_command>(),
                        std::make_unique<ServerTopicTest>())
    {
        m_irccdctl.client().request({
            { "command",    "server-topic"  },
            { "server",     "test"          },
            { "channel",    "#staff"        },
            { "topic",      "new version"   }
        });
    }
};

TEST_F(ServerTopicCommandTest, basic)
{
    try {
        poll([&] () {
            return !cmd_channel.empty() && !cmd_topic.empty();
        });

        ASSERT_EQ("#staff", cmd_channel);
        ASSERT_EQ("new version", cmd_topic);
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
