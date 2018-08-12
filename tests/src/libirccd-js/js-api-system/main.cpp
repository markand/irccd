/*
 * main.cpp -- test Irccd.System API
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

#define BOOST_TEST_MODULE "System Javascript API"
#include <boost/test/unit_test.hpp>

#include <irccd/system.hpp>

#include <irccd/test/js_fixture.hpp>

using namespace irccd::js;
using namespace irccd::test;

namespace irccd {

namespace {

BOOST_FIXTURE_TEST_SUITE(system_js_api_suite, js_fixture)

BOOST_AUTO_TEST_CASE(home)
{
    duk_peval_string_noresult(plugin_->get_context(), "result = Irccd.System.home();");

    BOOST_TEST(duk_get_global_string(plugin_->get_context(),"result"));
    BOOST_TEST(duk_get_string(plugin_->get_context(), -1) == sys::home());
}

#if defined(IRCCD_HAVE_POPEN)

BOOST_AUTO_TEST_CASE(popen)
{
    auto ret = duk_peval_string(plugin_->get_context(),
        "f = Irccd.System.popen(\"" IRCCD_EXECUTABLE " --version\", \"r\");"
        "r = f.readline();"
    );

    if (ret != 0)
        throw duk::get_stack(plugin_->get_context(), -1);

    BOOST_TEST(duk_get_global_string(plugin_->get_context(), "r"));
    BOOST_TEST(duk_get_string(plugin_->get_context(), -1) == IRCCD_VERSION);
}

#endif

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
