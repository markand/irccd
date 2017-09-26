/*
 * main.cpp -- test Irccd API
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

#include <irccd/irccd.hpp>
#include <irccd/js_irccd_module.hpp>
#include <irccd/js_plugin.hpp>
#include <irccd/service.hpp>
#include <irccd/sysconfig.hpp>

using namespace irccd;

class TestJsIrccd : public testing::Test {
protected:
    irccd::irccd m_irccd;
    std::shared_ptr<js_plugin> m_plugin;

    TestJsIrccd()
        : m_plugin(std::make_shared<js_plugin>("empty", SOURCEDIR "/empty.js"))
    {
        js_irccd_module().load(m_irccd, m_plugin);
    }
};

TEST_F(TestJsIrccd, version)
{
    try {
        auto ret = duk_peval_string(m_plugin->context(),
            "major = Irccd.version.major;"
            "minor = Irccd.version.minor;"
            "patch = Irccd.version.patch;"
        );

        if (ret != 0)
            throw dukx_exception(m_plugin->context(), -1);

        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "major"));
        ASSERT_EQ(IRCCD_VERSION_MAJOR, duk_get_int(m_plugin->context(), -1));
        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "minor"));
        ASSERT_EQ(IRCCD_VERSION_MINOR, duk_get_int(m_plugin->context(), -1));
        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "patch"));
        ASSERT_EQ(IRCCD_VERSION_PATCH, duk_get_int(m_plugin->context(), -1));
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

TEST_F(TestJsIrccd, fromJavascript)
{
    try {
        auto ret = duk_peval_string(m_plugin->context(),
            "try {"
            "  throw new Irccd.SystemError(1, 'test');"
            "} catch (e) {"
            "  errno = e.errno;"
            "  name = e.name;"
            "  message = e.message;"
            "  v1 = (e instanceof Error);"
            "  v2 = (e instanceof Irccd.SystemError);"
            "}"
        );

        if (ret != 0)
            throw dukx_exception(m_plugin->context(), -1);

        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "errno"));
        ASSERT_EQ(1, duk_get_int(m_plugin->context(), -1));
        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "name"));
        ASSERT_STREQ("SystemError", duk_get_string(m_plugin->context(), -1));
        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "message"));
        ASSERT_STREQ("test", duk_get_string(m_plugin->context(), -1));
        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "v1"));
        ASSERT_TRUE(duk_get_boolean(m_plugin->context(), -1));
        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "v2"));
        ASSERT_TRUE(duk_get_boolean(m_plugin->context(), -1));
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

TEST_F(TestJsIrccd, fromNative)
{
    try {
        duk_push_c_function(m_plugin->context(), [] (duk_context *ctx) -> duk_ret_t {
            dukx_throw(ctx, system_error(EINVAL, "hey"));

            return 0;
        }, 0);

        duk_put_global_string(m_plugin->context(), "f");

        auto ret = duk_peval_string(m_plugin->context(),
            "try {"
            "  f();"
            "} catch (e) {"
            "  errno = e.errno;"
            "  name = e.name;"
            "  message = e.message;"
            "  v1 = (e instanceof Error);"
            "  v2 = (e instanceof Irccd.SystemError);"
            "}"
        );

        if (ret != 0)
            throw dukx_exception(m_plugin->context(), -1);

        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "errno"));
        ASSERT_EQ(EINVAL, duk_get_int(m_plugin->context(), -1));
        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "name"));
        ASSERT_STREQ("SystemError", duk_get_string(m_plugin->context(), -1));
        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "message"));
        ASSERT_STREQ("hey", duk_get_string(m_plugin->context(), -1));
        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "v1"));
        ASSERT_TRUE(duk_get_boolean(m_plugin->context(), -1));
        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "v2"));
        ASSERT_TRUE(duk_get_boolean(m_plugin->context(), -1));
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
