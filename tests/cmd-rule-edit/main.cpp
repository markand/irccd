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
using namespace irccd::command;

class RuleEditCommandTest : public CommandTester {
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
    RuleEditCommandTest()
        : CommandTester(std::make_unique<RuleEditCommand>())
    {
        m_irccd.commands().add(std::make_unique<RuleInfoCommand>());
        m_irccd.rules().add(Rule(
            { "s1", "s2" },
            { "c1", "c2" },
            { "o1", "o2" },
            { "p1", "p2" },
            { "onMessage", "onCommand" },
            RuleAction::Drop
        ));
        m_irccdctl.client().onMessage.connect([&] (auto result) {
            m_result = result;
        });
    }
};

TEST_F(RuleEditCommandTest, addServer)
{
    try {
        m_irccdctl.client().request({
            { "command",        "rule-edit"     },
            { "add-servers",    { "new-s3" }    },
            { "index",          0               }
        });

        poll([&] () {
            return m_result.is_object();
        });

        ASSERT_TRUE(m_result.is_object());
        ASSERT_TRUE(m_result["status"].get<bool>());

        m_result = nullptr;
        m_irccdctl.client().request({
            { "command", "rule-info" },
            { "index", 0 }
        });

        poll([&] () {
            return m_result.is_object();
        });

        ASSERT_TRUE(m_result.is_object());
        ASSERT_TRUE(m_result["status"].get<bool>());
        ASSERT_TRUE(contains(m_result["servers"], "s1"));
        ASSERT_TRUE(contains(m_result["servers"], "s2"));
        ASSERT_TRUE(contains(m_result["servers"], "new-s3"));
        ASSERT_TRUE(contains(m_result["channels"], "c1"));
        ASSERT_TRUE(contains(m_result["channels"], "c2"));
        ASSERT_TRUE(contains(m_result["plugins"], "p1"));
        ASSERT_TRUE(contains(m_result["plugins"], "p2"));
        ASSERT_TRUE(contains(m_result["events"], "onMessage"));
        ASSERT_TRUE(contains(m_result["events"], "onCommand"));
        ASSERT_EQ(m_result["action"].get<std::string>(), "drop");
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

TEST_F(RuleEditCommandTest, addChannel)
{
    try {
        m_irccdctl.client().request({
            { "command",        "rule-edit"     },
            { "add-channels",   { "new-c3" }    },
            { "index",          0               }
        });

        poll([&] () {
            return m_result.is_object();
        });

        ASSERT_TRUE(m_result.is_object());
        ASSERT_TRUE(m_result["status"].get<bool>());

        m_result = nullptr;
        m_irccdctl.client().request({
            { "command", "rule-info" },
            { "index", 0 }
        });

        poll([&] () {
            return m_result.is_object();
        });

        ASSERT_TRUE(m_result.is_object());
        ASSERT_TRUE(m_result["status"].get<bool>());
        ASSERT_TRUE(contains(m_result["servers"], "s1"));
        ASSERT_TRUE(contains(m_result["servers"], "s2"));
        ASSERT_TRUE(contains(m_result["channels"], "c1"));
        ASSERT_TRUE(contains(m_result["channels"], "c2"));
        ASSERT_TRUE(contains(m_result["channels"], "new-c3"));
        ASSERT_TRUE(contains(m_result["plugins"], "p1"));
        ASSERT_TRUE(contains(m_result["plugins"], "p2"));
        ASSERT_TRUE(contains(m_result["events"], "onMessage"));
        ASSERT_TRUE(contains(m_result["events"], "onCommand"));
        ASSERT_EQ(m_result["action"].get<std::string>(), "drop");
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

TEST_F(RuleEditCommandTest, addPlugin)
{
    try {
        m_irccdctl.client().request({
            { "command",        "rule-edit"     },
            { "add-plugins",    { "new-p3" }    },
            { "index",          0               }
        });

        poll([&] () {
            return m_result.is_object();
        });

        ASSERT_TRUE(m_result.is_object());
        ASSERT_TRUE(m_result["status"].get<bool>());

        m_result = nullptr;
        m_irccdctl.client().request({
            { "command", "rule-info" },
            { "index", 0 }
        });

        poll([&] () {
            return m_result.is_object();
        });

        ASSERT_TRUE(m_result.is_object());
        ASSERT_TRUE(m_result["status"].get<bool>());
        ASSERT_TRUE(contains(m_result["servers"], "s1"));
        ASSERT_TRUE(contains(m_result["servers"], "s2"));
        ASSERT_TRUE(contains(m_result["channels"], "c1"));
        ASSERT_TRUE(contains(m_result["channels"], "c2"));
        ASSERT_TRUE(contains(m_result["plugins"], "p1"));
        ASSERT_TRUE(contains(m_result["plugins"], "p2"));
        ASSERT_TRUE(contains(m_result["plugins"], "new-p3"));
        ASSERT_TRUE(contains(m_result["events"], "onMessage"));
        ASSERT_TRUE(contains(m_result["events"], "onCommand"));
        ASSERT_EQ(m_result["action"].get<std::string>(), "drop");
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

TEST_F(RuleEditCommandTest, addEvent)
{
    try {
        m_irccdctl.client().request({
            { "command",        "rule-edit"     },
            { "add-events",     { "onQuery" }   },
            { "index",          0               }
        });

        poll([&] () {
            return m_result.is_object();
        });

        ASSERT_TRUE(m_result.is_object());
        ASSERT_TRUE(m_result["status"].get<bool>());

        m_result = nullptr;
        m_irccdctl.client().request({
            { "command", "rule-info" },
            { "index", 0 }
        });

        poll([&] () {
            return m_result.is_object();
        });

        ASSERT_TRUE(m_result.is_object());
        ASSERT_TRUE(m_result["status"].get<bool>());
        ASSERT_TRUE(contains(m_result["servers"], "s1"));
        ASSERT_TRUE(contains(m_result["servers"], "s2"));
        ASSERT_TRUE(contains(m_result["channels"], "c1"));
        ASSERT_TRUE(contains(m_result["channels"], "c2"));
        ASSERT_TRUE(contains(m_result["plugins"], "p1"));
        ASSERT_TRUE(contains(m_result["plugins"], "p2"));
        ASSERT_TRUE(contains(m_result["events"], "onMessage"));
        ASSERT_TRUE(contains(m_result["events"], "onCommand"));
        ASSERT_TRUE(contains(m_result["events"], "onQuery"));
        ASSERT_EQ(m_result["action"].get<std::string>(), "drop");
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

TEST_F(RuleEditCommandTest, addEventAndServer)
{
    try {
        m_irccdctl.client().request({
            { "command",        "rule-edit"     },
            { "add-servers",    { "new-s3" }    },
            { "add-events",     { "onQuery" }   },
            { "index",          0               }
        });

        poll([&] () {
            return m_result.is_object();
        });

        ASSERT_TRUE(m_result.is_object());
        ASSERT_TRUE(m_result["status"].get<bool>());

        m_result = nullptr;
        m_irccdctl.client().request({
            { "command", "rule-info" },
            { "index", 0 }
        });

        poll([&] () {
            return m_result.is_object();
        });

        ASSERT_TRUE(m_result.is_object());
        ASSERT_TRUE(m_result["status"].get<bool>());
        ASSERT_TRUE(contains(m_result["servers"], "s1"));
        ASSERT_TRUE(contains(m_result["servers"], "s2"));
        ASSERT_TRUE(contains(m_result["servers"], "new-s3"));
        ASSERT_TRUE(contains(m_result["channels"], "c1"));
        ASSERT_TRUE(contains(m_result["channels"], "c2"));
        ASSERT_TRUE(contains(m_result["plugins"], "p1"));
        ASSERT_TRUE(contains(m_result["plugins"], "p2"));
        ASSERT_TRUE(contains(m_result["events"], "onMessage"));
        ASSERT_TRUE(contains(m_result["events"], "onCommand"));
        ASSERT_TRUE(contains(m_result["events"], "onQuery"));
        ASSERT_EQ(m_result["action"].get<std::string>(), "drop");
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

TEST_F(RuleEditCommandTest, changeAction)
{
    try {
        m_irccdctl.client().request({
            { "command",        "rule-edit"     },
            { "action",         "accept"        },
            { "index",          0               }
        });

        poll([&] () {
            return m_result.is_object();
        });

        ASSERT_TRUE(m_result.is_object());
        ASSERT_TRUE(m_result["status"].get<bool>());

        m_result = nullptr;
        m_irccdctl.client().request({
            { "command", "rule-info" },
            { "index", 0 }
        });

        poll([&] () {
            return m_result.is_object();
        });

        ASSERT_TRUE(m_result.is_object());
        ASSERT_TRUE(m_result["status"].get<bool>());
        ASSERT_TRUE(contains(m_result["servers"], "s1"));
        ASSERT_TRUE(contains(m_result["servers"], "s2"));
        ASSERT_TRUE(contains(m_result["channels"], "c1"));
        ASSERT_TRUE(contains(m_result["channels"], "c2"));
        ASSERT_TRUE(contains(m_result["plugins"], "p1"));
        ASSERT_TRUE(contains(m_result["plugins"], "p2"));
        ASSERT_TRUE(contains(m_result["events"], "onMessage"));
        ASSERT_TRUE(contains(m_result["events"], "onCommand"));
        ASSERT_EQ(m_result["action"].get<std::string>(), "accept");
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

TEST_F(RuleEditCommandTest, removeServer)
{
    try {
        m_irccdctl.client().request({
            { "command",        "rule-edit"     },
            { "remove-servers", { "s2" }        },
            { "index",          0               }
        });

        poll([&] () {
            return m_result.is_object();
        });

        ASSERT_TRUE(m_result.is_object());
        ASSERT_TRUE(m_result["status"].get<bool>());

        m_result = nullptr;
        m_irccdctl.client().request({
            { "command", "rule-info" },
            { "index", 0 }
        });

        poll([&] () {
            return m_result.is_object();
        });

        ASSERT_TRUE(m_result.is_object());
        ASSERT_TRUE(m_result["status"].get<bool>());
        ASSERT_TRUE(contains(m_result["servers"], "s1"));
        ASSERT_FALSE(contains(m_result["servers"], "s2"));
        ASSERT_TRUE(contains(m_result["channels"], "c1"));
        ASSERT_TRUE(contains(m_result["channels"], "c2"));
        ASSERT_TRUE(contains(m_result["plugins"], "p1"));
        ASSERT_TRUE(contains(m_result["plugins"], "p2"));
        ASSERT_TRUE(contains(m_result["events"], "onMessage"));
        ASSERT_TRUE(contains(m_result["events"], "onCommand"));
        ASSERT_EQ(m_result["action"].get<std::string>(), "drop");
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

TEST_F(RuleEditCommandTest, removeChannel)
{
    try {
        m_irccdctl.client().request({
            { "command",        "rule-edit"     },
            { "remove-channels", { "c2" }       },
            { "index",          0               }
        });

        poll([&] () {
            return m_result.is_object();
        });

        ASSERT_TRUE(m_result.is_object());
        ASSERT_TRUE(m_result["status"].get<bool>());

        m_result = nullptr;
        m_irccdctl.client().request({
            { "command", "rule-info" },
            { "index", 0 }
        });

        poll([&] () {
            return m_result.is_object();
        });

        ASSERT_TRUE(m_result.is_object());
        ASSERT_TRUE(m_result["status"].get<bool>());
        ASSERT_TRUE(contains(m_result["servers"], "s1"));
        ASSERT_TRUE(contains(m_result["servers"], "s2"));
        ASSERT_TRUE(contains(m_result["channels"], "c1"));
        ASSERT_FALSE(contains(m_result["channels"], "c2"));
        ASSERT_TRUE(contains(m_result["plugins"], "p1"));
        ASSERT_TRUE(contains(m_result["plugins"], "p2"));
        ASSERT_TRUE(contains(m_result["events"], "onMessage"));
        ASSERT_TRUE(contains(m_result["events"], "onCommand"));
        ASSERT_EQ(m_result["action"].get<std::string>(), "drop");
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

TEST_F(RuleEditCommandTest, removePlugin)
{
    try {
        m_irccdctl.client().request({
            { "command",        "rule-edit"     },
            { "remove-plugins", { "p2" }        },
            { "index",          0               }
        });

        poll([&] () {
            return m_result.is_object();
        });

        ASSERT_TRUE(m_result.is_object());
        ASSERT_TRUE(m_result["status"].get<bool>());

        m_result = nullptr;
        m_irccdctl.client().request({
            { "command", "rule-info" },
            { "index", 0 }
        });

        poll([&] () {
            return m_result.is_object();
        });

        ASSERT_TRUE(m_result.is_object());
        ASSERT_TRUE(m_result["status"].get<bool>());
        ASSERT_TRUE(contains(m_result["servers"], "s1"));
        ASSERT_TRUE(contains(m_result["servers"], "s2"));
        ASSERT_TRUE(contains(m_result["channels"], "c1"));
        ASSERT_TRUE(contains(m_result["channels"], "c2"));
        ASSERT_TRUE(contains(m_result["plugins"], "p1"));
        ASSERT_FALSE(contains(m_result["plugins"], "p2"));
        ASSERT_TRUE(contains(m_result["events"], "onMessage"));
        ASSERT_TRUE(contains(m_result["events"], "onCommand"));
        ASSERT_EQ(m_result["action"].get<std::string>(), "drop");
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

TEST_F(RuleEditCommandTest, removeEvent)
{
    try {
        m_irccdctl.client().request({
            { "command",        "rule-edit"     },
            { "remove-events",  { "onCommand" } },
            { "index",          0               }
        });

        poll([&] () {
            return m_result.is_object();
        });

        ASSERT_TRUE(m_result.is_object());
        ASSERT_TRUE(m_result["status"].get<bool>());

        m_result = nullptr;
        m_irccdctl.client().request({
            { "command", "rule-info" },
            { "index", 0 }
        });

        poll([&] () {
            return m_result.is_object();
        });

        ASSERT_TRUE(m_result.is_object());
        ASSERT_TRUE(m_result["status"].get<bool>());
        ASSERT_TRUE(contains(m_result["servers"], "s1"));
        ASSERT_TRUE(contains(m_result["servers"], "s2"));
        ASSERT_TRUE(contains(m_result["channels"], "c1"));
        ASSERT_TRUE(contains(m_result["channels"], "c2"));
        ASSERT_TRUE(contains(m_result["plugins"], "p1"));
        ASSERT_TRUE(contains(m_result["plugins"], "p2"));
        ASSERT_TRUE(contains(m_result["events"], "onMessage"));
        ASSERT_FALSE(contains(m_result["events"], "onCommand"));
        ASSERT_EQ(m_result["action"].get<std::string>(), "drop");
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

TEST_F(RuleEditCommandTest, removeEventAndServer)
{
    try {
        m_irccdctl.client().request({
            { "command",        "rule-edit"     },
            { "remove-servers", { "s2" }        },
            { "remove-events",  { "onCommand" } },
            { "index",          0               }
        });

        poll([&] () {
            return m_result.is_object();
        });

        ASSERT_TRUE(m_result.is_object());
        ASSERT_TRUE(m_result["status"].get<bool>());

        m_result = nullptr;
        m_irccdctl.client().request({
            { "command", "rule-info" },
            { "index", 0 }
        });

        poll([&] () {
            return m_result.is_object();
        });

        ASSERT_TRUE(m_result.is_object());
        ASSERT_TRUE(m_result["status"].get<bool>());
        ASSERT_TRUE(contains(m_result["servers"], "s1"));
        ASSERT_FALSE(contains(m_result["servers"], "s2"));
        ASSERT_TRUE(contains(m_result["channels"], "c1"));
        ASSERT_TRUE(contains(m_result["channels"], "c2"));
        ASSERT_TRUE(contains(m_result["plugins"], "p1"));
        ASSERT_TRUE(contains(m_result["plugins"], "p2"));
        ASSERT_TRUE(contains(m_result["events"], "onMessage"));
        ASSERT_FALSE(contains(m_result["events"], "onCommand"));
        ASSERT_EQ(m_result["action"].get<std::string>(), "drop");
    } catch (const std::exception &ex) {
        FAIL() << ex.what();
    }
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
