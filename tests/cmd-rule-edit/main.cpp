/*
 * main.cpp -- test rule-edit remote command
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

#define BOOST_TEST_MODULE "rule-edit"
#include <boost/test/unit_test.hpp>

#include <irccd/json_util.hpp>

#include <irccd/command.hpp>
#include <irccd/rule_service.hpp>

#include <command_test.hpp>

namespace irccd {

namespace {

class rule_edit_test : public command_test<rule_edit_command> {
public:
    rule_edit_test()
    {
        daemon_->commands().add(std::make_unique<rule_info_command>());
        daemon_->rules().add(rule(
            { "s1", "s2" },
            { "c1", "c2" },
            { "o1", "o2" },
            { "p1", "p2" },
            { "onMessage", "onCommand" },
            rule::action_type::drop
        ));
    }
};

} // !namespace

BOOST_FIXTURE_TEST_SUITE(rule_edit_test_suite, rule_edit_test)

BOOST_AUTO_TEST_CASE(add_server)
{
    nlohmann::json result;

    ctl_->send({
        { "command",        "rule-edit"     },
        { "add-servers",    { "new-s3" }    },
        { "index",          0               }
    });
    ctl_->recv([&] (auto, auto msg) {
        result = msg;
    });

    wait_for([&] () {
        return result.is_object();
    });

    BOOST_TEST(result.is_object());

    result = nullptr;
    ctl_->send({
        { "command",        "rule-info"     },
        { "index",          0               }
    });
    ctl_->recv([&] (auto, auto msg) {
        result = msg;
    });

    wait_for([&] () {
        return result.is_object();
    });

    BOOST_TEST(result.is_object());
    BOOST_TEST(json_util::contains(result["servers"], "s1"));
    BOOST_TEST(json_util::contains(result["servers"], "s2"));
    BOOST_TEST(json_util::contains(result["servers"], "new-s3"));
    BOOST_TEST(json_util::contains(result["channels"], "c1"));
    BOOST_TEST(json_util::contains(result["channels"], "c2"));
    BOOST_TEST(json_util::contains(result["plugins"], "p1"));
    BOOST_TEST(json_util::contains(result["plugins"], "p2"));
    BOOST_TEST(json_util::contains(result["events"], "onMessage"));
    BOOST_TEST(json_util::contains(result["events"], "onCommand"));
    BOOST_TEST(result["action"].get<std::string>() == "drop");
}

BOOST_AUTO_TEST_CASE(add_channel)
{
    nlohmann::json result;

    ctl_->send({
        { "command",        "rule-edit"     },
        { "add-channels",   { "new-c3" }    },
        { "index",          0               }
    });
    ctl_->recv([&] (auto, auto msg) {
        result = msg;
    });

    wait_for([&] () {
        return result.is_object();
    });

    BOOST_TEST(result.is_object());

    result = nullptr;
    ctl_->send({
        { "command",        "rule-info"     },
        { "index",          0               }
    });
    ctl_->recv([&] (auto, auto msg) {
        result = msg;
    });

    wait_for([&] () {
        return result.is_object();
    });

    BOOST_TEST(result.is_object());
    BOOST_TEST(json_util::contains(result["servers"], "s1"));
    BOOST_TEST(json_util::contains(result["servers"], "s2"));
    BOOST_TEST(json_util::contains(result["channels"], "c1"));
    BOOST_TEST(json_util::contains(result["channels"], "c2"));
    BOOST_TEST(json_util::contains(result["channels"], "new-c3"));
    BOOST_TEST(json_util::contains(result["plugins"], "p1"));
    BOOST_TEST(json_util::contains(result["plugins"], "p2"));
    BOOST_TEST(json_util::contains(result["events"], "onMessage"));
    BOOST_TEST(json_util::contains(result["events"], "onCommand"));
    BOOST_TEST(result["action"].get<std::string>() == "drop");
}

BOOST_AUTO_TEST_CASE(add_plugin)
{
    nlohmann::json result;

    ctl_->send({
        { "command",        "rule-edit"     },
        { "add-plugins",    { "new-p3" }    },
        { "index",          0               }
    });
    ctl_->recv([&] (auto, auto msg) {
        result = msg;
    });

    wait_for([&] () {
        return result.is_object();
    });

    BOOST_TEST(result.is_object());

    result = nullptr;
    ctl_->send({
        { "command",        "rule-info"     },
        { "index",          0               }
    });
    ctl_->recv([&] (auto, auto msg) {
        result = msg;
    });

    wait_for([&] () {
        return result.is_object();
    });

    BOOST_TEST(result.is_object());
    BOOST_TEST(json_util::contains(result["servers"], "s1"));
    BOOST_TEST(json_util::contains(result["servers"], "s2"));
    BOOST_TEST(json_util::contains(result["channels"], "c1"));
    BOOST_TEST(json_util::contains(result["channels"], "c2"));
    BOOST_TEST(json_util::contains(result["plugins"], "p1"));
    BOOST_TEST(json_util::contains(result["plugins"], "p2"));
    BOOST_TEST(json_util::contains(result["plugins"], "new-p3"));
    BOOST_TEST(json_util::contains(result["events"], "onMessage"));
    BOOST_TEST(json_util::contains(result["events"], "onCommand"));
    BOOST_TEST(result["action"].get<std::string>() == "drop");
}

BOOST_AUTO_TEST_CASE(add_event)
{
    nlohmann::json result;

    ctl_->send({
        { "command",        "rule-edit"     },
        { "add-events",     { "onQuery" }   },
        { "index",          0               }
    });
    ctl_->recv([&] (auto, auto msg) {
        result = msg;
    });

    wait_for([&] () {
        return result.is_object();
    });

    BOOST_TEST(result.is_object());

    result = nullptr;
    ctl_->send({
        { "command",        "rule-info"     },
        { "index",          0               }
    });
    ctl_->recv([&] (auto, auto msg) {
        result = msg;
    });

    wait_for([&] () {
        return result.is_object();
    });

    BOOST_TEST(result.is_object());
    BOOST_TEST(json_util::contains(result["servers"], "s1"));
    BOOST_TEST(json_util::contains(result["servers"], "s2"));
    BOOST_TEST(json_util::contains(result["channels"], "c1"));
    BOOST_TEST(json_util::contains(result["channels"], "c2"));
    BOOST_TEST(json_util::contains(result["plugins"], "p1"));
    BOOST_TEST(json_util::contains(result["plugins"], "p2"));
    BOOST_TEST(json_util::contains(result["events"], "onMessage"));
    BOOST_TEST(json_util::contains(result["events"], "onCommand"));
    BOOST_TEST(json_util::contains(result["events"], "onQuery"));
    BOOST_TEST(result["action"].get<std::string>() == "drop");
}

BOOST_AUTO_TEST_CASE(add_event_and_server)
{
    nlohmann::json result;

    ctl_->send({
        { "command",        "rule-edit"     },
        { "add-servers",    { "new-s3" }    },
        { "add-events",     { "onQuery" }   },
        { "index",          0               }
    });
    ctl_->recv([&] (auto, auto msg) {
        result = msg;
    });

    wait_for([&] () {
        return result.is_object();
    });

    BOOST_TEST(result.is_object());

    result = nullptr;
    ctl_->send({
        { "command",        "rule-info"     },
        { "index",          0               }
    });
    ctl_->recv([&] (auto, auto msg) {
        result = msg;
    });

    wait_for([&] () {
        return result.is_object();
    });

    BOOST_TEST(result.is_object());
    BOOST_TEST(json_util::contains(result["servers"], "s1"));
    BOOST_TEST(json_util::contains(result["servers"], "s2"));
    BOOST_TEST(json_util::contains(result["servers"], "new-s3"));
    BOOST_TEST(json_util::contains(result["channels"], "c1"));
    BOOST_TEST(json_util::contains(result["channels"], "c2"));
    BOOST_TEST(json_util::contains(result["plugins"], "p1"));
    BOOST_TEST(json_util::contains(result["plugins"], "p2"));
    BOOST_TEST(json_util::contains(result["events"], "onMessage"));
    BOOST_TEST(json_util::contains(result["events"], "onCommand"));
    BOOST_TEST(json_util::contains(result["events"], "onQuery"));
    BOOST_TEST(result["action"].get<std::string>() == "drop");
}

BOOST_AUTO_TEST_CASE(change_action)
{
    nlohmann::json result;

    ctl_->send({
        { "command",        "rule-edit"     },
        { "action",         "accept"        },
        { "index",          0               }
    });
    ctl_->recv([&] (auto, auto msg) {
        result = msg;
    });

    wait_for([&] () {
        return result.is_object();
    });

    BOOST_TEST(result.is_object());

    result = nullptr;
    ctl_->send({
        { "command",        "rule-info"     },
        { "index",          0               }
    });
    ctl_->recv([&] (auto, auto msg) {
        result = msg;
    });

    wait_for([&] () {
        return result.is_object();
    });

    BOOST_TEST(result.is_object());
    BOOST_TEST(json_util::contains(result["servers"], "s1"));
    BOOST_TEST(json_util::contains(result["servers"], "s2"));
    BOOST_TEST(json_util::contains(result["channels"], "c1"));
    BOOST_TEST(json_util::contains(result["channels"], "c2"));
    BOOST_TEST(json_util::contains(result["plugins"], "p1"));
    BOOST_TEST(json_util::contains(result["plugins"], "p2"));
    BOOST_TEST(json_util::contains(result["events"], "onMessage"));
    BOOST_TEST(json_util::contains(result["events"], "onCommand"));
    BOOST_TEST(result["action"].get<std::string>() == "accept");
}

BOOST_AUTO_TEST_CASE(remove_server)
{
    nlohmann::json result;

    ctl_->send({
        { "command",        "rule-edit"     },
        { "remove-servers", { "s2" }        },
        { "index",          0               }
    });
    ctl_->recv([&] (auto, auto msg) {
        result = msg;
    });

    wait_for([&] () {
        return result.is_object();
    });

    BOOST_TEST(result.is_object());

    result = nullptr;
    ctl_->send({
        { "command",        "rule-info"     },
        { "index",          0               }
    });
    ctl_->recv([&] (auto, auto msg) {
        result = msg;
    });

    wait_for([&] () {
        return result.is_object();
    });

    BOOST_TEST(result.is_object());
    BOOST_TEST(json_util::contains(result["servers"], "s1"));
    BOOST_TEST(!json_util::contains(result["servers"], "s2"));
    BOOST_TEST(json_util::contains(result["channels"], "c1"));
    BOOST_TEST(json_util::contains(result["channels"], "c2"));
    BOOST_TEST(json_util::contains(result["plugins"], "p1"));
    BOOST_TEST(json_util::contains(result["plugins"], "p2"));
    BOOST_TEST(json_util::contains(result["events"], "onMessage"));
    BOOST_TEST(json_util::contains(result["events"], "onCommand"));
    BOOST_TEST(result["action"].get<std::string>() == "drop");
}

BOOST_AUTO_TEST_CASE(remove_channel)
{
    nlohmann::json result;

    ctl_->send({
        { "command",        "rule-edit"     },
        { "remove-channels", { "c2" }       },
        { "index",          0               }
    });
    ctl_->recv([&] (auto, auto msg) {
        result = msg;
    });

    wait_for([&] () {
        return result.is_object();
    });

    BOOST_TEST(result.is_object());

    result = nullptr;
    ctl_->send({
        { "command",        "rule-info"     },
        { "index",          0               }
    });
    ctl_->recv([&] (auto, auto msg) {
        result = msg;
    });

    wait_for([&] () {
        return result.is_object();
    });

    BOOST_TEST(result.is_object());
    BOOST_TEST(json_util::contains(result["servers"], "s1"));
    BOOST_TEST(json_util::contains(result["servers"], "s2"));
    BOOST_TEST(json_util::contains(result["channels"], "c1"));
    BOOST_TEST(!json_util::contains(result["channels"], "c2"));
    BOOST_TEST(json_util::contains(result["plugins"], "p1"));
    BOOST_TEST(json_util::contains(result["plugins"], "p2"));
    BOOST_TEST(json_util::contains(result["events"], "onMessage"));
    BOOST_TEST(json_util::contains(result["events"], "onCommand"));
    BOOST_TEST(result["action"].get<std::string>() == "drop");
}

BOOST_AUTO_TEST_CASE(remove_plugin)
{
    nlohmann::json result;

    ctl_->send({
        { "command",        "rule-edit"     },
        { "remove-plugins", { "p2" }        },
        { "index",          0               }
    });
    ctl_->recv([&] (auto, auto msg) {
        result = msg;
    });

    wait_for([&] () {
        return result.is_object();
    });

    BOOST_TEST(result.is_object());

    result = nullptr;
    ctl_->send({
        { "command",        "rule-info"     },
        { "index",          0               }
    });
    ctl_->recv([&] (auto, auto msg) {
        result = msg;
    });

    wait_for([&] () {
        return result.is_object();
    });

    BOOST_TEST(result.is_object());
    BOOST_TEST(json_util::contains(result["servers"], "s1"));
    BOOST_TEST(json_util::contains(result["servers"], "s2"));
    BOOST_TEST(json_util::contains(result["channels"], "c1"));
    BOOST_TEST(json_util::contains(result["channels"], "c2"));
    BOOST_TEST(json_util::contains(result["plugins"], "p1"));
    BOOST_TEST(!json_util::contains(result["plugins"], "p2"));
    BOOST_TEST(json_util::contains(result["events"], "onMessage"));
    BOOST_TEST(json_util::contains(result["events"], "onCommand"));
    BOOST_TEST(result["action"].get<std::string>() == "drop");
}

BOOST_AUTO_TEST_CASE(remove_event)
{
    nlohmann::json result;

    ctl_->send({
        { "command",        "rule-edit"     },
        { "remove-events",  { "onCommand" } },
        { "index",          0               }
    });
    ctl_->recv([&] (auto, auto msg) {
        result = msg;
    });

    wait_for([&] () {
        return result.is_object();
    });

    BOOST_TEST(result.is_object());

    result = nullptr;
    ctl_->send({
        { "command",        "rule-info"     },
        { "index",          0               }
    });
    ctl_->recv([&] (auto, auto msg) {
        result = msg;
    });

    wait_for([&] () {
        return result.is_object();
    });

    BOOST_TEST(result.is_object());
    BOOST_TEST(json_util::contains(result["servers"], "s1"));
    BOOST_TEST(json_util::contains(result["servers"], "s2"));
    BOOST_TEST(json_util::contains(result["channels"], "c1"));
    BOOST_TEST(json_util::contains(result["channels"], "c2"));
    BOOST_TEST(json_util::contains(result["plugins"], "p1"));
    BOOST_TEST(json_util::contains(result["plugins"], "p2"));
    BOOST_TEST(json_util::contains(result["events"], "onMessage"));
    BOOST_TEST(!json_util::contains(result["events"], "onCommand"));
    BOOST_TEST(result["action"].get<std::string>() == "drop");
}

BOOST_AUTO_TEST_CASE(remove_event_and_server)
{
    nlohmann::json result;

    ctl_->send({
        { "command",        "rule-edit"     },
        { "remove-servers", { "s2" }        },
        { "remove-events",  { "onCommand" } },
        { "index",          0               }
    });
    ctl_->recv([&] (auto, auto msg) {
        result = msg;
    });

    wait_for([&] () {
        return result.is_object();
    });

    BOOST_TEST(result.is_object());

    result = nullptr;
    ctl_->send({
        { "command",        "rule-info"     },
        { "index",          0               }
    });
    ctl_->recv([&] (auto, auto msg) {
        result = msg;
    });

    wait_for([&] () {
        return result.is_object();
    });

    BOOST_TEST(result.is_object());
    BOOST_TEST(json_util::contains(result["servers"], "s1"));
    BOOST_TEST(!json_util::contains(result["servers"], "s2"));
    BOOST_TEST(json_util::contains(result["channels"], "c1"));
    BOOST_TEST(json_util::contains(result["channels"], "c2"));
    BOOST_TEST(json_util::contains(result["plugins"], "p1"));
    BOOST_TEST(json_util::contains(result["plugins"], "p2"));
    BOOST_TEST(json_util::contains(result["events"], "onMessage"));
    BOOST_TEST(!json_util::contains(result["events"], "onCommand"));
    BOOST_TEST(result["action"].get<std::string>() == "drop");
}

BOOST_AUTO_TEST_SUITE_END()

} // !irccd
