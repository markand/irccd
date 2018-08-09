/*
 * main.cpp -- test irccdctl rule-add
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

#define BOOST_TEST_MODULE "irccdctl rule-add"
#include <boost/test/unit_test.hpp>

#include <irccd/test/cli_fixture.hpp>

using namespace irccd::test;

namespace irccd {

namespace {

BOOST_FIXTURE_TEST_SUITE(rule_add_suite, cli_fixture)

BOOST_AUTO_TEST_CASE(all)
{
    start();

    {
        const auto [code, out, err] = exec({ "rule-add",
            "-c c1",        "--add-channel c2",
            "-e onMessage", "--add-event onCommand",
            "-p p1",        "--add-plugin p2",
            "-s s1",        "--add-server s2",
            "drop"
        });

        BOOST_TEST(!code);
        BOOST_TEST(out.size() == 0U);
        BOOST_TEST(err.size() == 0U);
    }

    {
        const auto [code, out, err] = exec({ "rule-list" });

        BOOST_TEST(!code);
        BOOST_TEST(out.size() == 7U);
        BOOST_TEST(err.size() == 0U);
        BOOST_TEST(out[0]  == "rule:        0");
        BOOST_TEST(out[1]  == "servers:     s1 s2 ");
        BOOST_TEST(out[2]  == "channels:    c1 c2 ");
        BOOST_TEST(out[3]  == "plugins:     p1 p2 ");
        BOOST_TEST(out[4]  == "events:      onCommand onMessage ");
        BOOST_TEST(out[5]  == "action:      drop");
    }
}

BOOST_AUTO_TEST_CASE(server)
{
    start();

    {
        const auto [code, out, err] = exec({ "rule-add", "-s s1", "--add-server s2", "drop" });

        BOOST_TEST(!code);
        BOOST_TEST(out.size() == 0U);
        BOOST_TEST(err.size() == 0U);
    }

    {
        const auto [code, out, err] = exec({ "rule-list" });

        BOOST_TEST(!code);
        BOOST_TEST(out.size() == 7U);
        BOOST_TEST(err.size() == 0U);
        BOOST_TEST(out[0]  == "rule:        0");
        BOOST_TEST(out[1]  == "servers:     s1 s2 ");
        BOOST_TEST(out[2]  == "channels:    ");
        BOOST_TEST(out[3]  == "plugins:     ");
        BOOST_TEST(out[4]  == "events:      ");
        BOOST_TEST(out[5]  == "action:      drop");
    }
}

BOOST_AUTO_TEST_CASE(channel)
{
    start();

    {
        const auto [code, out, err] = exec({ "rule-add", "-c c1", "--add-channel c2", "drop" });

        BOOST_TEST(!code);
        BOOST_TEST(out.size() == 0U);
        BOOST_TEST(err.size() == 0U);
    }

    {
        const auto [code, out, err] = exec({ "rule-list" });

        BOOST_TEST(!code);
        BOOST_TEST(out.size() == 7U);
        BOOST_TEST(err.size() == 0U);
        BOOST_TEST(out[0]  == "rule:        0");
        BOOST_TEST(out[1]  == "servers:     ");
        BOOST_TEST(out[2]  == "channels:    c1 c2 ");
        BOOST_TEST(out[3]  == "plugins:     ");
        BOOST_TEST(out[4]  == "events:      ");
        BOOST_TEST(out[5]  == "action:      drop");
    }
}

BOOST_AUTO_TEST_CASE(plugin)
{
    start();

    {
        const auto [code, out, err] = exec({ "rule-add", "-p p1", "--add-plugin p2", "drop" });

        BOOST_TEST(!code);
        BOOST_TEST(out.size() == 0U);
        BOOST_TEST(err.size() == 0U);
    }

    {
        const auto [code, out, err] = exec({ "rule-list" });

        BOOST_TEST(!code);
        BOOST_TEST(out.size() == 7U);
        BOOST_TEST(err.size() == 0U);
        BOOST_TEST(out[0]  == "rule:        0");
        BOOST_TEST(out[1]  == "servers:     ");
        BOOST_TEST(out[2]  == "channels:    ");
        BOOST_TEST(out[3]  == "plugins:     p1 p2 ");
        BOOST_TEST(out[4]  == "events:      ");
        BOOST_TEST(out[5]  == "action:      drop");
    }
}

BOOST_AUTO_TEST_CASE(event)
{
    start();

    {
        const auto [code, out, err] = exec({ "rule-add", "-e onMessage", "--add-event onCommand", "drop" });

        BOOST_TEST(!code);
        BOOST_TEST(out.size() == 0U);
        BOOST_TEST(err.size() == 0U);
    }

    {
        const auto [code, out, err] = exec({ "rule-list" });

        BOOST_TEST(!code);
        BOOST_TEST(out.size() == 7U);
        BOOST_TEST(err.size() == 0U);
        BOOST_TEST(out[0]  == "rule:        0");
        BOOST_TEST(out[1]  == "servers:     ");
        BOOST_TEST(out[2]  == "channels:    ");
        BOOST_TEST(out[3]  == "plugins:     ");
        BOOST_TEST(out[4]  == "events:      onCommand onMessage ");
        BOOST_TEST(out[5]  == "action:      drop");
    }
}

BOOST_AUTO_TEST_SUITE(errors)

BOOST_AUTO_TEST_CASE(invalid_action)
{
    start();

    const auto [code, out, err] = exec({ "rule-add", "-p p1", "--add-plugin p2", "break" });

    BOOST_TEST(code);
    BOOST_TEST(out.size() == 0U);
    BOOST_TEST(err.size() == 1U);
    BOOST_TEST(err[0] == "abort: invalid rule action");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
