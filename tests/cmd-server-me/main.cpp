/*
 * main.cpp -- test server-me remote command
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

#include <cmd-server-me.hpp>
#include <command-tester.hpp>
#include <server-tester.hpp>

using namespace irccd;
using namespace irccd::command;

namespace {

std::string target;
std::string message;

} // !namespace

class ServerMeTest : public ServerTester {
public:
    void me(std::string target, std::string message) override
    {
        ::target = target;
        ::message = message;
    }
};

class ServerMeCommandTest : public CommandTester {
public:
    ServerMeCommandTest()
        : CommandTester(std::make_unique<ServerMeCommand>(),
                        std::make_unique<ServerMeTest>())
    {
        m_irccdctl.client().request({
            { "command",    "server-me"         },
            { "server",     "test"              },
            { "target",     "jean"              },
            { "message",    "hello!"            }
        });
    }
};

TEST_F(ServerMeCommandTest, basic)
{
    try {
        poll([&] () {
            return !target.empty() && !message.empty();
        });

        ASSERT_EQ("jean", target);
        ASSERT_EQ("hello!", message);
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
