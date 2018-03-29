/*
 * main.cpp -- test irccdctl plugin-reload
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

#define BOOST_TEST_MODULE "irccdctl plugin-reload"
#include <boost/test/unit_test.hpp>

#include <irccd/test/cli_test.hpp>

namespace irccd {

BOOST_FIXTURE_TEST_SUITE(plugin_reload_suite, cli_test)

BOOST_AUTO_TEST_CASE(simple)
{
    run_irccd("irccd-plugins.conf");

    {
        // onReload will update the config.
        const auto result = run_irccdctl({ "plugin-reload", "foo" });

        BOOST_TEST(result.first.size() == 0U);
        BOOST_TEST(result.second.size() == 0U);
    }

    {
        const auto result = run_irccdctl({ "plugin-config", "foo", "reloaded" });

        BOOST_TEST(result.first.size() == 2U);
        BOOST_TEST(result.second.size() == 0U);
        BOOST_TEST(result.first[0] == "true");
    }
}

BOOST_AUTO_TEST_SUITE_END()

} // !irccd
