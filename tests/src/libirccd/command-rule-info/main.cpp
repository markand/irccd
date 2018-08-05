/*
 * main.cpp -- test rule-info remote command
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

#define BOOST_TEST_MODULE "rule-info"
#include <boost/test/unit_test.hpp>

#include <irccd/json_util.hpp>

#include <irccd/daemon/rule_service.hpp>

#include <irccd/test/command_test.hpp>

namespace irccd {

namespace {

class rule_info_test : public command_test<rule_info_command> {
public:
    rule_info_test()
    {
        daemon_->rules().add(rule(
            { "s1", "s2" },
            { "c1", "c2" },
            { "o1", "o2" },
            { "p1", "p2" },
            { "onMessage", "onCommand" },
            rule::action::drop
        ));
        daemon_->rules().add(rule(
            { "s1", },
            { "c1", },
            { "o1", },
            { "p1", },
            { "onMessage", },
            rule::action::accept
        ));
    }
};

BOOST_FIXTURE_TEST_SUITE(rule_info_test_suite, rule_info_test)

BOOST_AUTO_TEST_CASE(basic)
{
    const auto [json, code] = request({
        { "command",    "rule-info" },
        { "index",      0           }
    });

    auto servers = json["servers"];
    auto channels = json["channels"];
    auto plugins = json["plugins"];
    auto events = json["events"];

    BOOST_TEST(!code);
    BOOST_TEST(json_util::contains(servers, "s1"));
    BOOST_TEST(json_util::contains(servers, "s2"));
    BOOST_TEST(json_util::contains(channels, "c1"));
    BOOST_TEST(json_util::contains(channels, "c2"));
    BOOST_TEST(json_util::contains(plugins, "p1"));
    BOOST_TEST(json_util::contains(plugins, "p2"));
    BOOST_TEST(json_util::contains(events, "onMessage"));
    BOOST_TEST(json_util::contains(events, "onCommand"));
    BOOST_TEST(json["action"].get<std::string>() == "drop");
}

BOOST_AUTO_TEST_SUITE(errors)

BOOST_AUTO_TEST_CASE(invalid_index_1)
{
    const auto [json, code] = request({
        { "command",    "rule-info" },
        { "index",      -100        }
    });

    BOOST_TEST(code == rule_error::invalid_index);
    BOOST_TEST(json["error"].get<int>() == rule_error::invalid_index);
    BOOST_TEST(json["errorCategory"].get<std::string>() == "rule");
}

BOOST_AUTO_TEST_CASE(invalid_index_2)
{
    const auto [json, code] = request({
        { "command",    "rule-info" },
        { "index",      100         }
    });

    BOOST_TEST(code == rule_error::invalid_index);
    BOOST_TEST(json["error"].get<int>() == rule_error::invalid_index);
    BOOST_TEST(json["errorCategory"].get<std::string>() == "rule");
}

BOOST_AUTO_TEST_CASE(invalid_index_3)
{
    const auto [json, code] = request({
        { "command",    "rule-info" },
        { "index",      "notaint"   }
    });

    BOOST_TEST(code == rule_error::invalid_index);
    BOOST_TEST(json["error"].get<int>() == rule_error::invalid_index);
    BOOST_TEST(json["errorCategory"].get<std::string>() == "rule");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
