/*
 * main.cpp -- test rule-remove remote command
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

#include <command.hpp>
#include <command-tester.hpp>
#include <service.hpp>

using namespace irccd;

class RuleRemoveCommandTest : public CommandTester {
protected:
    nlohmann::json m_result;

    /*
     * Rule sets are unordered so use this function to search a string in
     * the JSON array.
     */
    inline bool contains(const nlohmann::json &array, const std::string &str)
    {
        for (const auto &v : array)
            if (v.is_string() && v == str)
                return true;

        return false;
    }

public:
    RuleRemoveCommandTest()
        : CommandTester(std::make_unique<rule_remove_command>())
    {
        m_irccd.commands().add(std::make_unique<rule_list_command>());
        m_irccd.rules().add(rule(
            { "s1", "s2" },
            { "c1", "c2" },
            { "o1", "o2" },
            { "p1", "p2" },
            { "onMessage", "onCommand" },
            rule::action_type::drop
        ));
        m_irccd.rules().add(rule(
            { "s1", },
            { "c1", },
            { "o1", },
            { "p1", },
            { "onMessage", },
            rule::action_type::accept
        ));
        m_irccdctl.client().onMessage.connect([&] (auto result) {
            m_result = result;
        });
    }
};

TEST_F(RuleRemoveCommandTest, basic)
{
    try {
        m_irccdctl.client().request({
            { "command",    "rule-remove"   },
            { "index",      1               }
        });

        poll([&] () {
            return m_result.is_object();
        });

        ASSERT_TRUE(m_result.is_object());
        ASSERT_TRUE(m_result["status"].get<bool>());

        m_result = nullptr;
        m_irccdctl.client().request({{ "command", "rule-list" }});

        poll([&] () {
            return m_result.is_object();
        });

        ASSERT_TRUE(m_result["list"].is_array());
        ASSERT_EQ(1U, m_result["list"].size());

        auto servers = m_result["list"][0]["servers"];
        auto channels = m_result["list"][0]["channels"];
        auto plugins = m_result["list"][0]["plugins"];
        auto events = m_result["list"][0]["events"];

        ASSERT_TRUE(contains(servers, "s1"));
        ASSERT_TRUE(contains(servers, "s2"));
        ASSERT_TRUE(contains(channels, "c1"));
        ASSERT_TRUE(contains(channels, "c2"));
        ASSERT_TRUE(contains(plugins, "p1"));
        ASSERT_TRUE(contains(plugins, "p2"));
        ASSERT_TRUE(contains(events, "onMessage"));
        ASSERT_TRUE(contains(events, "onCommand"));
        ASSERT_EQ("drop", m_result["list"][0]["action"]);
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

TEST_F(RuleRemoveCommandTest, empty)
{
    m_irccd.rules().remove(0);
    m_irccd.rules().remove(0);

    try {
        m_irccdctl.client().request({
            { "command",    "rule-remove"   },
            { "index",      1               }
        });

        poll([&] () {
            return m_result.is_object();
        });

        ASSERT_TRUE(m_result.is_object());
        ASSERT_FALSE(m_result["status"].get<bool>());
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

TEST_F(RuleRemoveCommandTest, outOfBounds)
{
    try {
        m_irccdctl.client().request({
            { "command",    "rule-remove"   },
            { "index",      123             }
        });

        poll([&] () {
            return m_result.is_object();
        });

        ASSERT_TRUE(m_result.is_object());
        ASSERT_FALSE(m_result["status"].get<bool>());
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
