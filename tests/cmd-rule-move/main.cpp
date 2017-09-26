/*
 * main.cpp -- test rule-move remote command
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

class RuleMoveCommandTest : public CommandTester {
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
    RuleMoveCommandTest()
        : CommandTester(std::make_unique<rule_move_command>())
    {
        m_irccd.commands().add(std::make_unique<rule_list_command>());
        m_irccd.rules().add(rule(
            { "s0" },
            { "c0" },
            { "o0" },
            { "p0" },
            { "onMessage" },
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
        m_irccd.rules().add(rule(
            { "s2", },
            { "c2", },
            { "o2", },
            { "p2", },
            { "onMessage", },
            rule::action_type::accept
        ));
        m_irccdctl.client().onMessage.connect([&] (auto result) {
            m_result = result;
        });
    }
};

TEST_F(RuleMoveCommandTest, backward)
{
    try {
        m_irccdctl.client().request({
            { "command",    "rule-move" },
            { "from",       2           },
            { "to",         0           }
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

        ASSERT_TRUE(m_result.is_object());
        ASSERT_TRUE(m_result["status"].get<bool>());

        // Rule 2.
        {
            auto servers = m_result["list"][0]["servers"];
            auto channels = m_result["list"][0]["channels"];
            auto plugins = m_result["list"][0]["plugins"];
            auto events = m_result["list"][0]["events"];

            ASSERT_TRUE(contains(servers, "s2"));
            ASSERT_TRUE(contains(channels, "c2"));
            ASSERT_TRUE(contains(plugins, "p2"));
            ASSERT_TRUE(contains(events, "onMessage"));
            ASSERT_EQ("accept", m_result["list"][0]["action"].get<std::string>());
        }

        // Rule 0.
        {
            auto servers = m_result["list"][1]["servers"];
            auto channels = m_result["list"][1]["channels"];
            auto plugins = m_result["list"][1]["plugins"];
            auto events = m_result["list"][1]["events"];

            ASSERT_TRUE(contains(servers, "s0"));
            ASSERT_TRUE(contains(channels, "c0"));
            ASSERT_TRUE(contains(plugins, "p0"));
            ASSERT_TRUE(contains(events, "onMessage"));
            ASSERT_EQ("drop", m_result["list"][1]["action"].get<std::string>());
        }

        // Rule 1.
        {
            auto servers = m_result["list"][2]["servers"];
            auto channels = m_result["list"][2]["channels"];
            auto plugins = m_result["list"][2]["plugins"];
            auto events = m_result["list"][2]["events"];

            ASSERT_TRUE(contains(servers, "s1"));
            ASSERT_TRUE(contains(channels, "c1"));
            ASSERT_TRUE(contains(plugins, "p1"));
            ASSERT_TRUE(contains(events, "onMessage"));
            ASSERT_EQ("accept", m_result["list"][2]["action"].get<std::string>());
        }
    } catch (const std::exception& ex) {
        FAIL() << ex.what();
    }
}

TEST_F(RuleMoveCommandTest, upward)
{
    try {
        m_irccdctl.client().request({
            { "command",    "rule-move" },
            { "from",       0           },
            { "to",         2           }
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

        ASSERT_TRUE(m_result.is_object());
        ASSERT_TRUE(m_result["status"].get<bool>());

        // Rule 1.
        {
            auto servers = m_result["list"][0]["servers"];
            auto channels = m_result["list"][0]["channels"];
            auto plugins = m_result["list"][0]["plugins"];
            auto events = m_result["list"][0]["events"];

            ASSERT_TRUE(contains(servers, "s1"));
            ASSERT_TRUE(contains(channels, "c1"));
            ASSERT_TRUE(contains(plugins, "p1"));
            ASSERT_TRUE(contains(events, "onMessage"));
            ASSERT_EQ("accept", m_result["list"][0]["action"].get<std::string>());
        }

        // Rule 2.
        {
            auto servers = m_result["list"][1]["servers"];
            auto channels = m_result["list"][1]["channels"];
            auto plugins = m_result["list"][1]["plugins"];
            auto events = m_result["list"][1]["events"];

            ASSERT_TRUE(contains(servers, "s2"));
            ASSERT_TRUE(contains(channels, "c2"));
            ASSERT_TRUE(contains(plugins, "p2"));
            ASSERT_TRUE(contains(events, "onMessage"));
            ASSERT_EQ("accept", m_result["list"][1]["action"].get<std::string>());
        }

        // Rule 0.
        {
            auto servers = m_result["list"][2]["servers"];
            auto channels = m_result["list"][2]["channels"];
            auto plugins = m_result["list"][2]["plugins"];
            auto events = m_result["list"][2]["events"];

            ASSERT_TRUE(contains(servers, "s0"));
            ASSERT_TRUE(contains(channels, "c0"));
            ASSERT_TRUE(contains(plugins, "p0"));
            ASSERT_TRUE(contains(events, "onMessage"));
            ASSERT_EQ("drop", m_result["list"][2]["action"].get<std::string>());
        }
    } catch (const std::exception& ex) {
        FAIL() << ex.what();
    }
}

TEST_F(RuleMoveCommandTest, same)
{
    try {
        m_irccdctl.client().request({
            { "command",    "rule-move" },
            { "from",       1           },
            { "to",         1           }
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

        ASSERT_TRUE(m_result.is_object());
        ASSERT_TRUE(m_result["status"].get<bool>());

        // Rule 0.
        {
            auto servers = m_result["list"][0]["servers"];
            auto channels = m_result["list"][0]["channels"];
            auto plugins = m_result["list"][0]["plugins"];
            auto events = m_result["list"][0]["events"];

            ASSERT_TRUE(contains(servers, "s0"));
            ASSERT_TRUE(contains(channels, "c0"));
            ASSERT_TRUE(contains(plugins, "p0"));
            ASSERT_TRUE(contains(events, "onMessage"));
            ASSERT_EQ("drop", m_result["list"][0]["action"].get<std::string>());
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
            ASSERT_EQ("accept", m_result["list"][1]["action"].get<std::string>());
        }

        // Rule 2.
        {
            auto servers = m_result["list"][2]["servers"];
            auto channels = m_result["list"][2]["channels"];
            auto plugins = m_result["list"][2]["plugins"];
            auto events = m_result["list"][2]["events"];

            ASSERT_TRUE(contains(servers, "s2"));
            ASSERT_TRUE(contains(channels, "c2"));
            ASSERT_TRUE(contains(plugins, "p2"));
            ASSERT_TRUE(contains(events, "onMessage"));
            ASSERT_EQ("accept", m_result["list"][2]["action"].get<std::string>());
        }
    } catch (const std::exception& ex) {
        FAIL() << ex.what();
    }
}

TEST_F(RuleMoveCommandTest, beyond)
{
    try {
        m_irccdctl.client().request({
            { "command",    "rule-move" },
            { "from",       0           },
            { "to",         123         }
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

        ASSERT_TRUE(m_result.is_object());
        ASSERT_TRUE(m_result["status"].get<bool>());

        // Rule 1.
        {
            auto servers = m_result["list"][0]["servers"];
            auto channels = m_result["list"][0]["channels"];
            auto plugins = m_result["list"][0]["plugins"];
            auto events = m_result["list"][0]["events"];

            ASSERT_TRUE(contains(servers, "s1"));
            ASSERT_TRUE(contains(channels, "c1"));
            ASSERT_TRUE(contains(plugins, "p1"));
            ASSERT_TRUE(contains(events, "onMessage"));
            ASSERT_EQ("accept", m_result["list"][0]["action"].get<std::string>());
        }

        // Rule 2.
        {
            auto servers = m_result["list"][1]["servers"];
            auto channels = m_result["list"][1]["channels"];
            auto plugins = m_result["list"][1]["plugins"];
            auto events = m_result["list"][1]["events"];

            ASSERT_TRUE(contains(servers, "s2"));
            ASSERT_TRUE(contains(channels, "c2"));
            ASSERT_TRUE(contains(plugins, "p2"));
            ASSERT_TRUE(contains(events, "onMessage"));
            ASSERT_EQ("accept", m_result["list"][1]["action"].get<std::string>());
        }

        // Rule 0.
        {
            auto servers = m_result["list"][2]["servers"];
            auto channels = m_result["list"][2]["channels"];
            auto plugins = m_result["list"][2]["plugins"];
            auto events = m_result["list"][2]["events"];

            ASSERT_TRUE(contains(servers, "s0"));
            ASSERT_TRUE(contains(channels, "c0"));
            ASSERT_TRUE(contains(plugins, "p0"));
            ASSERT_TRUE(contains(events, "onMessage"));
            ASSERT_EQ("drop", m_result["list"][2]["action"].get<std::string>());
        }
    } catch (const std::exception& ex) {
        FAIL() << ex.what();
    }
}

TEST_F(RuleMoveCommandTest, outOfBounds)
{
    try {
        m_irccdctl.client().request({
            { "command",    "rule-move" },
            { "from",       1024        },
            { "to",         0           }
        });

        poll([&] () {
            return m_result.is_object();
        });

        ASSERT_TRUE(m_result.is_object());
        ASSERT_FALSE(m_result["status"].get<bool>());
    } catch (const std::exception& ex) {
        FAIL() << ex.what();
    }
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
