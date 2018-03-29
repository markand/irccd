/*
 * main.cpp -- test irccdctl rule-move
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

#define BOOST_TEST_MODULE "irccdctl rule-move"
#include <boost/test/unit_test.hpp>

#include <irccd/test/cli_test.hpp>

namespace irccd {

BOOST_FIXTURE_TEST_SUITE(rule_move_suite, cli_test)

BOOST_AUTO_TEST_CASE(from_0_to_1)
{
    run_irccd("irccd-multiple-rules.conf");

    {
        const auto result = run_irccdctl({ "rule-move", "0", "1" });

        BOOST_TEST(result.first.size() == 0U);
        BOOST_TEST(result.second.size() == 0U);
    }

    {
        const auto result = run_irccdctl({ "rule-list" });

        BOOST_TEST(result.first.size() == 22U);
        BOOST_TEST(result.second.size() == 0U);
        BOOST_TEST(result.first[0]  == "rule:        0");
        BOOST_TEST(result.first[1]  == "servers:     s2 ");
        BOOST_TEST(result.first[2]  == "channels:    c2 ");
        BOOST_TEST(result.first[3]  == "plugins:     p2 ");
        BOOST_TEST(result.first[4]  == "events:      onCommand ");
        BOOST_TEST(result.first[5]  == "action:      drop");
        BOOST_TEST(result.first[6]  == "");
        BOOST_TEST(result.first[7]  == "rule:        1");
        BOOST_TEST(result.first[8]  == "servers:     s1 ");
        BOOST_TEST(result.first[9]  == "channels:    c1 ");
        BOOST_TEST(result.first[10] == "plugins:     p1 ");
        BOOST_TEST(result.first[11] == "events:      onTopic ");
        BOOST_TEST(result.first[12] == "action:      accept");
        BOOST_TEST(result.first[13] == "");
        BOOST_TEST(result.first[14] == "rule:        2");
        BOOST_TEST(result.first[15] == "servers:     s3 ");
        BOOST_TEST(result.first[16] == "channels:    c3 ");
        BOOST_TEST(result.first[17] == "plugins:     p3 ");
        BOOST_TEST(result.first[18] == "events:      onMessage ");
        BOOST_TEST(result.first[19] == "action:      accept");
        BOOST_TEST(result.first[20] == "");
    }
}

BOOST_AUTO_TEST_CASE(from_2_to_0)
{
    run_irccd("irccd-multiple-rules.conf");

    {
        const auto result = run_irccdctl({ "rule-move", "2", "0" });

        BOOST_TEST(result.first.size() == 0U);
        BOOST_TEST(result.second.size() == 0U);
    }

    {
        const auto result = run_irccdctl({ "rule-list" });

        BOOST_TEST(result.first.size() == 22U);
        BOOST_TEST(result.second.size() == 0U);
        BOOST_TEST(result.first[0]  == "rule:        0");
        BOOST_TEST(result.first[1]  == "servers:     s3 ");
        BOOST_TEST(result.first[2]  == "channels:    c3 ");
        BOOST_TEST(result.first[3]  == "plugins:     p3 ");
        BOOST_TEST(result.first[4]  == "events:      onMessage ");
        BOOST_TEST(result.first[5]  == "action:      accept");
        BOOST_TEST(result.first[6]  == "");
        BOOST_TEST(result.first[7]  == "rule:        1");
        BOOST_TEST(result.first[8]  == "servers:     s1 ");
        BOOST_TEST(result.first[9]  == "channels:    c1 ");
        BOOST_TEST(result.first[10] == "plugins:     p1 ");
        BOOST_TEST(result.first[11] == "events:      onTopic ");
        BOOST_TEST(result.first[12] == "action:      accept");
        BOOST_TEST(result.first[13] == "");
        BOOST_TEST(result.first[14] == "rule:        2");
        BOOST_TEST(result.first[15] == "servers:     s2 ");
        BOOST_TEST(result.first[16] == "channels:    c2 ");
        BOOST_TEST(result.first[17] == "plugins:     p2 ");
        BOOST_TEST(result.first[18] == "events:      onCommand ");
        BOOST_TEST(result.first[19] == "action:      drop");
        BOOST_TEST(result.first[20] == "");
    }
}

BOOST_AUTO_TEST_CASE(same)
{
    run_irccd("irccd-multiple-rules.conf");

    {
        const auto result = run_irccdctl({ "rule-move", "2", "2" });

        BOOST_TEST(result.first.size() == 0U);
        BOOST_TEST(result.second.size() == 0U);
    }

    {
        const auto result = run_irccdctl({ "rule-list" });

        BOOST_TEST(result.first.size() == 22U);
        BOOST_TEST(result.second.size() == 0U);
        BOOST_TEST(result.first[0]  == "rule:        0");
        BOOST_TEST(result.first[1]  == "servers:     s1 ");
        BOOST_TEST(result.first[2]  == "channels:    c1 ");
        BOOST_TEST(result.first[3]  == "plugins:     p1 ");
        BOOST_TEST(result.first[4]  == "events:      onTopic ");
        BOOST_TEST(result.first[5]  == "action:      accept");
        BOOST_TEST(result.first[6]  == "");
        BOOST_TEST(result.first[7]  == "rule:        1");
        BOOST_TEST(result.first[8]  == "servers:     s2 ");
        BOOST_TEST(result.first[9]  == "channels:    c2 ");
        BOOST_TEST(result.first[10] == "plugins:     p2 ");
        BOOST_TEST(result.first[11] == "events:      onCommand ");
        BOOST_TEST(result.first[12] == "action:      drop");
        BOOST_TEST(result.first[13] == "");
        BOOST_TEST(result.first[14] == "rule:        2");
        BOOST_TEST(result.first[15] == "servers:     s3 ");
        BOOST_TEST(result.first[16] == "channels:    c3 ");
        BOOST_TEST(result.first[17] == "plugins:     p3 ");
        BOOST_TEST(result.first[18] == "events:      onMessage ");
        BOOST_TEST(result.first[19] == "action:      accept");
        BOOST_TEST(result.first[20] == "");
    }
}

BOOST_AUTO_TEST_SUITE_END()

} // !irccd
