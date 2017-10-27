/*
 * main.cpp -- test duktape.hpp extensions
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

#define BOOST_TEST_MODULE "Javascript"
#include <boost/test/unit_test.hpp>

#include <duktape.hpp>

#include <irccd/fs_util.hpp>
#include <irccd/util.hpp>

namespace irccd {

class test {
protected:
    UniqueContext ctx_;
};

/*
 * dukx_peval_file
 * ------------------------------------------------------------------
 */

BOOST_FIXTURE_TEST_SUITE(test_suite, test)

BOOST_AUTO_TEST_CASE(no_file)
{
    BOOST_REQUIRE_THROW(dukx_peval_file(ctx_, "nonexistent"), Exception);

    try {
        dukx_peval_file(ctx_, "nonexistent");
    } catch (const Exception& ex) {
        BOOST_REQUIRE_EQUAL("Error", ex.name);
        BOOST_REQUIRE_EQUAL("nonexistent", ex.fileName);
    }
}

BOOST_AUTO_TEST_CASE(syntax_error)
{
    BOOST_REQUIRE_THROW(dukx_peval_file(ctx_, SOURCEDIR "/syntax-error.js"), Exception);

    try {
        dukx_peval_file(ctx_, SOURCEDIR "/syntax-error.js");
    } catch (const Exception& ex) {
        BOOST_REQUIRE_EQUAL("SyntaxError", ex.name);
        BOOST_REQUIRE_EQUAL("syntax-error.js", fs_util::base_name(ex.fileName));
        BOOST_REQUIRE_EQUAL(6, ex.lineNumber);
        BOOST_REQUIRE_EQUAL("empty expression not allowed (line 6)", ex.message);
    }
}

BOOST_AUTO_TEST_SUITE_END()

} // !irccd
