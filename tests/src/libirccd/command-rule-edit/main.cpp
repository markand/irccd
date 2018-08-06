/*
 * main.cpp -- test rule-edit remote command
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

#define BOOST_TEST_MODULE "rule-edit"
#include <boost/test/unit_test.hpp>

#include <irccd/json_util.hpp>

#include <irccd/test/command_fixture.hpp>

namespace irccd::test {

namespace {

class rule_edit_fixture : public command_fixture {
public:
    rule_edit_fixture()
    {
        irccd_.rules().add(rule(
            { "s1", "s2" },
            { "c1", "c2" },
            { "o1", "o2" },
            { "p1", "p2" },
            { "onMessage", "onCommand" },
            rule::action::drop
        ));
    }
};

BOOST_FIXTURE_TEST_SUITE(rule_edit_fixture_suite, rule_edit_fixture)

BOOST_AUTO_TEST_CASE(add_server)
{
    request({
        { "command",        "rule-edit"     },
        { "add-servers",    { "new-s3" }    },
        { "index",          0               }
    });

    const auto [json, code] = request({
        { "command",        "rule-info"     },
        { "index",          0               }
    });

    BOOST_TEST(!code);
    BOOST_TEST(json.is_object());
    BOOST_TEST(json_util::contains(json["servers"], "s1"));
    BOOST_TEST(json_util::contains(json["servers"], "s2"));
    BOOST_TEST(json_util::contains(json["servers"], "new-s3"));
    BOOST_TEST(json_util::contains(json["channels"], "c1"));
    BOOST_TEST(json_util::contains(json["channels"], "c2"));
    BOOST_TEST(json_util::contains(json["plugins"], "p1"));
    BOOST_TEST(json_util::contains(json["plugins"], "p2"));
    BOOST_TEST(json_util::contains(json["events"], "onMessage"));
    BOOST_TEST(json_util::contains(json["events"], "onCommand"));
    BOOST_TEST(json["action"].get<std::string>() == "drop");
}

BOOST_AUTO_TEST_CASE(add_channel)
{
    request({
        { "command",        "rule-edit"     },
        { "add-channels",   { "new-c3" }    },
        { "index",          0               }
    });

    const auto [json, code] = request({
        { "command",        "rule-info"     },
        { "index",          0               }
    });

    BOOST_TEST(!code);
    BOOST_TEST(json.is_object());
    BOOST_TEST(json_util::contains(json["servers"], "s1"));
    BOOST_TEST(json_util::contains(json["servers"], "s2"));
    BOOST_TEST(json_util::contains(json["channels"], "c1"));
    BOOST_TEST(json_util::contains(json["channels"], "c2"));
    BOOST_TEST(json_util::contains(json["channels"], "new-c3"));
    BOOST_TEST(json_util::contains(json["plugins"], "p1"));
    BOOST_TEST(json_util::contains(json["plugins"], "p2"));
    BOOST_TEST(json_util::contains(json["events"], "onMessage"));
    BOOST_TEST(json_util::contains(json["events"], "onCommand"));
    BOOST_TEST(json["action"].get<std::string>() == "drop");
}

BOOST_AUTO_TEST_CASE(add_plugin)
{
    request({
        { "command",        "rule-edit"     },
        { "add-plugins",    { "new-p3" }    },
        { "index",          0               }
    });

    const auto [json, code] = request({
        { "command",        "rule-info"     },
        { "index",          0               }
    });

    BOOST_TEST(!code);
    BOOST_TEST(json.is_object());
    BOOST_TEST(json_util::contains(json["servers"], "s1"));
    BOOST_TEST(json_util::contains(json["servers"], "s2"));
    BOOST_TEST(json_util::contains(json["channels"], "c1"));
    BOOST_TEST(json_util::contains(json["channels"], "c2"));
    BOOST_TEST(json_util::contains(json["plugins"], "p1"));
    BOOST_TEST(json_util::contains(json["plugins"], "p2"));
    BOOST_TEST(json_util::contains(json["plugins"], "new-p3"));
    BOOST_TEST(json_util::contains(json["events"], "onMessage"));
    BOOST_TEST(json_util::contains(json["events"], "onCommand"));
    BOOST_TEST(json["action"].get<std::string>() == "drop");
}

BOOST_AUTO_TEST_CASE(add_event)
{
    request({
        { "command",        "rule-edit"     },
        { "add-events",     { "onQuery" }   },
        { "index",          0               }
    });

    const auto [json, code] = request({
        { "command",        "rule-info"     },
        { "index",          0               }
    });

    BOOST_TEST(!code);
    BOOST_TEST(json.is_object());
    BOOST_TEST(json_util::contains(json["servers"], "s1"));
    BOOST_TEST(json_util::contains(json["servers"], "s2"));
    BOOST_TEST(json_util::contains(json["channels"], "c1"));
    BOOST_TEST(json_util::contains(json["channels"], "c2"));
    BOOST_TEST(json_util::contains(json["plugins"], "p1"));
    BOOST_TEST(json_util::contains(json["plugins"], "p2"));
    BOOST_TEST(json_util::contains(json["events"], "onMessage"));
    BOOST_TEST(json_util::contains(json["events"], "onCommand"));
    BOOST_TEST(json_util::contains(json["events"], "onQuery"));
    BOOST_TEST(json["action"].get<std::string>() == "drop");
}

BOOST_AUTO_TEST_CASE(add_event_and_server)
{
    request({
        { "command",        "rule-edit"     },
        { "add-servers",    { "new-s3" }    },
        { "add-events",     { "onQuery" }   },
        { "index",          0               }
    });

    const auto [json, code] = request({
        { "command",        "rule-info"     },
        { "index",          0               }
    });

    BOOST_TEST(!code);
    BOOST_TEST(json.is_object());
    BOOST_TEST(json_util::contains(json["servers"], "s1"));
    BOOST_TEST(json_util::contains(json["servers"], "s2"));
    BOOST_TEST(json_util::contains(json["servers"], "new-s3"));
    BOOST_TEST(json_util::contains(json["channels"], "c1"));
    BOOST_TEST(json_util::contains(json["channels"], "c2"));
    BOOST_TEST(json_util::contains(json["plugins"], "p1"));
    BOOST_TEST(json_util::contains(json["plugins"], "p2"));
    BOOST_TEST(json_util::contains(json["events"], "onMessage"));
    BOOST_TEST(json_util::contains(json["events"], "onCommand"));
    BOOST_TEST(json_util::contains(json["events"], "onQuery"));
    BOOST_TEST(json["action"].get<std::string>() == "drop");
}

BOOST_AUTO_TEST_CASE(change_action)
{
    request({
        { "command",        "rule-edit"     },
        { "action",         "accept"        },
        { "index",          0               }
    });

    const auto [json, code] = request({
        { "command",        "rule-info"     },
        { "index",          0               }
    });

    BOOST_TEST(!code);
    BOOST_TEST(json.is_object());
    BOOST_TEST(json_util::contains(json["servers"], "s1"));
    BOOST_TEST(json_util::contains(json["servers"], "s2"));
    BOOST_TEST(json_util::contains(json["channels"], "c1"));
    BOOST_TEST(json_util::contains(json["channels"], "c2"));
    BOOST_TEST(json_util::contains(json["plugins"], "p1"));
    BOOST_TEST(json_util::contains(json["plugins"], "p2"));
    BOOST_TEST(json_util::contains(json["events"], "onMessage"));
    BOOST_TEST(json_util::contains(json["events"], "onCommand"));
    BOOST_TEST(json["action"].get<std::string>() == "accept");
}

BOOST_AUTO_TEST_CASE(remove_server)
{
    request({
        { "command",        "rule-edit"     },
        { "remove-servers", { "s2" }        },
        { "index",          0               }
    });

    const auto [json, code] = request({
        { "command",        "rule-info"     },
        { "index",          0               }
    });

    BOOST_TEST(!code);
    BOOST_TEST(json.is_object());
    BOOST_TEST(json_util::contains(json["servers"], "s1"));
    BOOST_TEST(!json_util::contains(json["servers"], "s2"));
    BOOST_TEST(json_util::contains(json["channels"], "c1"));
    BOOST_TEST(json_util::contains(json["channels"], "c2"));
    BOOST_TEST(json_util::contains(json["plugins"], "p1"));
    BOOST_TEST(json_util::contains(json["plugins"], "p2"));
    BOOST_TEST(json_util::contains(json["events"], "onMessage"));
    BOOST_TEST(json_util::contains(json["events"], "onCommand"));
    BOOST_TEST(json["action"].get<std::string>() == "drop");
}

BOOST_AUTO_TEST_CASE(remove_channel)
{
    request({
        { "command",        "rule-edit"     },
        { "remove-channels", { "c2" }       },
        { "index",          0               }
    });

    const auto [json, code] = request({
        { "command",        "rule-info"     },
        { "index",          0               }
    });

    BOOST_TEST(!code);
    BOOST_TEST(json.is_object());
    BOOST_TEST(json_util::contains(json["servers"], "s1"));
    BOOST_TEST(json_util::contains(json["servers"], "s2"));
    BOOST_TEST(json_util::contains(json["channels"], "c1"));
    BOOST_TEST(!json_util::contains(json["channels"], "c2"));
    BOOST_TEST(json_util::contains(json["plugins"], "p1"));
    BOOST_TEST(json_util::contains(json["plugins"], "p2"));
    BOOST_TEST(json_util::contains(json["events"], "onMessage"));
    BOOST_TEST(json_util::contains(json["events"], "onCommand"));
    BOOST_TEST(json["action"].get<std::string>() == "drop");
}

BOOST_AUTO_TEST_CASE(remove_plugin)
{
    request({
        { "command",        "rule-edit"     },
        { "remove-plugins", { "p2" }        },
        { "index",          0               }
    });

    const auto [json, code] = request({
        { "command",        "rule-info"     },
        { "index",          0               }
    });

    BOOST_TEST(!code);
    BOOST_TEST(json.is_object());
    BOOST_TEST(json_util::contains(json["servers"], "s1"));
    BOOST_TEST(json_util::contains(json["servers"], "s2"));
    BOOST_TEST(json_util::contains(json["channels"], "c1"));
    BOOST_TEST(json_util::contains(json["channels"], "c2"));
    BOOST_TEST(json_util::contains(json["plugins"], "p1"));
    BOOST_TEST(!json_util::contains(json["plugins"], "p2"));
    BOOST_TEST(json_util::contains(json["events"], "onMessage"));
    BOOST_TEST(json_util::contains(json["events"], "onCommand"));
    BOOST_TEST(json["action"].get<std::string>() == "drop");
}

BOOST_AUTO_TEST_CASE(remove_event)
{
    request({
        { "command",        "rule-edit"     },
        { "remove-events",  { "onCommand" } },
        { "index",          0               }
    });

    const auto [json, code] = request({
        { "command",        "rule-info"     },
        { "index",          0               }
    });

    BOOST_TEST(!code);
    BOOST_TEST(json.is_object());
    BOOST_TEST(json_util::contains(json["servers"], "s1"));
    BOOST_TEST(json_util::contains(json["servers"], "s2"));
    BOOST_TEST(json_util::contains(json["channels"], "c1"));
    BOOST_TEST(json_util::contains(json["channels"], "c2"));
    BOOST_TEST(json_util::contains(json["plugins"], "p1"));
    BOOST_TEST(json_util::contains(json["plugins"], "p2"));
    BOOST_TEST(json_util::contains(json["events"], "onMessage"));
    BOOST_TEST(!json_util::contains(json["events"], "onCommand"));
    BOOST_TEST(json["action"].get<std::string>() == "drop");
}

BOOST_AUTO_TEST_CASE(remove_event_and_server)
{
    request({
        { "command",        "rule-edit"     },
        { "remove-servers", { "s2" }        },
        { "remove-events",  { "onCommand" } },
        { "index",          0               }
    });

    const auto [json, code] = request({
        { "command",        "rule-info"     },
        { "index",          0               }
    });

    BOOST_TEST(!code);
    BOOST_TEST(json.is_object());
    BOOST_TEST(json_util::contains(json["servers"], "s1"));
    BOOST_TEST(!json_util::contains(json["servers"], "s2"));
    BOOST_TEST(json_util::contains(json["channels"], "c1"));
    BOOST_TEST(json_util::contains(json["channels"], "c2"));
    BOOST_TEST(json_util::contains(json["plugins"], "p1"));
    BOOST_TEST(json_util::contains(json["plugins"], "p2"));
    BOOST_TEST(json_util::contains(json["events"], "onMessage"));
    BOOST_TEST(!json_util::contains(json["events"], "onCommand"));
    BOOST_TEST(json["action"].get<std::string>() == "drop");
}

BOOST_AUTO_TEST_SUITE(errors)

BOOST_AUTO_TEST_CASE(invalid_index_1)
{
    const auto [json, code] = request({
        { "command",    "rule-edit" },
        { "index",      -100        },
        { "action",     "drop"      }
    });

    BOOST_TEST(code == rule_error::invalid_index);
    BOOST_TEST(json["error"].get<int>() == rule_error::invalid_index);
    BOOST_TEST(json["errorCategory"].get<std::string>() == "rule");
}

BOOST_AUTO_TEST_CASE(invalid_index_2)
{
    const auto [json, code] = request({
        { "command",    "rule-edit" },
        { "index",      100         },
        { "action",     "drop"      }
    });

    BOOST_TEST(code == rule_error::invalid_index);
    BOOST_TEST(json["error"].get<int>() == rule_error::invalid_index);
    BOOST_TEST(json["errorCategory"].get<std::string>() == "rule");
}

BOOST_AUTO_TEST_CASE(invalid_index_3)
{
    const auto [json, code] = request({
        { "command",    "rule-edit" },
        { "index",      "notaint"   },
        { "action",     "drop"      }
    });

    BOOST_TEST(code == rule_error::invalid_index);
    BOOST_TEST(json["error"].get<int>() == rule_error::invalid_index);
    BOOST_TEST(json["errorCategory"].get<std::string>() == "rule");
}

BOOST_AUTO_TEST_CASE(invalid_action)
{
    const auto [json, code] = request({
        { "command",    "rule-edit" },
        { "index",      0           },
        { "action",     "unknown"   }
    });

    BOOST_TEST(code == rule_error::invalid_action);
    BOOST_TEST(json["error"].get<int>() == rule_error::invalid_action);
    BOOST_TEST(json["errorCategory"].get<std::string>() == "rule");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd::test
