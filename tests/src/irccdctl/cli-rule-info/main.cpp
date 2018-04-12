/*
 * main.cpp -- test irccdctl rule-info
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

#define BOOST_TEST_MODULE "irccdctl rule-info"
#include <boost/test/unit_test.hpp>

#include <irccd/test/rule_cli_test.hpp>

namespace irccd {

BOOST_FIXTURE_TEST_SUITE(rule_info_suite, rule_cli_test)

BOOST_AUTO_TEST_CASE(info)
{
    irccd_.rules().add({
        { "s1", "s2" },
        { "c1", "c2" },
        { "o1", "o2" },
        { "p1", "p2" },
        { "onCommand", "onMessage" },
        rule::action::drop
    });
    start();

    const auto result = exec({ "rule-info", "0" });

    BOOST_TEST(result.first.size() == 7U);
    BOOST_TEST(result.second.size() == 0U);
    BOOST_TEST(result.first[0]  == "rule:        0");
    BOOST_TEST(result.first[1]  == "servers:     s1 s2 ");
    BOOST_TEST(result.first[2]  == "channels:    c1 c2 ");
    BOOST_TEST(result.first[3]  == "plugins:     p1 p2 ");
    BOOST_TEST(result.first[4]  == "events:      onCommand onMessage ");
    BOOST_TEST(result.first[5]  == "action:      drop");
}

BOOST_AUTO_TEST_SUITE_END()

} // !irccd
