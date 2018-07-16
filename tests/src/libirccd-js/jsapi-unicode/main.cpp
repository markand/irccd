/*
 * main.cpp -- test Irccd.Unicode API
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

/*
 * /!\ Be sure that this file is kept saved in UTF-8 /!\
 */

#define BOOST_TEST_MODULE "Unicode Javascript API"
#include <boost/test/unit_test.hpp>

#include <irccd/js/unicode_jsapi.hpp>

#include <irccd/test/js_test.hpp>

namespace irccd {

BOOST_FIXTURE_TEST_SUITE(unicode_jsapi_suite, js_test<unicode_jsapi>)

BOOST_AUTO_TEST_CASE(is_letter)
{
    duk_peval_string_noresult(plugin_->get_context(), "result = Irccd.Unicode.isLetter(String('é').charCodeAt(0));");
    BOOST_TEST(duk_get_global_string(plugin_->get_context(), "result"));
    BOOST_TEST(duk_get_boolean(plugin_->get_context(), -1));

    duk_peval_string_noresult(plugin_->get_context(), "result = Irccd.Unicode.isLetter(String('€').charCodeAt(0));");
    BOOST_TEST(duk_get_global_string(plugin_->get_context(), "result"));
    BOOST_TEST(!duk_get_boolean(plugin_->get_context(), -1));
}

BOOST_AUTO_TEST_CASE(is_lower)
{
    duk_peval_string_noresult(plugin_->get_context(), "result = Irccd.Unicode.isLower(String('é').charCodeAt(0));");
    BOOST_TEST(duk_get_global_string(plugin_->get_context(), "result"));
    BOOST_TEST(duk_get_boolean(plugin_->get_context(), -1));

    duk_peval_string_noresult(plugin_->get_context(), "result = Irccd.Unicode.isLower(String('É').charCodeAt(0));");
    BOOST_TEST(duk_get_global_string(plugin_->get_context(), "result"));
    BOOST_TEST(!duk_get_boolean(plugin_->get_context(), -1));
}

BOOST_AUTO_TEST_CASE(is_upper)
{
    duk_peval_string_noresult(plugin_->get_context(), "result = Irccd.Unicode.isUpper(String('É').charCodeAt(0));");
    BOOST_TEST(duk_get_global_string(plugin_->get_context(), "result"));
    BOOST_TEST(duk_get_boolean(plugin_->get_context(), -1));

    duk_peval_string_noresult(plugin_->get_context(), "result = Irccd.Unicode.isUpper(String('é').charCodeAt(0));");
    BOOST_TEST(duk_get_global_string(plugin_->get_context(), "result"));
    BOOST_TEST(!duk_get_boolean(plugin_->get_context(), -1));
}

BOOST_AUTO_TEST_SUITE_END()

} // !irccd
