/*
 * main.cpp -- test rule-move remote command
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

#define BOOST_TEST_MODULE "rule-move"
#include <boost/test/unit_test.hpp>

#include <irccd/json_util.hpp>

#include <irccd/daemon/rule_list_command.hpp>
#include <irccd/daemon/rule_move_command.hpp>
#include <irccd/daemon/service/rule_service.hpp>

#include <irccd/test/command_test.hpp>

namespace irccd {

namespace {

class rule_move_test : public command_test<rule_move_command, rule_list_command> {
public:
    rule_move_test()
    {
        daemon_->rules().add(rule(
            { "s0" },
            { "c0" },
            { "o0" },
            { "p0" },
            { "onMessage" },
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
        daemon_->rules().add(rule(
            { "s2", },
            { "c2", },
            { "o2", },
            { "p2", },
            { "onMessage", },
            rule::action::accept
        ));
    }
};

} // !namespace

BOOST_FIXTURE_TEST_SUITE(rule_move_test_suite, rule_move_test)

BOOST_AUTO_TEST_CASE(backward)
{
    nlohmann::json result;

    ctl_->send({
        { "command",    "rule-move" },
        { "from",       2           },
        { "to",         0           }
    });
    ctl_->recv([&] (auto, auto msg) {
        result = msg;
    });

    wait_for([&] () {
        return result.is_object();
    });

    BOOST_TEST(result.is_object());

    result = nullptr;
    ctl_->send({{ "command", "rule-list" }});
    ctl_->recv([&] (auto, auto msg) {
        result = msg;
    });

    wait_for([&] () {
        return result.is_object();
    });

    BOOST_TEST(result.is_object());

    // Rule 2.
    {
        auto servers = result["list"][0]["servers"];
        auto channels = result["list"][0]["channels"];
        auto plugins = result["list"][0]["plugins"];
        auto events = result["list"][0]["events"];

        BOOST_TEST(json_util::contains(servers, "s2"));
        BOOST_TEST(json_util::contains(channels, "c2"));
        BOOST_TEST(json_util::contains(plugins, "p2"));
        BOOST_TEST(json_util::contains(events, "onMessage"));
        BOOST_TEST(result["list"][0]["action"].get<std::string>() == "accept");
    }

    // Rule 0.
    {
        auto servers = result["list"][1]["servers"];
        auto channels = result["list"][1]["channels"];
        auto plugins = result["list"][1]["plugins"];
        auto events = result["list"][1]["events"];

        BOOST_TEST(json_util::contains(servers, "s0"));
        BOOST_TEST(json_util::contains(channels, "c0"));
        BOOST_TEST(json_util::contains(plugins, "p0"));
        BOOST_TEST(json_util::contains(events, "onMessage"));
        BOOST_TEST(result["list"][1]["action"].get<std::string>() == "drop");
    }

    // Rule 1.
    {
        auto servers = result["list"][2]["servers"];
        auto channels = result["list"][2]["channels"];
        auto plugins = result["list"][2]["plugins"];
        auto events = result["list"][2]["events"];

        BOOST_TEST(json_util::contains(servers, "s1"));
        BOOST_TEST(json_util::contains(channels, "c1"));
        BOOST_TEST(json_util::contains(plugins, "p1"));
        BOOST_TEST(json_util::contains(events, "onMessage"));
        BOOST_TEST(result["list"][2]["action"].get<std::string>() == "accept");
    }
}

BOOST_AUTO_TEST_CASE(upward)
{
    nlohmann::json result;

    ctl_->send({
        { "command",    "rule-move" },
        { "from",       0           },
        { "to",         2           }
    });
    ctl_->recv([&] (auto, auto msg) {
        result = msg;
    });

    wait_for([&] () {
        return result.is_object();
    });

    BOOST_TEST(result.is_object());

    result = nullptr;
    ctl_->send({{ "command", "rule-list" }});
    ctl_->recv([&] (auto, auto msg) {
        result = msg;
    });

    wait_for([&] () {
        return result.is_object();
    });

    BOOST_TEST(result.is_object());

    // Rule 1.
    {
        auto servers = result["list"][0]["servers"];
        auto channels = result["list"][0]["channels"];
        auto plugins = result["list"][0]["plugins"];
        auto events = result["list"][0]["events"];

        BOOST_TEST(json_util::contains(servers, "s1"));
        BOOST_TEST(json_util::contains(channels, "c1"));
        BOOST_TEST(json_util::contains(plugins, "p1"));
        BOOST_TEST(json_util::contains(events, "onMessage"));
        BOOST_TEST(result["list"][0]["action"].get<std::string>() == "accept");
    }

    // Rule 2.
    {
        auto servers = result["list"][1]["servers"];
        auto channels = result["list"][1]["channels"];
        auto plugins = result["list"][1]["plugins"];
        auto events = result["list"][1]["events"];

        BOOST_TEST(json_util::contains(servers, "s2"));
        BOOST_TEST(json_util::contains(channels, "c2"));
        BOOST_TEST(json_util::contains(plugins, "p2"));
        BOOST_TEST(json_util::contains(events, "onMessage"));
        BOOST_TEST(result["list"][1]["action"].get<std::string>() == "accept");
    }

    // Rule 0.
    {
        auto servers = result["list"][2]["servers"];
        auto channels = result["list"][2]["channels"];
        auto plugins = result["list"][2]["plugins"];
        auto events = result["list"][2]["events"];

        BOOST_TEST(json_util::contains(servers, "s0"));
        BOOST_TEST(json_util::contains(channels, "c0"));
        BOOST_TEST(json_util::contains(plugins, "p0"));
        BOOST_TEST(json_util::contains(events, "onMessage"));
        BOOST_TEST(result["list"][2]["action"].get<std::string>() == "drop");
    }
}

BOOST_AUTO_TEST_CASE(same)
{
    nlohmann::json result;

    ctl_->send({
        { "command",    "rule-move" },
        { "from",       1           },
        { "to",         1           }
    });
    ctl_->recv([&] (auto, auto msg) {
        result = msg;
    });

    wait_for([&] () {
        return result.is_object();
    });

    BOOST_TEST(result.is_object());

    result = nullptr;
    ctl_->send({{ "command", "rule-list" }});
    ctl_->recv([&] (auto, auto msg) {
        result = msg;
    });

    wait_for([&] () {
        return result.is_object();
    });

    BOOST_TEST(result.is_object());

    // Rule 0.
    {
        auto servers = result["list"][0]["servers"];
        auto channels = result["list"][0]["channels"];
        auto plugins = result["list"][0]["plugins"];
        auto events = result["list"][0]["events"];

        BOOST_TEST(json_util::contains(servers, "s0"));
        BOOST_TEST(json_util::contains(channels, "c0"));
        BOOST_TEST(json_util::contains(plugins, "p0"));
        BOOST_TEST(json_util::contains(events, "onMessage"));
        BOOST_TEST(result["list"][0]["action"].get<std::string>() == "drop");
    }

    // Rule 1.
    {
        auto servers = result["list"][1]["servers"];
        auto channels = result["list"][1]["channels"];
        auto plugins = result["list"][1]["plugins"];
        auto events = result["list"][1]["events"];

        BOOST_TEST(json_util::contains(servers, "s1"));
        BOOST_TEST(json_util::contains(channels, "c1"));
        BOOST_TEST(json_util::contains(plugins, "p1"));
        BOOST_TEST(json_util::contains(events, "onMessage"));
        BOOST_TEST(result["list"][1]["action"].get<std::string>() == "accept");
    }

    // Rule 2.
    {
        auto servers = result["list"][2]["servers"];
        auto channels = result["list"][2]["channels"];
        auto plugins = result["list"][2]["plugins"];
        auto events = result["list"][2]["events"];

        BOOST_TEST(json_util::contains(servers, "s2"));
        BOOST_TEST(json_util::contains(channels, "c2"));
        BOOST_TEST(json_util::contains(plugins, "p2"));
        BOOST_TEST(json_util::contains(events, "onMessage"));
        BOOST_TEST(result["list"][2]["action"].get<std::string>() == "accept");
    }
}

BOOST_AUTO_TEST_CASE(beyond)
{
    nlohmann::json result;

    ctl_->send({
        { "command",    "rule-move" },
        { "from",       0           },
        { "to",         123         }
    });
    ctl_->recv([&] (auto, auto msg) {
        result = msg;
    });

    wait_for([&] () {
        return result.is_object();
    });

    BOOST_TEST(result.is_object());

    result = nullptr;
    ctl_->send({{ "command", "rule-list" }});
    ctl_->recv([&] (auto, auto msg) {
        result = msg;
    });

    wait_for([&] () {
        return result.is_object();
    });

    BOOST_TEST(result.is_object());

    // Rule 1.
    {
        auto servers = result["list"][0]["servers"];
        auto channels = result["list"][0]["channels"];
        auto plugins = result["list"][0]["plugins"];
        auto events = result["list"][0]["events"];

        BOOST_TEST(json_util::contains(servers, "s1"));
        BOOST_TEST(json_util::contains(channels, "c1"));
        BOOST_TEST(json_util::contains(plugins, "p1"));
        BOOST_TEST(json_util::contains(events, "onMessage"));
        BOOST_TEST(result["list"][0]["action"].get<std::string>() == "accept");
    }

    // Rule 2.
    {
        auto servers = result["list"][1]["servers"];
        auto channels = result["list"][1]["channels"];
        auto plugins = result["list"][1]["plugins"];
        auto events = result["list"][1]["events"];

        BOOST_TEST(json_util::contains(servers, "s2"));
        BOOST_TEST(json_util::contains(channels, "c2"));
        BOOST_TEST(json_util::contains(plugins, "p2"));
        BOOST_TEST(json_util::contains(events, "onMessage"));
        BOOST_TEST(result["list"][1]["action"].get<std::string>() == "accept");
    }

    // Rule 0.
    {
        auto servers = result["list"][2]["servers"];
        auto channels = result["list"][2]["channels"];
        auto plugins = result["list"][2]["plugins"];
        auto events = result["list"][2]["events"];

        BOOST_TEST(json_util::contains(servers, "s0"));
        BOOST_TEST(json_util::contains(channels, "c0"));
        BOOST_TEST(json_util::contains(plugins, "p0"));
        BOOST_TEST(json_util::contains(events, "onMessage"));
        BOOST_TEST(result["list"][2]["action"].get<std::string>() == "drop");
    }
}

BOOST_AUTO_TEST_SUITE(errors)

BOOST_AUTO_TEST_CASE(invalid_index_1_from)
{
    boost::system::error_code result;
    nlohmann::json message;

    ctl_->send({
        { "command",    "rule-move" },
        { "from",       -100        },
        { "to",         0           }
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

BOOST_AUTO_TEST_CASE(invalid_index_1_to)
{
    boost::system::error_code result;
    nlohmann::json message;

    ctl_->send({
        { "command",    "rule-move" },
        { "from",       0           },
        { "to",         -100        }
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

BOOST_AUTO_TEST_CASE(invalid_index_2_from)
{
    boost::system::error_code result;
    nlohmann::json message;

    ctl_->send({
        { "command",    "rule-move" },
        { "from",       100         },
        { "to",         0           }
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

BOOST_AUTO_TEST_CASE(invalid_index_3_from)
{
    boost::system::error_code result;
    nlohmann::json message;

    ctl_->send({
        { "command",    "rule-move" },
        { "from",       "notaint"   },
        { "to",         0           }
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

BOOST_AUTO_TEST_CASE(invalid_index_3_to)
{
    boost::system::error_code result;
    nlohmann::json message;

    ctl_->send({
        { "command",    "rule-move" },
        { "from",       0           },
        { "to",         "notaint"   }
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
