/*
 * main.cpp -- test Irccd.Util API
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
#include <irccd/js_util_module.hpp>
#include <irccd/js_plugin.hpp>
#include <irccd/service.hpp>
#include <irccd/system.hpp>

using namespace irccd;

class TestJsUtil : public testing::Test {
protected:
    irccd::irccd m_irccd;
    std::shared_ptr<js_plugin> m_plugin;

    TestJsUtil()
        : m_plugin(std::make_shared<js_plugin>("empty", SOURCEDIR "/empty.js"))
    {
        js_irccd_module().load(m_irccd, m_plugin);
        js_util_module().load(m_irccd, m_plugin);
    }
};

TEST_F(TestJsUtil, formatSimple)
{
    try {
        auto ret = duk_peval_string(m_plugin->context(),
            "result = Irccd.Util.format(\"#{target}\", { target: \"markand\" })"
        );

        if (ret != 0)
            throw dukx_exception(m_plugin->context(), -1);

        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "result"));
        ASSERT_STREQ("markand", duk_get_string(m_plugin->context(), -1));
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

TEST_F(TestJsUtil, splituser)
{
    try {
        if (duk_peval_string(m_plugin->context(), "result = Irccd.Util.splituser(\"user!~user@hyper/super/host\");") != 0)
            throw dukx_exception(m_plugin->context(), -1);

        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "result"));
        ASSERT_STREQ("user", duk_get_string(m_plugin->context(), -1));
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

TEST_F(TestJsUtil, splithost)
{
    try {
        if (duk_peval_string(m_plugin->context(), "result = Irccd.Util.splithost(\"user!~user@hyper/super/host\");") != 0)
            throw dukx_exception(m_plugin->context(), -1);

        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "result"));
        ASSERT_STREQ("!~user@hyper/super/host", duk_get_string(m_plugin->context(), -1));
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

/*
 * Irccd.Util.cut
 * ------------------------------------------------------------------
 */

TEST_F(TestJsUtil, cut_string_simple)
{
    try {
        auto ret = duk_peval_string(m_plugin->context(),
            "lines = Irccd.Util.cut('hello world');\n"
            "line0 = lines[0];\n"
        );

        if (ret != 0) {
            throw dukx_exception(m_plugin->context(), -1);
        }

        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "line0"));
        ASSERT_STREQ("hello world", duk_get_string(m_plugin->context(), -1));
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

TEST_F(TestJsUtil, cut_string_double)
{
    try {
        auto ret = duk_peval_string(m_plugin->context(),
            "lines = Irccd.Util.cut('hello world', 5);\n"
            "line0 = lines[0];\n"
            "line1 = lines[1];\n"
        );

        if (ret != 0) {
            throw dukx_exception(m_plugin->context(), -1);
        }

        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "line0"));
        ASSERT_STREQ("hello", duk_get_string(m_plugin->context(), -1));
        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "line1"));
        ASSERT_STREQ("world", duk_get_string(m_plugin->context(), -1));
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

TEST_F(TestJsUtil, cut_string_dirty)
{
    try {
        auto ret = duk_peval_string(m_plugin->context(),
            "lines = Irccd.Util.cut('     hello    world     ', 5);\n"
            "line0 = lines[0];\n"
            "line1 = lines[1];\n"
        );

        if (ret != 0) {
            throw dukx_exception(m_plugin->context(), -1);
        }

        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "line0"));
        ASSERT_STREQ("hello", duk_get_string(m_plugin->context(), -1));
        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "line1"));
        ASSERT_STREQ("world", duk_get_string(m_plugin->context(), -1));
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

TEST_F(TestJsUtil, cut_string_too_much_lines)
{
    try {
        auto ret = duk_peval_string(m_plugin->context(),
            "lines = Irccd.Util.cut('abc def ghi jkl', 3, 3);\n"
        );

        if (ret != 0) {
            throw dukx_exception(m_plugin->context(), -1);
        }

        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "lines"));
        ASSERT_TRUE(duk_is_undefined(m_plugin->context(), -1));
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

