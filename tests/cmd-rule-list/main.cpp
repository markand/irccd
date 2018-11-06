/*
 * main.cpp -- test rule-list remote command
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
using namespace irccd::command;

class RuleListCommandTest : public CommandTester {
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
    RuleListCommandTest()
        : CommandTester(std::make_unique<RuleListCommand>())
    {
        m_irccd.rules().add(Rule(
            { "s1", "s2" },
            { "c1", "c2" },
            { "o1", "o2" },
            { "p1", "p2" },
            { "onMessage", "onCommand" },
            RuleAction::Drop
        ));
        m_irccd.rules().add(Rule(
            { "s1", },
            { "c1", },
            { "o1", },
            { "p1", },
            { "onMessage", },
            RuleAction::Accept
        ));
        m_irccdctl.client().request({{ "command", "rule-list" }});
        m_irccdctl.client().onMessage.connect([&] (auto result) {
            m_result = result;
        });
    }
};

TEST_F(RuleListCommandTest, basic)
{
    try {
        poll([&] () {
            return m_result.is_object();
        });

        ASSERT_TRUE(m_result.is_object());
        ASSERT_TRUE(m_result["list"].is_array());
        ASSERT_EQ(2U, m_result["list"].size());

        // Rule 0.
        {
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
        }

        // Rule 1.
        {
            auto servers = m_result["list"][1]["servers"];
            auto channels = m_result["list"][1]["channels"];
            auto plugins = m_result["list"][1]["plugins"];
            auto events = m_result["list"][1]["events"];

            ASSERT_TRUE(contains(servers, "s1"));
            ASSERT_TRUE(contains(channels, "c1"));
            ASSERT_TRUE(contains(plugins, "p1"));
            ASSERT_TRUE(contains(events, "onMessage"));
            ASSERT_EQ("accept", m_result["list"][1]["action"]);
        }
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

TEST_F(RuleListCommandTest, empty)
{
    m_irccd.rules().remove(0);
    m_irccd.rules().remove(0);

    try {
        poll([&] () {
            return m_result.is_object();
        });

        ASSERT_TRUE(m_result.is_object());
        ASSERT_TRUE(m_result["list"].is_array());
        ASSERT_EQ(0U, m_result["list"].size());
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
