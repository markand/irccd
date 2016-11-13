/*
 * main.cpp -- test plugin-unload remote command
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
#include <plugin.hpp>

using namespace irccd;
using namespace irccd::command;

namespace {

bool called = false;

class CustomPlugin : public Plugin {
public:
    CustomPlugin()
        : Plugin("test", "")
    {
    }

    void onUnload(Irccd &) override
    {
        called = true;
    }
};

class PluginUnloadCommandTest : public CommandTester {
public:
    PluginUnloadCommandTest()
        : CommandTester(std::make_unique<PluginUnloadCommand>())
    {
        called = false;
    }
};

TEST_F(PluginUnloadCommandTest, basic)
{
    try {
        m_irccd.plugins().add(std::make_unique<CustomPlugin>());
        m_irccdctl.client().request({
            { "command", "plugin-unload" },
            { "plugin", "test" }
        });

        poll([&] () {
            return called;
        });

        ASSERT_TRUE(called);
        ASSERT_TRUE(m_irccd.plugins().list().empty());
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

} // !namespace

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
