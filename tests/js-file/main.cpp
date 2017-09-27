/*
 * main.cpp -- test Irccd.File API
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

#define BOOST_TEST_MODULE "File Javascript API"
#include <boost/test/unit_test.hpp>

#include <fstream>

#include <irccd/js_file_module.hpp>

#include <js_test.hpp>

namespace irccd {

BOOST_FIXTURE_TEST_SUITE(js_file_suite, js_test<js_file_module>)

BOOST_AUTO_TEST_CASE(function_basename)
{
    if (duk_peval_string(plugin_->context(), "result = Irccd.File.basename('/usr/local/etc/irccd.conf');"))
        throw dukx_exception(plugin_->context(), -1);

    BOOST_TEST(duk_get_global_string(plugin_->context(), "result"));
    BOOST_TEST("irccd.conf" == duk_get_string(plugin_->context(), -1));
}

BOOST_AUTO_TEST_CASE(function_dirname)
{
    if (duk_peval_string(plugin_->context(), "result = Irccd.File.dirname('/usr/local/etc/irccd.conf');"))
        throw dukx_exception(plugin_->context(), -1);

    BOOST_TEST(duk_get_global_string(plugin_->context(), "result"));
    BOOST_TEST("/usr/local/etc" == duk_get_string(plugin_->context(), -1));
}


BOOST_AUTO_TEST_CASE(function_exists)
{
    duk_push_string(plugin_->context(), IRCCD_TESTS_DIRECTORY);
    duk_put_global_string(plugin_->context(), "directory");

    if (duk_peval_string(plugin_->context(), "result = Irccd.File.exists(directory + '/file.txt')"))
        throw dukx_exception(plugin_->context(), -1);
    
    BOOST_TEST(duk_get_global_string(plugin_->context(), "result"));
    BOOST_TEST(duk_get_boolean(plugin_->context(), -1));
}

BOOST_AUTO_TEST_CASE(function_exists2)
{
    if (duk_peval_string(plugin_->context(), "result = Irccd.File.exists('file_which_does_not_exist.txt')"))
        throw dukx_exception(plugin_->context(), -1);
    
    BOOST_TEST(duk_get_global_string(plugin_->context(), "result"));
    BOOST_TEST(!duk_get_boolean(plugin_->context(), -1));
}

BOOST_AUTO_TEST_CASE(function_remove)
{
    // First create a dummy file
    std::ofstream("test-js-fs.remove");

    if (duk_peval_string(plugin_->context(), "Irccd.File.remove('test-js-fs.remove');") != 0)
        throw dukx_exception(plugin_->context(), -1);

    std::ifstream in("test-js-fs.remove");

    BOOST_TEST(!in.is_open());
}

BOOST_AUTO_TEST_CASE(method_basename)
{
    duk_push_string(plugin_->context(), IRCCD_TESTS_DIRECTORY);
    duk_put_global_string(plugin_->context(), "directory");

    auto ret = duk_peval_string(plugin_->context(),
        "f = new Irccd.File(directory + '/level-1/file-1.txt', 'r');"
        "result = f.basename();"
    );

    if (ret != 0)
        throw dukx_exception(plugin_->context(), -1);

    BOOST_TEST(duk_get_global_string(plugin_->context(), "result"));
    BOOST_TEST("file-1.txt" == duk_get_string(plugin_->context(), -1));
}

BOOST_AUTO_TEST_CASE(method_basename_closed)
{
    duk_push_string(plugin_->context(), IRCCD_TESTS_DIRECTORY);
    duk_put_global_string(plugin_->context(), "directory");

    auto ret = duk_peval_string(plugin_->context(),
        "f = new Irccd.File(directory + '/level-1/file-1.txt', 'r');"
        "f.close();"
        "result = f.basename();"
    );

    if (ret != 0)
        throw dukx_exception(plugin_->context(), -1);

    BOOST_TEST(duk_get_global_string(plugin_->context(), "result"));
    BOOST_TEST("file-1.txt" == duk_get_string(plugin_->context(), -1));
}

BOOST_AUTO_TEST_CASE(method_dirname)
{
    duk_push_string(plugin_->context(), IRCCD_TESTS_DIRECTORY);
    duk_put_global_string(plugin_->context(), "directory");

    auto ret = duk_peval_string(plugin_->context(),
        "f = new Irccd.File(directory + '/level-1/file-1.txt', 'r');"
        "result = f.dirname();"
    );

    if (ret != 0)
        throw dukx_exception(plugin_->context(), -1);

    BOOST_TEST(duk_get_global_string(plugin_->context(), "result"));
    BOOST_TEST(IRCCD_TESTS_DIRECTORY "/level-1" == duk_get_string(plugin_->context(), -1));
}

BOOST_AUTO_TEST_CASE(method_dirname_closed)
{
    duk_push_string(plugin_->context(), IRCCD_TESTS_DIRECTORY);
    duk_put_global_string(plugin_->context(), "directory");

    auto ret = duk_peval_string(plugin_->context(),
        "f = new Irccd.File(directory + '/level-1/file-1.txt', 'r');"
        "f.close();"
        "result = f.dirname();"
    );

    if (ret != 0)
        throw dukx_exception(plugin_->context(), -1);

    BOOST_TEST(duk_get_global_string(plugin_->context(), "result"));
    BOOST_TEST(IRCCD_TESTS_DIRECTORY "/level-1" == duk_get_string(plugin_->context(), -1));
}

BOOST_AUTO_TEST_CASE(method_lines)
{
    duk_push_string(plugin_->context(), IRCCD_TESTS_DIRECTORY);
    duk_put_global_string(plugin_->context(), "directory");

    auto ret = duk_peval_string(plugin_->context(),
        "result = new Irccd.File(directory + '/lines.txt', 'r').lines();"
    );

    if (ret != 0)
        throw dukx_exception(plugin_->context(), -1);

    std::vector<std::string> expected{"a", "b", "c"};

    BOOST_TEST(duk_get_global_string(plugin_->context(), "result"));
    BOOST_TEST(expected == dukx_get_array(plugin_->context(), -1, dukx_get_std_string));
}

BOOST_AUTO_TEST_CASE(method_seek1)
{
    duk_push_string(plugin_->context(), IRCCD_TESTS_DIRECTORY);
    duk_put_global_string(plugin_->context(), "directory");

    auto ret = duk_peval_string(plugin_->context(),
        "f = new Irccd.File(directory + '/file.txt', 'r');"
        "f.seek(Irccd.File.SeekSet, 4);"
        "result = f.read(1);"
    );

    if (ret != 0)
        throw dukx_exception(plugin_->context(), -1);

    BOOST_TEST(duk_get_global_string(plugin_->context(), "result"));
    BOOST_TEST(".", dukx_get_std_string(plugin_->context(), -1));
}

BOOST_AUTO_TEST_CASE(method_seek1_closed)
{
    duk_push_string(plugin_->context(), IRCCD_TESTS_DIRECTORY);
    duk_put_global_string(plugin_->context(), "directory");

    auto ret = duk_peval_string(plugin_->context(),
        "f = new Irccd.File(directory + '/file.txt', 'r');"
        "f.close();"
        "f.seek(Irccd.File.SeekSet, 4);"
        "result = f.read(1);"
        "result = typeof (result) === \"undefined\";"
    );

    if (ret != 0)
        throw dukx_exception(plugin_->context(), -1);

    BOOST_TEST(duk_get_global_string(plugin_->context(), "result"));
    BOOST_TEST(duk_get_boolean(plugin_->context(), -1));
}

BOOST_AUTO_TEST_CASE(method_seek2)
{
    duk_push_string(plugin_->context(), IRCCD_TESTS_DIRECTORY);
    duk_put_global_string(plugin_->context(), "directory");

    auto ret = duk_peval_string(plugin_->context(),
        "f = new Irccd.File(directory + '/file.txt', 'r');"
        "f.seek(Irccd.File.SeekSet, 2);"
        "f.seek(Irccd.File.SeekCur, 2);"
        "result = f.read(1);"
    );

    if (ret != 0)
        throw dukx_exception(plugin_->context(), -1);

    BOOST_TEST(duk_get_global_string(plugin_->context(), "result"));
    BOOST_TEST("." == dukx_get_std_string(plugin_->context(), -1));
}

BOOST_AUTO_TEST_CASE(method_seek2c_losed)
{
    duk_push_string(plugin_->context(), IRCCD_TESTS_DIRECTORY);
    duk_put_global_string(plugin_->context(), "directory");

    auto ret = duk_peval_string(plugin_->context(),
        "f = new Irccd.File(directory + '/file.txt', 'r');"
        "f.close();"
        "f.seek(Irccd.File.SeekSet, 2);"
        "f.seek(Irccd.File.SeekCur, 2);"
        "result = f.read(1);"
        "result = typeof (result) === \"undefined\";"
    );

    if (ret != 0)
        throw dukx_exception(plugin_->context(), -1);

    BOOST_TEST(duk_get_global_string(plugin_->context(), "result"));
    BOOST_TEST(duk_get_boolean(plugin_->context(), -1));
}

BOOST_AUTO_TEST_CASE(method_seek3)
{
    duk_push_string(plugin_->context(), IRCCD_TESTS_DIRECTORY);
    duk_put_global_string(plugin_->context(), "directory");

    auto ret = duk_peval_string(plugin_->context(),
        "f = new Irccd.File(directory + '/file.txt', 'r');"
        "f.seek(Irccd.File.SeekEnd, -2);"
        "result = f.read(1);"
    );

    if (ret != 0)
        throw dukx_exception(plugin_->context(), -1);

    BOOST_TEST(duk_get_global_string(plugin_->context(), "result"));
    BOOST_TEST("x" == duk_get_string(plugin_->context(), -1));
}

BOOST_AUTO_TEST_CASE(method_seek3_closed)
{
    duk_push_string(plugin_->context(), IRCCD_TESTS_DIRECTORY);
    duk_put_global_string(plugin_->context(), "directory");

    auto ret = duk_peval_string(plugin_->context(),
        "f = new Irccd.File(directory + '/file.txt', 'r');"
        "f.close();"
        "f.seek(Irccd.File.SeekEnd, -2);"
        "result = f.read(1);"
        "result = typeof (result) === \"undefined\";"
    );

    if (ret != 0)
        throw dukx_exception(plugin_->context(), -1);

    BOOST_TEST(duk_get_global_string(plugin_->context(), "result"));
    BOOST_TEST(duk_get_boolean(plugin_->context(), -1));
}

BOOST_AUTO_TEST_CASE(method_read1)
{
    duk_push_string(plugin_->context(), IRCCD_TESTS_DIRECTORY);
    duk_put_global_string(plugin_->context(), "directory");

    auto ret = duk_peval_string(plugin_->context(),
        "f = new Irccd.File(directory + '/file.txt', 'r');"
        "result = f.read();"
    );

    if (ret != 0)
        throw dukx_exception(plugin_->context(), -1);

    BOOST_TEST(duk_get_global_string(plugin_->context(), "result"));
    BOOST_TEST("file.txt" == duk_get_string(plugin_->context(), -1));
}

BOOST_AUTO_TEST_CASE(method_readline)
{
    duk_push_string(plugin_->context(), IRCCD_TESTS_DIRECTORY);
    duk_put_global_string(plugin_->context(), "directory");

    auto ret = duk_peval_string(plugin_->context(),
        "result = [];"
        "f = new Irccd.File(directory + '/lines.txt', 'r');"
        "for (var s; s = f.readline(); ) {"
        "  result.push(s);"
        "}"
    );

    if (ret != 0)
        throw dukx_exception(plugin_->context(), -1);

    std::vector<std::string> expected{"a", "b", "c"};

    BOOST_TEST(duk_get_global_string(plugin_->context(), "result"));
    BOOST_TEST(expected == dukx_get_array(plugin_->context(), -1, dukx_get_std_string));
}

BOOST_AUTO_TEST_CASE(methodReadlineClosed)
{
    duk_push_string(plugin_->context(), IRCCD_TESTS_DIRECTORY);
    duk_put_global_string(plugin_->context(), "directory");

    auto ret = duk_peval_string(plugin_->context(),
        "result = [];"
        "f = new Irccd.File(directory + '/lines.txt', 'r');"
        "f.close();"
        "for (var s; s = f.readline(); ) {"
        "  result.push(s);"
        "}"
    );

    if (ret != 0)
        throw dukx_exception(plugin_->context(), -1);

    std::vector<std::string> expected;

    BOOST_TEST(duk_get_global_string(plugin_->context(), "result"));
    BOOST_TEST(expected == dukx_get_array(plugin_->context(), -1, dukx_get_std_string));
}

BOOST_AUTO_TEST_SUITE_END()

} // !irccd
