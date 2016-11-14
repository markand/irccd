/*
 * main.cpp -- test Irccd.File API
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

#include <fstream>

#include <gtest/gtest.h>

#include <irccd/irccd.hpp>
#include <irccd/mod-file.hpp>
#include <irccd/mod-irccd.hpp>
#include <irccd/plugin-js.hpp>
#include <irccd/service.hpp>

using namespace irccd;

class TestJsFile : public testing::Test {
protected:
    Irccd m_irccd;
    std::shared_ptr<JsPlugin> m_plugin;

    TestJsFile()
        : m_plugin(std::make_shared<JsPlugin>("empty", SOURCEDIR "/empty.js"))
    {
        IrccdModule().load(m_irccd, m_plugin);
        FileModule().load(m_irccd, m_plugin);
    }
};

TEST_F(TestJsFile, functionBasename)
{
    try {
        if (duk_peval_string(m_plugin->context(), "result = Irccd.File.basename('/usr/local/etc/irccd.conf');") != 0)
            throw dukx_exception(m_plugin->context(), -1);

        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "result"));
        ASSERT_STREQ("irccd.conf", duk_get_string(m_plugin->context(), -1));
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

TEST_F(TestJsFile, functionDirname)
{
    try {
        duk_peval_string(m_plugin->context(), "result = Irccd.File.dirname('/usr/local/etc/irccd.conf');");

        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "result"));
        ASSERT_STREQ("/usr/local/etc", duk_get_string(m_plugin->context(), -1));
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

TEST_F(TestJsFile, functionExists)
{
    try {
        duk_push_string(m_plugin->context(), IRCCD_TESTS_DIRECTORY);
        duk_put_global_string(m_plugin->context(), "directory");
        duk_peval_string(m_plugin->context(), "result = Irccd.File.exists(directory + '/file.txt')");

        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "result"));
        ASSERT_TRUE(duk_get_boolean(m_plugin->context(), -1));
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

TEST_F(TestJsFile, functionExists2)
{
    try {
        duk_peval_string(m_plugin->context(), "result = Irccd.File.exists('file_which_does_not_exist.txt')");

        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "result"));
        ASSERT_FALSE(duk_get_boolean(m_plugin->context(), -1));
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

TEST_F(TestJsFile, functionRemove)
{
    // First create a dummy file
    std::ofstream("test-js-fs.remove");

    try {
        if (duk_peval_string(m_plugin->context(), "Irccd.File.remove('test-js-fs.remove');") != 0)
            throw dukx_exception(m_plugin->context(), -1);
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }

    std::ifstream in("test-js-fs.remove");

    ASSERT_FALSE(in.is_open());
}

TEST_F(TestJsFile, methodBasename)
{
    try {
        duk_push_string(m_plugin->context(), IRCCD_TESTS_DIRECTORY);
        duk_put_global_string(m_plugin->context(), "directory");

        auto ret = duk_peval_string(m_plugin->context(),
            "f = new Irccd.File(directory + '/level-1/file-1.txt', 'r');"
            "result = f.basename();"
        );

        if (ret != 0)
            throw dukx_exception(m_plugin->context(), -1);

        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "result"));
        ASSERT_STREQ("file-1.txt", duk_get_string(m_plugin->context(), -1));
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

TEST_F(TestJsFile, methodBasenameClosed)
{
    try {
        duk_push_string(m_plugin->context(), IRCCD_TESTS_DIRECTORY);
        duk_put_global_string(m_plugin->context(), "directory");

        auto ret = duk_peval_string(m_plugin->context(),
            "f = new Irccd.File(directory + '/level-1/file-1.txt', 'r');"
            "f.close();"
            "result = f.basename();"
        );

        if (ret != 0)
            throw dukx_exception(m_plugin->context(), -1);

        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "result"));
        ASSERT_STREQ("file-1.txt", duk_get_string(m_plugin->context(), -1));
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

TEST_F(TestJsFile, methodDirname)
{
    try {
        duk_push_string(m_plugin->context(), IRCCD_TESTS_DIRECTORY);
        duk_put_global_string(m_plugin->context(), "directory");

        auto ret = duk_peval_string(m_plugin->context(),
            "f = new Irccd.File(directory + '/level-1/file-1.txt', 'r');"
            "result = f.dirname();"
        );

        if (ret != 0)
            throw dukx_exception(m_plugin->context(), -1);

        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "result"));
        ASSERT_STREQ(IRCCD_TESTS_DIRECTORY "/level-1", duk_get_string(m_plugin->context(), -1));
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

TEST_F(TestJsFile, methodDirnameClosed)
{
    try {
        duk_push_string(m_plugin->context(), IRCCD_TESTS_DIRECTORY);
        duk_put_global_string(m_plugin->context(), "directory");

        auto ret = duk_peval_string(m_plugin->context(),
            "f = new Irccd.File(directory + '/level-1/file-1.txt', 'r');"
            "f.close();"
            "result = f.dirname();"
        );

        if (ret != 0)
            throw dukx_exception(m_plugin->context(), -1);

        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "result"));
        ASSERT_STREQ(IRCCD_TESTS_DIRECTORY "/level-1", duk_get_string(m_plugin->context(), -1));
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

TEST_F(TestJsFile, methodLines)
{
    try {
        duk_push_string(m_plugin->context(), IRCCD_TESTS_DIRECTORY);
        duk_put_global_string(m_plugin->context(), "directory");

        auto ret = duk_peval_string(m_plugin->context(),
            "result = new Irccd.File(directory + '/lines.txt', 'r').lines();"
        );

        if (ret != 0)
            throw dukx_exception(m_plugin->context(), -1);

        std::vector<std::string> expected{"a", "b", "c"};

        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "result"));
        ASSERT_EQ(expected, dukx_get_array(m_plugin->context(), -1, dukx_get_std_string));
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

TEST_F(TestJsFile, methodSeek1)
{
    try {
        duk_push_string(m_plugin->context(), IRCCD_TESTS_DIRECTORY);
        duk_put_global_string(m_plugin->context(), "directory");

        auto ret = duk_peval_string(m_plugin->context(),
            "f = new Irccd.File(directory + '/file.txt', 'r');"
            "f.seek(Irccd.File.SeekSet, 4);"
            "result = f.read(1);"
        );

        if (ret != 0)
            throw dukx_exception(m_plugin->context(), -1);

        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "result"));
        ASSERT_EQ(".", dukx_get_std_string(m_plugin->context(), -1));
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

TEST_F(TestJsFile, methodSeek1Closed)
{
    try {
        duk_push_string(m_plugin->context(), IRCCD_TESTS_DIRECTORY);
        duk_put_global_string(m_plugin->context(), "directory");

        auto ret = duk_peval_string(m_plugin->context(),
            "f = new Irccd.File(directory + '/file.txt', 'r');"
            "f.close();"
            "f.seek(Irccd.File.SeekSet, 4);"
            "result = f.read(1);"
            "result = typeof (result) === \"undefined\";"
        );

        if (ret != 0)
            throw dukx_exception(m_plugin->context(), -1);

        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "result"));
        ASSERT_TRUE(duk_get_boolean(m_plugin->context(), -1));
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

TEST_F(TestJsFile, methodSeek2)
{
    try {
        duk_push_string(m_plugin->context(), IRCCD_TESTS_DIRECTORY);
        duk_put_global_string(m_plugin->context(), "directory");

        auto ret = duk_peval_string(m_plugin->context(),
            "f = new Irccd.File(directory + '/file.txt', 'r');"
            "f.seek(Irccd.File.SeekSet, 2);"
            "f.seek(Irccd.File.SeekCur, 2);"
            "result = f.read(1);"
        );

        if (ret != 0)
            throw dukx_exception(m_plugin->context(), -1);

        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "result"));
        ASSERT_EQ(".", dukx_get_std_string(m_plugin->context(), -1));
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

TEST_F(TestJsFile, methodSeek2Closed)
{
    try {
        duk_push_string(m_plugin->context(), IRCCD_TESTS_DIRECTORY);
        duk_put_global_string(m_plugin->context(), "directory");

        auto ret = duk_peval_string(m_plugin->context(),
            "f = new Irccd.File(directory + '/file.txt', 'r');"
            "f.close();"
            "f.seek(Irccd.File.SeekSet, 2);"
            "f.seek(Irccd.File.SeekCur, 2);"
            "result = f.read(1);"
            "result = typeof (result) === \"undefined\";"
        );

        if (ret != 0)
            throw dukx_exception(m_plugin->context(), -1);

        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "result"));
        ASSERT_TRUE(duk_get_boolean(m_plugin->context(), -1));
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

TEST_F(TestJsFile, methodSeek3)
{
    try {
        duk_push_string(m_plugin->context(), IRCCD_TESTS_DIRECTORY);
        duk_put_global_string(m_plugin->context(), "directory");

        auto ret = duk_peval_string(m_plugin->context(),
            "f = new Irccd.File(directory + '/file.txt', 'r');"
            "f.seek(Irccd.File.SeekEnd, -2);"
            "result = f.read(1);"
        );

        if (ret != 0)
            throw dukx_exception(m_plugin->context(), -1);

        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "result"));
        ASSERT_STREQ("x", duk_get_string(m_plugin->context(), -1));
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

TEST_F(TestJsFile, methodSeek3Closed)
{
    try {
        duk_push_string(m_plugin->context(), IRCCD_TESTS_DIRECTORY);
        duk_put_global_string(m_plugin->context(), "directory");

        auto ret = duk_peval_string(m_plugin->context(),
            "f = new Irccd.File(directory + '/file.txt', 'r');"
            "f.close();"
            "f.seek(Irccd.File.SeekEnd, -2);"
            "result = f.read(1);"
            "result = typeof (result) === \"undefined\";"
        );

        if (ret != 0)
            throw dukx_exception(m_plugin->context(), -1);

        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "result"));
        ASSERT_TRUE(duk_get_boolean(m_plugin->context(), -1));
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

TEST_F(TestJsFile, methodRead1)
{
    try {
        duk_push_string(m_plugin->context(), IRCCD_TESTS_DIRECTORY);
        duk_put_global_string(m_plugin->context(), "directory");

        auto ret = duk_peval_string(m_plugin->context(),
            "f = new Irccd.File(directory + '/file.txt', 'r');"
            "result = f.read();"
        );

        if (ret != 0)
            throw dukx_exception(m_plugin->context(), -1);

        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "result"));
        ASSERT_STREQ("file.txt", duk_get_string(m_plugin->context(), -1));
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

TEST_F(TestJsFile, methodReadline)
{
    try {
        duk_push_string(m_plugin->context(), IRCCD_TESTS_DIRECTORY);
        duk_put_global_string(m_plugin->context(), "directory");

        auto ret = duk_peval_string(m_plugin->context(),
            "result = [];"
            "f = new Irccd.File(directory + '/lines.txt', 'r');"
            "for (var s; s = f.readline(); ) {"
            "  result.push(s);"
            "}"
        );

        if (ret != 0)
            throw dukx_exception(m_plugin->context(), -1);

        std::vector<std::string> expected{"a", "b", "c"};

        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "result"));
        ASSERT_EQ(expected, dukx_get_array(m_plugin->context(), -1, dukx_get_std_string));
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

TEST_F(TestJsFile, methodReadlineClosed)
{
    try {
        duk_push_string(m_plugin->context(), IRCCD_TESTS_DIRECTORY);
        duk_put_global_string(m_plugin->context(), "directory");

        auto ret = duk_peval_string(m_plugin->context(),
            "result = [];"
            "f = new Irccd.File(directory + '/lines.txt', 'r');"
            "f.close();"
            "for (var s; s = f.readline(); ) {"
            "  result.push(s);"
            "}"
        );

        if (ret != 0)
            throw dukx_exception(m_plugin->context(), -1);

        std::vector<std::string> expected;

        ASSERT_TRUE(duk_get_global_string(m_plugin->context(), "result"));
        ASSERT_EQ(expected, dukx_get_array(m_plugin->context(), -1, dukx_get_std_string));
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
