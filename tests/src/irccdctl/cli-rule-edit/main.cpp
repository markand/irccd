/*
 * main.cpp -- test irccdctl rule-edit
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

#define BOOST_TEST_MODULE "irccdctl rule-edit"
#include <boost/test/unit_test.hpp>

#include <irccd/test/cli_test.hpp>

namespace irccd {

BOOST_FIXTURE_TEST_SUITE(rule_edit_suite, cli_test)

BOOST_AUTO_TEST_CASE(server)
{
    run_irccd("irccd-rules.conf");

    {
        const auto result = run_irccdctl({ "rule-edit",
            "-s ts1",  "--add-server ts2",
            "-S s1",    "--remove-server s2",
            "0"
        });

        BOOST_TEST(result.first.size() == 0U);
        BOOST_TEST(result.second.size() == 0U);
    }

    {
        const auto result = run_irccdctl({ "rule-list" });

        BOOST_TEST(result.first.size() == 8U);
        BOOST_TEST(result.second.size() == 0U);
        BOOST_TEST(result.first[0]  == "rule:        0");
        BOOST_TEST(result.first[1]  == "servers:     ts1 ts2 ");
        BOOST_TEST(result.first[2]  == "channels:    c1 c2 ");
        BOOST_TEST(result.first[3]  == "plugins:     p1 p2 ");
        BOOST_TEST(result.first[4]  == "events:      onCommand onMessage ");
        BOOST_TEST(result.first[5]  == "action:      drop");
    }
}

BOOST_AUTO_TEST_CASE(channel)
{
    run_irccd("irccd-rules.conf");

    {
        const auto result = run_irccdctl({ "rule-edit",
            "-c tc1",   "--add-channel tc2",
            "-C c1",    "--remove-channel c2",
            "0"
        });

        BOOST_TEST(result.first.size() == 0U);
        BOOST_TEST(result.second.size() == 0U);
    }

    {
        const auto result = run_irccdctl({ "rule-list" });

        BOOST_TEST(result.first.size() == 8U);
        BOOST_TEST(result.second.size() == 0U);
        BOOST_TEST(result.first[0]  == "rule:        0");
        BOOST_TEST(result.first[1]  == "servers:     s1 s2 ");
        BOOST_TEST(result.first[2]  == "channels:    tc1 tc2 ");
        BOOST_TEST(result.first[3]  == "plugins:     p1 p2 ");
        BOOST_TEST(result.first[4]  == "events:      onCommand onMessage ");
        BOOST_TEST(result.first[5]  == "action:      drop");
    }
}

BOOST_AUTO_TEST_CASE(plugin)
{
    run_irccd("irccd-rules.conf");

    {
        const auto result = run_irccdctl({ "rule-edit",
            "-p tp1",   "--add-plugin tp2",
            "-P p1",    "--remove-plugin p2",
            "0"
        });

        BOOST_TEST(result.first.size() == 0U);
        BOOST_TEST(result.second.size() == 0U);
    }

    {
        const auto result = run_irccdctl({ "rule-list" });

        BOOST_TEST(result.first.size() == 8U);
        BOOST_TEST(result.second.size() == 0U);
        BOOST_TEST(result.first[0]  == "rule:        0");
        BOOST_TEST(result.first[1]  == "servers:     s1 s2 ");
        BOOST_TEST(result.first[2]  == "channels:    c1 c2 ");
        BOOST_TEST(result.first[3]  == "plugins:     tp1 tp2 ");
        BOOST_TEST(result.first[4]  == "events:      onCommand onMessage ");
        BOOST_TEST(result.first[5]  == "action:      drop");
    }
}

BOOST_AUTO_TEST_CASE(event)
{
    run_irccd("irccd-rules.conf");

    {
        const auto result = run_irccdctl({ "rule-edit",
            "-e onKick",    "--add-event onNickname",
            "-E onMessage", "--remove-event onCommand",
            "0"
        });

        BOOST_TEST(result.first.size() == 0U);
        BOOST_TEST(result.second.size() == 0U);
    }

    {
        const auto result = run_irccdctl({ "rule-list" });

        BOOST_TEST(result.first.size() == 8U);
        BOOST_TEST(result.second.size() == 0U);
        BOOST_TEST(result.first[0]  == "rule:        0");
        BOOST_TEST(result.first[1]  == "servers:     s1 s2 ");
        BOOST_TEST(result.first[2]  == "channels:    c1 c2 ");
        BOOST_TEST(result.first[3]  == "plugins:     p1 p2 ");
        BOOST_TEST(result.first[4]  == "events:      onKick onNickname ");
        BOOST_TEST(result.first[5]  == "action:      drop");
    }
}

BOOST_AUTO_TEST_CASE(action_1)
{
    run_irccd("irccd-rules.conf");

    {
        const auto result = run_irccdctl({ "rule-edit", "-a accept", "0" });

        BOOST_TEST(result.first.size() == 0U);
        BOOST_TEST(result.second.size() == 0U);
    }

    {
        const auto result = run_irccdctl({ "rule-list" });

        BOOST_TEST(result.first.size() == 8U);
        BOOST_TEST(result.second.size() == 0U);
        BOOST_TEST(result.first[0]  == "rule:        0");
        BOOST_TEST(result.first[1]  == "servers:     s1 s2 ");
        BOOST_TEST(result.first[2]  == "channels:    c1 c2 ");
        BOOST_TEST(result.first[3]  == "plugins:     p1 p2 ");
        BOOST_TEST(result.first[4]  == "events:      onCommand onMessage ");
        BOOST_TEST(result.first[5]  == "action:      accept");
    }
}

BOOST_AUTO_TEST_CASE(action_2)
{
    run_irccd("irccd-rules.conf");

    {
        const auto result = run_irccdctl({ "rule-edit", "--action accept", "0" });

        BOOST_TEST(result.first.size() == 0U);
        BOOST_TEST(result.second.size() == 0U);
    }

    {
        const auto result = run_irccdctl({ "rule-list" });

        BOOST_TEST(result.first.size() == 8U);
        BOOST_TEST(result.second.size() == 0U);
        BOOST_TEST(result.first[0]  == "rule:        0");
        BOOST_TEST(result.first[1]  == "servers:     s1 s2 ");
        BOOST_TEST(result.first[2]  == "channels:    c1 c2 ");
        BOOST_TEST(result.first[3]  == "plugins:     p1 p2 ");
        BOOST_TEST(result.first[4]  == "events:      onCommand onMessage ");
        BOOST_TEST(result.first[5]  == "action:      accept");
    }
}

BOOST_AUTO_TEST_SUITE_END()

} // !irccd
