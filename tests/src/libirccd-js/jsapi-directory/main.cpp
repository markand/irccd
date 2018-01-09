/*
 * main.cpp -- test Irccd.Directory API
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

#define BOOST_TEST_MODULE "Directory Javascript API"
#include <boost/test/unit_test.hpp>

#include <irccd/js/directory_jsapi.hpp>

#include <irccd/test/js_test.hpp>

namespace irccd {

BOOST_FIXTURE_TEST_SUITE(directory_jsapi_suite, js_test<directory_jsapi>)

BOOST_AUTO_TEST_CASE(constructor)
{
    const std::string script(
        "d = new Irccd.Directory(CMAKE_SOURCE_DIR + \"/tests/root\");"
        "p = d.path;"
        "l = d.entries.length;"
    );

    if (duk_peval_string(plugin_->context(), script.c_str()) != 0)
        throw dukx_stack(plugin_->context(), -1);

    duk_get_global_string(plugin_->context(), "l");
    BOOST_TEST(duk_get_int(plugin_->context(), -1) == 3);
}

BOOST_AUTO_TEST_SUITE_END()

} // !irccd
