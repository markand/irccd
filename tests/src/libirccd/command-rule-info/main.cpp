/*
 * main.cpp -- test rule-info remote command
 *
 * Copyright (c) 2013-2017 David Demelier <markand@malikania.fr>
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

#include <irccd/daemon/rule_info_command.hpp>
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
            rule::action_type::drop
        ));
        daemon_->rules().add(rule(
            { "s1", },
            { "c1", },
            { "o1", },
            { "p1", },
            { "onMessage", },
            rule::action_type::accept
        ));
    }
};

} // !namespace

BOOST_FIXTURE_TEST_SUITE(rule_info_test_suite, rule_info_test)

BOOST_AUTO_TEST_CASE(basic)
{
    nlohmann::json result;

    ctl_->send({
        { "command",    "rule-info" },
        { "index",      0           }
    });
    ctl_->recv([&] (auto, auto msg) {
        result = msg;
    });

    wait_for([&] () {
        return result.is_object();
    });

    BOOST_TEST(result.is_object());

    auto servers = result["servers"];
    auto channels = result["channels"];
    auto plugins = result["plugins"];
    auto events = result["events"];

    BOOST_TEST(json_util::contains(servers, "s1"));
    BOOST_TEST(json_util::contains(servers, "s2"));
    BOOST_TEST(json_util::contains(channels, "c1"));
    BOOST_TEST(json_util::contains(channels, "c2"));
    BOOST_TEST(json_util::contains(plugins, "p1"));
    BOOST_TEST(json_util::contains(plugins, "p2"));
    BOOST_TEST(json_util::contains(events, "onMessage"));
    BOOST_TEST(json_util::contains(events, "onCommand"));
    BOOST_TEST(result["action"].get<std::string>() == "drop");
}

BOOST_AUTO_TEST_SUITE(errors)

BOOST_AUTO_TEST_CASE(invalid_index_1)
{
    boost::system::error_code result;
    nlohmann::json message;

    ctl_->send({
        { "command",    "rule-info" },
        { "index",      -100        }
    });
    ctl_->recv([&] (auto rresult, auto rmessage) {
        result = rresult;
        message = rmessage;
    });

    wait_for([&] {
        return result;
    });

    BOOST_TEST(result == rule_error::invalid_index);
    BOOST_TEST(message["error"].template get<int>() == rule_error::invalid_index);
    BOOST_TEST(message["errorCategory"].template get<std::string>() == "rule");
}

BOOST_AUTO_TEST_CASE(invalid_index_2)
{
    boost::system::error_code result;
    nlohmann::json message;

    ctl_->send({
        { "command",    "rule-info" },
        { "index",      100         }
    });
    ctl_->recv([&] (auto rresult, auto rmessage) {
        result = rresult;
        message = rmessage;
    });

    wait_for([&] {
        return result;
    });

    BOOST_TEST(result == rule_error::invalid_index);
    BOOST_TEST(message["error"].template get<int>() == rule_error::invalid_index);
    BOOST_TEST(message["errorCategory"].template get<std::string>() == "rule");
}

BOOST_AUTO_TEST_CASE(invalid_index_3)
{
    boost::system::error_code result;
    nlohmann::json message;

    ctl_->send({
        { "command",    "rule-info" },
        { "index",      "notaint"   }
    });
    ctl_->recv([&] (auto rresult, auto rmessage) {
        result = rresult;
        message = rmessage;
    });

    wait_for([&] {
        return result;
    });

    BOOST_TEST(result == rule_error::invalid_index);
    BOOST_TEST(message["error"].template get<int>() == rule_error::invalid_index);
    BOOST_TEST(message["errorCategory"].template get<std::string>() == "rule");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

} // !irccd
