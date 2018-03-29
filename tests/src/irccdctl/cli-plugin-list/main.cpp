/*
 * main.cpp -- test irccdctl plugin-list
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

#define BOOST_TEST_MODULE "irccdctl plugin-list"
#include <boost/test/unit_test.hpp>

#include <irccd/test/cli_test.hpp>

namespace irccd {

BOOST_FIXTURE_TEST_SUITE(plugin_list_suite, cli_test)

BOOST_AUTO_TEST_CASE(output)
{
    const auto result = run("irccd-plugins.conf", { "plugin-list" });

    BOOST_TEST(result.first.size() == 3U);
    BOOST_TEST(result.second.size() == 0U);
    BOOST_TEST(result.first[0] == "foo");
    BOOST_TEST(result.first[1] == "bar");
}

BOOST_AUTO_TEST_SUITE_END()

} // !irccd
