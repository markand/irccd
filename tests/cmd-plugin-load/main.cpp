/*
 * main.cpp -- test plugin-load remote command
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
#include <plugin.hpp>

using namespace irccd;

namespace {

class CustomLoader : public plugin_loader {
public:
    std::shared_ptr<plugin> open(const std::string &,
                                 const std::string &) noexcept override
    {
        return nullptr;
    }

    std::shared_ptr<plugin> find(const std::string &id) noexcept override
    {
        return std::make_unique<plugin>(id, "");
    }
};

class PluginLoadCommandTest : public CommandTester {
public:
    PluginLoadCommandTest()
        : CommandTester(std::make_unique<plugin_load_command>())
    {
        m_irccd.plugins().add_loader(std::make_unique<CustomLoader>());
    }
};

TEST_F(PluginLoadCommandTest, basic)
{
    try {
        m_irccdctl.client().request({
            { "command", "plugin-load" },
            { "plugin", "foo" }
        });

        poll([&] () {
            return m_irccd.plugins().has("foo");
        });

        ASSERT_FALSE(m_irccd.plugins().list().empty());
        ASSERT_TRUE(m_irccd.plugins().has("foo"));
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