TEST_F(TestJsUtil, cut_string_token_too_big)
{
    try {
        auto ret = duk_peval_string(m_plugin->context(),
            "try {\n"
            "  lines = Irccd.Util.cut('hello world', 3);\n"
            "} catch (e) {\n"
            "  name = e.name;\n"
            "  message = e.message;\n"
            "}\n"
        );

        if (ret != 0) {
            throw dukx_exception(m_plugin->context(), -1);
        }

        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "name"));
        ASSERT_STREQ("RangeError", duk_get_string(m_plugin->context(), -1));
        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "message"));
        ASSERT_STREQ("word 'hello' could not fit in maxc limit (3)", duk_get_string(m_plugin->context(), -1));
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

TEST_F(TestJsUtil, cut_string_negative_maxc)
{
    try {
        auto ret = duk_peval_string(m_plugin->context(),
            "try {\n"
            "  lines = Irccd.Util.cut('hello world', -3);\n"
            "} catch (e) {\n"
            "  name = e.name;\n"
            "  message = e.message;\n"
            "}\n"
        );

        if (ret != 0) {
            throw dukx_exception(m_plugin->context(), -1);
        }

        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "name"));
        ASSERT_STREQ("RangeError", duk_get_string(m_plugin->context(), -1));
        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "message"));
        ASSERT_STREQ("argument 1 (maxc) must be positive", duk_get_string(m_plugin->context(), -1));
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

TEST_F(TestJsUtil, cut_string_negative_maxl)
{
    try {
        auto ret = duk_peval_string(m_plugin->context(),
            "try {\n"
            "  lines = Irccd.Util.cut('hello world', undefined, -1);\n"
            "} catch (e) {\n"
            "  name = e.name;\n"
            "  message = e.message;\n"
            "}\n"
        );

        if (ret != 0) {
            throw dukx_exception(m_plugin->context(), -1);
        }

        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "name"));
        ASSERT_STREQ("RangeError", duk_get_string(m_plugin->context(), -1));
        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "message"));
        ASSERT_STREQ("argument 2 (maxl) must be positive", duk_get_string(m_plugin->context(), -1));
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

TEST_F(TestJsUtil, cut_array_simple)
{
    try {
        auto ret = duk_peval_string(m_plugin->context(),
            "lines = Irccd.Util.cut([ 'hello', 'world' ]);\n"
            "line0 = lines[0];\n"
        );

        if (ret != 0) {
            throw dukx_exception(m_plugin->context(), -1);
        }

        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "line0"));
        ASSERT_STREQ("hello world", duk_get_string(m_plugin->context(), -1));
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

TEST_F(TestJsUtil, cut_array_double)
{
    try {
        auto ret = duk_peval_string(m_plugin->context(),
            "lines = Irccd.Util.cut([ 'hello', 'world' ], 5);\n"
            "line0 = lines[0];\n"
            "line1 = lines[1];\n"
        );

        if (ret != 0) {
            throw dukx_exception(m_plugin->context(), -1);
        }

        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "line0"));
        ASSERT_STREQ("hello", duk_get_string(m_plugin->context(), -1));
        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "line1"));
        ASSERT_STREQ("world", duk_get_string(m_plugin->context(), -1));
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

TEST_F(TestJsUtil, cut_array_dirty)
{
    try {
        auto ret = duk_peval_string(m_plugin->context(),
            "lines = Irccd.Util.cut([ '   ', ' hello  ', '  world ', '    '], 5);\n"
            "line0 = lines[0];\n"
            "line1 = lines[1];\n"
        );

        if (ret != 0) {
            throw dukx_exception(m_plugin->context(), -1);
        }

        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "line0"));
        ASSERT_STREQ("hello", duk_get_string(m_plugin->context(), -1));
        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "line1"));
        ASSERT_STREQ("world", duk_get_string(m_plugin->context(), -1));
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

TEST_F(TestJsUtil, cut_invalid_data)
{
    try {
        auto ret = duk_peval_string(m_plugin->context(),
            "try {\n"
            "  lines = Irccd.Util.cut(123);\n"
            "} catch (e) {\n"
            "  name = e.name;\n"
            "  message = e.message;\n"
            "}\n"
        );

        if (ret != 0) {
            throw dukx_exception(m_plugin->context(), -1);
        }

        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "name"));
        ASSERT_STREQ("TypeError", duk_get_string(m_plugin->context(), -1));
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
