/*
 * main.cpp -- test Irccd.Util API
 *
 * Copyright (c) 2013-2018 David Demelier <markand@malikania.fr>
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

#define BOOST_TEST_MODULE "Unicode Javascript API"
#include <boost/test/unit_test.hpp>

#include <irccd/test/js_fixture.hpp>

using namespace irccd::js;
using namespace irccd::test;

namespace irccd {

namespace {

BOOST_FIXTURE_TEST_SUITE(util_js_api_suite, js_fixture)

/*
 * Irccd.Util misc.
 * ------------------------------------------------------------------
 */

BOOST_AUTO_TEST_CASE(format_simple)
{
    const auto ret = duk_peval_string(plugin_->get_context(),
        "result = Irccd.Util.format(\"#{target}\", { target: \"markand\" })"
    );

    if (ret != 0)
        throw duk::get_stack(plugin_->get_context(), -1);

    BOOST_TEST(duk_get_global_string(plugin_->get_context(), "result"));
    BOOST_TEST(duk_get_string(plugin_->get_context(), -1) == "markand");
}

BOOST_AUTO_TEST_CASE(splituser)
{
    if (duk_peval_string(plugin_->get_context(), "result = Irccd.Util.splituser(\"user!~user@hyper/super/host\");") != 0)
        throw duk::get_stack(plugin_->get_context(), -1);

    BOOST_TEST(duk_get_global_string(plugin_->get_context(), "result"));
    BOOST_TEST(duk_get_string(plugin_->get_context(), -1) == "user");
}

BOOST_AUTO_TEST_CASE(splithost)
{
    if (duk_peval_string(plugin_->get_context(), "result = Irccd.Util.splithost(\"user!~user@hyper/super/host\");") != 0)
        throw duk::get_stack(plugin_->get_context(), -1);

    BOOST_TEST(duk_get_global_string(plugin_->get_context(), "result"));
    BOOST_TEST(duk_get_string(plugin_->get_context(), -1) == "~user@hyper/super/host");
}

/*
 * Irccd.Util.cut.
 * ------------------------------------------------------------------
 */

BOOST_AUTO_TEST_CASE(cut_string_simple)
{
    const auto ret = duk_peval_string(plugin_->get_context(),
        "lines = Irccd.Util.cut('hello world');\n"
        "line0 = lines[0];\n"
    );

    if (ret != 0)
        throw duk::get_stack(plugin_->get_context(), -1);

    BOOST_TEST(duk_get_global_string(plugin_->get_context(), "line0"));
    BOOST_TEST(duk_get_string(plugin_->get_context(), -1) == "hello world");
}

BOOST_AUTO_TEST_CASE(cut_string_double)
{
    const auto ret = duk_peval_string(plugin_->get_context(),
        "lines = Irccd.Util.cut('hello world', 5);\n"
        "line0 = lines[0];\n"
        "line1 = lines[1];\n"
    );

    if (ret != 0)
        throw duk::get_stack(plugin_->get_context(), -1);

    BOOST_TEST(duk_get_global_string(plugin_->get_context(), "line0"));
    BOOST_TEST(duk_get_string(plugin_->get_context(), -1) == "hello");
    BOOST_TEST(duk_get_global_string(plugin_->get_context(), "line1"));
    BOOST_TEST(duk_get_string(plugin_->get_context(), -1) == "world");
}

BOOST_AUTO_TEST_CASE(cut_string_dirty)
{
    const auto ret = duk_peval_string(plugin_->get_context(),
        "lines = Irccd.Util.cut('     hello    world     ', 5);\n"
        "line0 = lines[0];\n"
        "line1 = lines[1];\n"
    );

    if (ret != 0)
        throw duk::get_stack(plugin_->get_context(), -1);

    BOOST_TEST(duk_get_global_string(plugin_->get_context(), "line0"));
    BOOST_TEST(duk_get_string(plugin_->get_context(), -1) == "hello");
    BOOST_TEST(duk_get_global_string(plugin_->get_context(), "line1"));
    BOOST_TEST(duk_get_string(plugin_->get_context(), -1) == "world");
}

BOOST_AUTO_TEST_CASE(cut_string_too_much_lines)
{
    const auto ret = duk_peval_string(plugin_->get_context(),
        "lines = Irccd.Util.cut('abc def ghi jkl', 3, 3);\n"
    );

    if (ret != 0)
        throw duk::get_stack(plugin_->get_context(), -1);

    BOOST_TEST(duk_get_global_string(plugin_->get_context(), "lines"));
    BOOST_TEST(duk_is_undefined(plugin_->get_context(), -1));
}

