/*
 * main.cpp -- test Irccd.ElapsedTimer API
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

#include <gtest/gtest.h>

#include <thread>

#include <irccd/irccd.hpp>
#include <irccd/mod-irccd.hpp>
#include <irccd/mod-elapsed-timer.hpp>
#include <irccd/plugin-js.hpp>
#include <irccd/service.hpp>

using namespace irccd;
using namespace std::chrono_literals;

class TestElapsedTimer : public testing::Test {
protected:
    Irccd m_irccd;
    std::shared_ptr<JsPlugin> m_plugin;

    TestElapsedTimer()
        : m_plugin(std::make_shared<JsPlugin>("empty", SOURCEDIR "/empty.js"))
    {
        IrccdModule().load(m_irccd, m_plugin);
        ElapsedTimerModule().load(m_irccd, m_plugin);
    }
};

TEST_F(TestElapsedTimer, standard)
{
    try {
        if (duk_peval_string(m_plugin->context(), "timer = new Irccd.ElapsedTimer();") != 0)
            throw dukx_exception(m_plugin->context(), -1);

        std::this_thread::sleep_for(300ms);

        if (duk_peval_string(m_plugin->context(), "result = timer.elapsed();") != 0)
            throw dukx_exception(m_plugin->context(), -1);

        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "result"));
        ASSERT_GE(duk_get_int(m_plugin->context(), -1), 250);
        ASSERT_LE(duk_get_int(m_plugin->context(), -1), 350);
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

TEST_F(TestElapsedTimer, reset)
{
    try {
        if (duk_peval_string(m_plugin->context(), "timer = new Irccd.ElapsedTimer();") != 0)
            throw dukx_exception(m_plugin->context(), -1);

        std::this_thread::sleep_for(300ms);

        if (duk_peval_string(m_plugin->context(), "timer.reset(); result = timer.elapsed();") != 0)
            throw dukx_exception(m_plugin->context(), -1);

        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "result"));
        ASSERT_LE(duk_get_int(m_plugin->context(), -1), 100);
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
