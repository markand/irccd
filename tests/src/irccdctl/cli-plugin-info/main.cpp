/*
 * main.cpp -- test irccdctl plugin-info
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

#define BOOST_TEST_MODULE "irccdctl plugin-info"
#include <boost/test/unit_test.hpp>

#include <irccd/test/plugin_cli_test.hpp>
#include <irccd/test/mock_plugin.hpp>

namespace irccd {

namespace {

BOOST_FIXTURE_TEST_SUITE(plugin_info_suite, plugin_cli_test)

BOOST_AUTO_TEST_CASE(simple)
{
    irccd_.plugins().add(std::make_unique<mock_plugin>("test"));
    start();

    const auto [out, err] = exec({ "plugin-info", "test" });

    BOOST_TEST(out.size() == 4U);
    BOOST_TEST(err.size() == 0U);
    BOOST_TEST(out[0] == "Author         : David Demelier <markand@malikania.fr>");
    BOOST_TEST(out[1] == "License        : ISC");
    BOOST_TEST(out[2] == "Summary        : mock plugin");
    BOOST_TEST(out[3] == "Version        : 1.0");
}

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