BOOST_AUTO_TEST_CASE(cut_string_token_too_big)
{
    const auto ret = duk_peval_string(plugin_->get_context(),
        "try {\n"
        "  lines = Irccd.Util.cut('hello world', 3);\n"
        "} catch (e) {\n"
        "  name = e.name;\n"
        "  message = e.message;\n"
        "}\n"
    );

    if (ret != 0)
        throw duk::get_stack(plugin_->get_context(), -1);

    BOOST_TEST(duk_get_global_string(plugin_->get_context(), "name"));
    BOOST_TEST(duk_get_string(plugin_->get_context(), -1) == "RangeError");
    BOOST_TEST(duk_get_global_string(plugin_->get_context(), "message"));
    BOOST_TEST(duk_get_string(plugin_->get_context(), -1) == "word 'hello' could not fit in maxc limit (3)");
}

BOOST_AUTO_TEST_CASE(cut_string_negative_maxc)
{
    const auto ret = duk_peval_string(plugin_->get_context(),
        "try {\n"
        "  lines = Irccd.Util.cut('hello world', -3);\n"
        "} catch (e) {\n"
        "  name = e.name;\n"
        "  message = e.message;\n"
        "}\n"
    );

    if (ret != 0)
        throw duk::get_stack(plugin_->get_context(), -1);

    BOOST_TEST(duk_get_global_string(plugin_->get_context(), "name"));
    BOOST_TEST(duk_get_string(plugin_->get_context(), -1) == "RangeError");
    BOOST_TEST(duk_get_global_string(plugin_->get_context(), "message"));
    BOOST_TEST(duk_get_string(plugin_->get_context(), -1) == "argument 1 (maxc) must be positive");
}

BOOST_AUTO_TEST_CASE(cut_string_negative_maxl)
{
    const auto ret = duk_peval_string(plugin_->get_context(),
        "try {\n"
        "  lines = Irccd.Util.cut('hello world', undefined, -1);\n"
        "} catch (e) {\n"
        "  name = e.name;\n"
        "  message = e.message;\n"
        "}\n"
    );

    if (ret != 0)
        throw duk::get_stack(plugin_->get_context(), -1);

    BOOST_TEST(duk_get_global_string(plugin_->get_context(), "name"));
    BOOST_TEST(duk_get_string(plugin_->get_context(), -1) == "RangeError");
    BOOST_TEST(duk_get_global_string(plugin_->get_context(), "message"));
    BOOST_TEST(duk_get_string(plugin_->get_context(), -1) == "argument 2 (maxl) must be positive");
}

BOOST_AUTO_TEST_CASE(cut_array_simple)
{
    const auto ret = duk_peval_string(plugin_->get_context(),
        "lines = Irccd.Util.cut([ 'hello', 'world' ]);\n"
        "line0 = lines[0];\n"
    );

    if (ret != 0)
        throw duk::get_stack(plugin_->get_context(), -1);

    BOOST_TEST(duk_get_global_string(plugin_->get_context(), "line0"));
    BOOST_TEST(duk_get_string(plugin_->get_context(), -1) == "hello world");
}

BOOST_AUTO_TEST_CASE(cut_array_double)
{
    const auto ret = duk_peval_string(plugin_->get_context(),
        "lines = Irccd.Util.cut([ 'hello', 'world' ], 5);\n"
        "line0 = lines[0];\n"
        "line1 = lines[1];\n"
    );

    if (ret != 0)
        throw duk::get_stack(plugin_->get_context(), -1);

    BOOST_TEST(duk_get_global_string(plugin_->get_context(), "line0"));
    BOOST_TEST(duk_get_string(plugin_->get_context(), -1) == "hello");
    BOOST_TEST(duk_get_global_string(plugin_->get_context(), "line1"));
    BOOST_TEST(duk_get_string(plugin_->get_context(), -1) == "world");
}

BOOST_AUTO_TEST_CASE(cut_array_dirty)
{
    const auto ret = duk_peval_string(plugin_->get_context(),
        "lines = Irccd.Util.cut([ '   ', ' hello  ', '  world ', '    '], 5);\n"
        "line0 = lines[0];\n"
        "line1 = lines[1];\n"
    );

    if (ret != 0)
        throw duk::get_stack(plugin_->get_context(), -1);

    BOOST_TEST(duk_get_global_string(plugin_->get_context(), "line0"));
    BOOST_TEST(duk_get_string(plugin_->get_context(), -1) == "hello");
    BOOST_TEST(duk_get_global_string(plugin_->get_context(), "line1"));
    BOOST_TEST(duk_get_string(plugin_->get_context(), -1) == "world");
}

BOOST_AUTO_TEST_CASE(cut_invalid_data)
{
    const auto ret = duk_peval_string(plugin_->get_context(),
        "try {\n"
        "  lines = Irccd.Util.cut(123);\n"
        "} catch (e) {\n"
        "  name = e.name;\n"
        "  message = e.message;\n"
        "}\n"
    );

    if (ret != 0)
        throw duk::get_stack(plugin_->get_context(), -1);

    BOOST_TEST(duk_get_global_string(plugin_->get_context(), "name"));
    BOOST_TEST(duk_get_string(plugin_->get_context(), -1) == "TypeError");
}

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
