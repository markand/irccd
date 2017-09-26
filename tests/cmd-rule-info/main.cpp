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

#include <command.hpp>
#include <command-tester.hpp>
#include <service.hpp>

using namespace irccd;

class RuleInfoCommandTest : public CommandTester {
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
    RuleInfoCommandTest()
        : CommandTester(std::make_unique<rule_info_command>())
    {
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

TEST_F(RuleInfoCommandTest, basic)
{
    m_irccdctl.client().request({
        { "command",    "rule-info" },
        { "index",      0           }
    });

    try {
        poll([&] () {
            return m_result.is_object();
        });

        ASSERT_TRUE(m_result.is_object());

        auto servers = m_result["servers"];
        auto channels = m_result["channels"];
        auto plugins = m_result["plugins"];
        auto events = m_result["events"];

        ASSERT_TRUE(contains(servers, "s1"));
        ASSERT_TRUE(contains(servers, "s2"));
        ASSERT_TRUE(contains(channels, "c1"));
        ASSERT_TRUE(contains(channels, "c2"));
        ASSERT_TRUE(contains(plugins, "p1"));
        ASSERT_TRUE(contains(plugins, "p2"));
        ASSERT_TRUE(contains(events, "onMessage"));
        ASSERT_TRUE(contains(events, "onCommand"));
        ASSERT_EQ("drop", m_result["action"]);
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

TEST_F(RuleInfoCommandTest, outOfBounds)
{
    m_irccdctl.client().request({
        { "command",    "rule-info" },
        { "index",      123         }
    });

    try {
        poll([&] () {
            return m_result.is_object();
        });

        ASSERT_TRUE(m_result.is_object());
        ASSERT_FALSE(m_result["status"]);
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
