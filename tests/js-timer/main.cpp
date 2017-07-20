/*
 * main.cpp -- test Irccd.Timer API
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

#include <gtest/gtest.h>

#include <irccd/elapsed-timer.hpp>
#include <irccd/irccd.hpp>
#include <irccd/logger.hpp>
#include <irccd/mod-irccd.hpp>
#include <irccd/mod-plugin.hpp>
#include <irccd/mod-timer.hpp>
#include <irccd/plugin-js.hpp>
#include <irccd/service.hpp>
#include <irccd/system.hpp>

using namespace irccd;

class TestJsTimer : public testing::Test {
protected:
    Irccd m_irccd;
    std::shared_ptr<JsPlugin> m_plugin;

    void open(const std::string &file)
    {
        m_plugin = std::make_shared<JsPlugin>("timer", file);

        IrccdModule().load(m_irccd, m_plugin);
        PluginModule().load(m_irccd, m_plugin);
        TimerModule().load(m_irccd, m_plugin);

        m_plugin->onLoad(m_irccd);
        m_irccd.plugins().add(m_plugin);
    }
};

TEST_F(TestJsTimer, single)
{
    open(DIRECTORY "/timer-single.js");

    ElapsedTimer timer;

    while (timer.elapsed() < 3000)
        util::poller::poll(512, m_irccd);

    ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "count"));
    ASSERT_EQ(1, duk_get_int(m_plugin->context(), -1));
}

TEST_F(TestJsTimer, repeat)
{
    open(DIRECTORY "/timer-repeat.js");

    ElapsedTimer timer;

    while (timer.elapsed() < 3000)
        util::poller::poll(512, m_irccd);

    ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "count"));
    ASSERT_GE(duk_get_int(m_plugin->context(), -1), 5);
}

#if 0

/*
 * XXX: currently disabled because it will break single-shot timers.
 */

TEST(Basic, pending)
{
    /*
     * This test ensure that if pending actions on a stopped timer are never executed.
     */
    Irccd irccd;
    ElapsedTimer timer;

    auto plugin = std::make_shared<Plugin>("timer", DIRECTORY "/timer-pending.js");

    irccd.addPlugin(plugin);
    irccd.poll();
    irccd.dispatch();

    ASSERT_EQ(0, plugin->context().getGlobal<int>("count"));
}

#endif

int main(int argc, char **argv)
{
    // Needed for some components.
    sys::setProgramName("irccd");
    log::setLogger(std::make_unique<log::SilentLogger>());
    log::setVerbose(true);
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
