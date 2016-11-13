/*
 * main.cpp -- test server-reconnect remote command
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

bool s1;
bool s2;

} // !namespace

class ServerReconnectTest : public ServerTester {
private:
    bool &m_ref;

public:
    inline ServerReconnectTest(std::string name, bool &ref) noexcept
        : ServerTester(name)
        , m_ref(ref)
    {
        m_ref = false;
    }

    void reconnect() noexcept override
    {
        m_ref = true;
    }
};

class ServerReconnectCommandTest : public CommandTester {
public:
    ServerReconnectCommandTest()
        : CommandTester(std::make_unique<ServerReconnectCommand>())
    {
        m_irccd.servers().add(std::make_unique<ServerReconnectTest>("s1", s1));
        m_irccd.servers().add(std::make_unique<ServerReconnectTest>("s2", s2));
    }
};

TEST_F(ServerReconnectCommandTest, basic)
{
    try {
        m_irccdctl.client().request({
            { "command", "server-reconnect" },
            { "server", "s1" }
        });

        poll([&] () {
            return s1;
        });

        ASSERT_TRUE(s1);
        ASSERT_FALSE(s2);
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

TEST_F(ServerReconnectCommandTest, all)
{
    try {
        m_irccdctl.client().request({{ "command", "server-reconnect" }});

        poll([&] () {
            return s1 && s2;
        });

        ASSERT_TRUE(s1);
        ASSERT_TRUE(s2);
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
