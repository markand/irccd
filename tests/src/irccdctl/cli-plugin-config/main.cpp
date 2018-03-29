/*
 * main.cpp -- test irccdctl plugin-config
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

#define BOOST_TEST_MODULE "irccdctl plugin-config"
#include <boost/test/unit_test.hpp>

#include <irccd/test/cli_test.hpp>

namespace irccd {

BOOST_FIXTURE_TEST_SUITE(plugin_config_suite, cli_test)

BOOST_AUTO_TEST_CASE(set_and_get)
{
    run_irccd("irccd-plugins.conf");

    // First, configure. No output yet
    {
        const auto result = run_irccdctl({ "plugin-config", "foo", "verbose", "false" });

        // no output yet.
        BOOST_TEST(result.first.size() == 0U);
        BOOST_TEST(result.second.size() == 0U);
    }

    // Get the newly created value.
    {
        const auto result = run_irccdctl({ "plugin-config", "foo", "verbose" });

        BOOST_TEST(result.first.size() == 2U);
        BOOST_TEST(result.second.size() == 0U);
        BOOST_TEST(result.first[0] == "false");
    }
}

BOOST_AUTO_TEST_CASE(getall)
{
    const auto result = run("irccd-plugins.conf", { "plugin-config", "bar" });

    BOOST_TEST(result.first.size() == 3U);
    BOOST_TEST(result.second.size() == 0U);
    BOOST_TEST(result.first[0] == "v1               : 123");
    BOOST_TEST(result.first[1] == "v2               : 456");
}

BOOST_AUTO_TEST_SUITE_END()

} // !irccd
