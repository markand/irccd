/*
 * main.cpp -- test rule-add remote command
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

#define BOOST_TEST_MODULE "rule-add"
#include <boost/test/unit_test.hpp>

#include <irccd/json_util.hpp>

#include <irccd/command.hpp>
#include <irccd/rule_service.hpp>

#include <command_test.hpp>

namespace irccd {

namespace {

class rule_add_test : public command_test<rule_add_command> {
public:
    rule_add_test()
    {
        daemon_->commands().add(std::make_unique<rule_list_command>());
    }
};

} // !namespace

BOOST_FIXTURE_TEST_SUITE(rule_add_test_suite, rule_add_test)

BOOST_AUTO_TEST_CASE(basic)
{
    nlohmann::json result;

    ctl_->send({
        { "command",    "rule-add"          },
        { "servers",    { "s1", "s2" }      },
        { "channels",   { "c1", "c2" }      },
        { "plugins",    { "p1", "p2" }      },
        { "events",     { "onMessage" }     },
        { "action",     "accept"            },
        { "index",      0                   }
    });
    ctl_->recv([&] (auto, auto msg) {
        result = msg;
    });

    wait_for([&] () {
        return result.is_object();
    });

    BOOST_TEST(result.is_object());

    result = nullptr;
    ctl_->send({{"command", "rule-list"}});
    ctl_->recv([&] (auto, auto msg) {
        result = msg;
    });

    wait_for([&] () {
        return result.is_object();
    });

    BOOST_TEST(result.is_object());

    auto servers = result["list"][0]["servers"];
    auto channels = result["list"][0]["channels"];
    auto plugins = result["list"][0]["plugins"];
    auto events = result["list"][0]["events"];

    BOOST_TEST(json_util::contains(servers, "s1"));
    BOOST_TEST(json_util::contains(servers, "s2"));
    BOOST_TEST(json_util::contains(channels, "c1"));
    BOOST_TEST(json_util::contains(channels, "c2"));
    BOOST_TEST(json_util::contains(plugins, "p1"));
    BOOST_TEST(json_util::contains(plugins, "p2"));
    BOOST_TEST(json_util::contains(events, "onMessage"));
    BOOST_TEST(result["list"][0]["action"].get<std::string>() == "accept");
}

BOOST_AUTO_TEST_CASE(append)
{
    nlohmann::json result;

    ctl_->send({
        { "command",    "rule-add"          },
        { "servers",    { "s1" }            },
        { "channels",   { "c1" }            },
        { "plugins",    { "p1" }            },
        { "events",     { "onMessage" }     },
        { "action",     "accept"            },
        { "index",      0                   }
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
        { "command",    "rule-add"          },
        { "servers",    { "s2" }            },
        { "channels",   { "c2" }            },
        { "plugins",    { "p2" }            },
        { "events",     { "onMessage" }     },
        { "action",     "drop"              },
        { "index",      1                   }
    });
    ctl_->recv([&] (auto, auto msg) {
        result = msg;
    });

    wait_for([&] () {
        return result.is_object();
    });

    BOOST_TEST(result.is_object());

    result = nullptr;
    ctl_->send({{"command", "rule-list"}});
    ctl_->recv([&] (auto, auto msg) {
        result = msg;
    });

    wait_for([&] () {
        return result.is_object();
    });

    BOOST_TEST(result.is_object());
    BOOST_TEST(result["list"].size() == 2U);

    // Rule 0.
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

    // Rule 1.
    {
        auto servers = result["list"][1]["servers"];
        auto channels = result["list"][1]["channels"];
        auto plugins = result["list"][1]["plugins"];
        auto events = result["list"][1]["events"];

        BOOST_TEST(json_util::contains(servers, "s2"));
        BOOST_TEST(json_util::contains(channels, "c2"));
        BOOST_TEST(json_util::contains(plugins, "p2"));
        BOOST_TEST(json_util::contains(events, "onMessage"));
        BOOST_TEST(result["list"][1]["action"].get<std::string>() == "drop");
    }
}

BOOST_AUTO_TEST_SUITE(errors)

BOOST_AUTO_TEST_CASE(invalid_action)
{
    boost::system::error_code result;

    ctl_->send({
        { "command",    "rule-add"  },
        { "action",     "unknown"   }
    });
    ctl_->recv([&] (auto code, auto msg) {
        result = code;
    });

    wait_for([&] {
        return result;
    });

    BOOST_ASSERT(result == rule_error::invalid_action);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

} // !irccd
