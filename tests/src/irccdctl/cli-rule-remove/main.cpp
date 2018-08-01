/*
 * main.cpp -- test irccdctl rule-remove
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

#define BOOST_TEST_MODULE "irccdctl rule-remove"
#include <boost/test/unit_test.hpp>

#include <irccd/test/rule_cli_test.hpp>

namespace irccd {

namespace {

class custom_rule_cli_test : public rule_cli_test {
public:
    custom_rule_cli_test()
    {
        irccd_.rules().add({
            { "s1" },
            { "c1" },
            { "o1" },
            { "p1" },
            { "onTopic" },
            rule::action::accept
        });
        irccd_.rules().add({
            { "s2" },
            { "c2" },
            { "o2" },
            { "p2" },
            { "onCommand" },
            rule::action::drop
        });
        irccd_.rules().add({
            { "s3" },
            { "c3" },
            { "o3" },
            { "p3" },
            { "onMessage" },
            rule::action::accept
        });
    }
};

BOOST_FIXTURE_TEST_SUITE(rule_move_suite, custom_rule_cli_test)

BOOST_AUTO_TEST_CASE(simple)
{
    start();

    {
        const auto [code, out, err] = exec({ "rule-remove", "0" });

        BOOST_TEST(!code);
        BOOST_TEST(out.size() == 0U);
        BOOST_TEST(err.size() == 0U);
    }

    {
        const auto [code, out, err] = exec({ "rule-list" });

        BOOST_TEST(!code);
        BOOST_TEST(out.size() == 14U);
        BOOST_TEST(err.size() == 0U);
        BOOST_TEST(out[0]  == "rule:        0");
        BOOST_TEST(out[1]  == "servers:     s2 ");
        BOOST_TEST(out[2]  == "channels:    c2 ");
        BOOST_TEST(out[3]  == "plugins:     p2 ");
        BOOST_TEST(out[4]  == "events:      onCommand ");
        BOOST_TEST(out[5]  == "action:      drop");
        BOOST_TEST(out[6]  == "");
        BOOST_TEST(out[7]  == "rule:        1");
        BOOST_TEST(out[8]  == "servers:     s3 ");
        BOOST_TEST(out[9]  == "channels:    c3 ");
        BOOST_TEST(out[10] == "plugins:     p3 ");
        BOOST_TEST(out[11] == "events:      onMessage ");
        BOOST_TEST(out[12] == "action:      accept");
        BOOST_TEST(out[13] == "");
    }
}

BOOST_AUTO_TEST_SUITE(errors)

BOOST_AUTO_TEST_CASE(invalid_index_1)
{
    start();

    const auto [code, out, err] = exec({ "rule-remove", "100" });

    BOOST_TEST(code);
    BOOST_TEST(out.size() == 0U);
    BOOST_TEST(err.size() == 1U);
    BOOST_TEST(err[0] == "abort: invalid rule index");
}

BOOST_AUTO_TEST_CASE(invalid_index_2)
{
    start();

    const auto [code, out, err] = exec({ "rule-remove", "notaint" });

    BOOST_TEST(code);
    BOOST_TEST(out.size() == 0U);
    BOOST_TEST(err.size() == 1U);
    BOOST_TEST(err[0] == "abort: invalid rule index");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
