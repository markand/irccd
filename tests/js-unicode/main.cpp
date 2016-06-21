/*
 * main.cpp -- test Irccd.Unicode API
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

/*
 * /!\ Be sure that this file is kept saved in UTF-8 /!\
 */

#include <gtest/gtest.h>

#include <irccd/irccd.hpp>
#include <irccd/mod-irccd.hpp>
#include <irccd/mod-unicode.hpp>
#include <irccd/plugin-js.hpp>
#include <irccd/service-module.hpp>
#include <irccd/system.hpp>

using namespace irccd;

class TestJsUnicode : public testing::Test {
protected:
    Irccd m_irccd;
    std::shared_ptr<JsPlugin> m_plugin;

    TestJsUnicode()
        : m_plugin(std::make_shared<JsPlugin>("empty", SOURCEDIR "/empty.js"))
    {
        m_irccd.moduleService().get("Irccd")->load(m_irccd, m_plugin);
        m_irccd.moduleService().get("Irccd.Unicode")->load(m_irccd, m_plugin);
    }
};

TEST_F(TestJsUnicode, isLetter)
{
    try {
        duk_peval_string_noresult(m_plugin->context(), "result = Irccd.Unicode.isLetter(String('é').charCodeAt(0));");
        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "result"));
        ASSERT_TRUE(duk_get_boolean(m_plugin->context(), -1));

        duk_peval_string_noresult(m_plugin->context(), "result = Irccd.Unicode.isLetter(String('€').charCodeAt(0));");
        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "result"));
        ASSERT_FALSE(duk_get_boolean(m_plugin->context(), -1));
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

TEST_F(TestJsUnicode, isLower)
{
    try {
        duk_peval_string_noresult(m_plugin->context(), "result = Irccd.Unicode.isLower(String('é').charCodeAt(0));");
        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "result"));
        ASSERT_TRUE(duk_get_boolean(m_plugin->context(), -1));

        duk_peval_string_noresult(m_plugin->context(), "result = Irccd.Unicode.isLower(String('É').charCodeAt(0));");
        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "result"));
        ASSERT_FALSE(duk_get_boolean(m_plugin->context(), -1));
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

TEST_F(TestJsUnicode, isUpper)
{
    try {
        duk_peval_string_noresult(m_plugin->context(), "result = Irccd.Unicode.isUpper(String('É').charCodeAt(0));");
        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "result"));
        ASSERT_TRUE(duk_get_boolean(m_plugin->context(), -1));

        duk_peval_string_noresult(m_plugin->context(), "result = Irccd.Unicode.isUpper(String('é').charCodeAt(0));");
        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "result"));
        ASSERT_FALSE(duk_get_boolean(m_plugin->context(), -1));
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
