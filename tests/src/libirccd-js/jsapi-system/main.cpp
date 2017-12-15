/*
 * main.cpp -- test Irccd.System API
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

#define BOOST_TEST_MODULE "System Javascript API"
#include <boost/test/unit_test.hpp>

#include <irccd/system.hpp>

#include <irccd/daemon/irccd.hpp>

#include <irccd/js/file_jsapi.hpp>
#include <irccd/js/system_jsapi.hpp>

#include <irccd/test/js_test.hpp>

namespace irccd {

using fixture = js_test<file_jsapi, system_jsapi>;

BOOST_FIXTURE_TEST_SUITE(system_jsapi_suite, fixture)

BOOST_AUTO_TEST_CASE(home)
{
    duk_peval_string_noresult(plugin_->context(), "result = Irccd.System.home();");

    BOOST_TEST(duk_get_global_string(plugin_->context(),"result"));
    BOOST_TEST(duk_get_string(plugin_->context(), -1) == sys::home());
}

#if defined(HAVE_POPEN)

BOOST_AUTO_TEST_CASE(popen)
{
    auto ret = duk_peval_string(plugin_->context(),
        "f = Irccd.System.popen(\"" IRCCD_EXECUTABLE " --version\", \"r\");"
        "r = f.readline();"
    );

    if (ret != 0)
        throw dukx_stack(plugin_->context(), -1);

    BOOST_TEST(duk_get_global_string(plugin_->context(), "r"));
    BOOST_TEST(duk_get_string(plugin_->context(), -1) == IRCCD_VERSION);
}

#endif

BOOST_AUTO_TEST_SUITE_END()

} // !irccd
