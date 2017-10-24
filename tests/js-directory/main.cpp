/*
 * main.cpp -- test Irccd.Directory API
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

#define BOOST_TEST_MODULE "Directory Javascript API"
#include <boost/test/unit_test.hpp>

#include <irccd/js_directory_module.hpp>

#include <js_test.hpp>

namespace irccd {

class directory_test : public js_test<js_directory_module> {
public:
};

BOOST_FIXTURE_TEST_SUITE(js_directory_suite, directory_test)

BOOST_AUTO_TEST_CASE(constructor)
{
    const std::string script(
        "d = new Irccd.Directory(TESTS_BINARY_DIR + \"/level-1\");"
        "p = d.path;"
        "e = d.entries;"
    );

    if (duk_peval_string(plugin_->context(), script.c_str()) != 0)
        throw dukx_exception(plugin_->context(), -1);

    duk_get_global_string(plugin_->context(), "l");
    BOOST_TEST(duk_get_int(plugin_->context(), -1) == 2);
}

BOOST_AUTO_TEST_SUITE_END()

} // !irccd