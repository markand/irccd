/*
 * main.cpp -- test rule-move remote command
 *
 * Copyright (c) 2013-2020 David Demelier <markand@malikania.fr>
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

#include <irccd/test/command_fixture.hpp>

namespace irccd {

namespace {

class rule_move_fixture : public test::command_fixture {
public:
	rule_move_fixture()
	{
		bot_.get_rules().add(daemon::rule{
			{ "s0" },
			{ "c0" },
			{ "o0" },
			{ "p0" },
			{ "onMessage" },
			daemon::rule::action_type::drop
		});
		bot_.get_rules().add(daemon::rule{
			{ "s1", },
			{ "c1", },
			{ "o1", },
			{ "p1", },
			{ "onMessage", },
			daemon::rule::action_type::accept
		});
		bot_.get_rules().add(daemon::rule{
			{ "s2", },
			{ "c2", },
			{ "o2", },
			{ "p2", },
			{ "onMessage", },
			daemon::rule::action_type::accept
		});
	}
};

BOOST_FIXTURE_TEST_SUITE(rule_move_fixture_suite, rule_move_fixture)

BOOST_AUTO_TEST_CASE(backward)
{
	request({
		{ "command",    "rule-move" },
		{ "from",       2U          },
		{ "to",         0U          }
	});

	stream_->clear();

	const auto json = request({{ "command", "rule-list" }});

	BOOST_TEST(json.size() == 2U);
	BOOST_TEST(json["command"].get<std::string>() == "rule-list");

	// Rule 2.
	{
		auto servers = json["list"][0]["servers"];
		auto channels = json["list"][0]["channels"];
		auto plugins = json["list"][0]["plugins"];
		auto events = json["list"][0]["events"];

		BOOST_TEST(json_util::contains(servers, "s2"));
		BOOST_TEST(json_util::contains(channels, "c2"));
		BOOST_TEST(json_util::contains(plugins, "p2"));
		BOOST_TEST(json_util::contains(events, "onMessage"));
		BOOST_TEST(json["list"][0]["action"].get<std::string>() == "accept");
	}

	// Rule 0.
	{
		auto servers = json["list"][1]["servers"];
		auto channels = json["list"][1]["channels"];
		auto plugins = json["list"][1]["plugins"];
		auto events = json["list"][1]["events"];

		BOOST_TEST(json_util::contains(servers, "s0"));
		BOOST_TEST(json_util::contains(channels, "c0"));
		BOOST_TEST(json_util::contains(plugins, "p0"));
		BOOST_TEST(json_util::contains(events, "onMessage"));
		BOOST_TEST(json["list"][1]["action"].get<std::string>() == "drop");
	}

	// Rule 1.
	{
		auto servers = json["list"][2]["servers"];
		auto channels = json["list"][2]["channels"];
		auto plugins = json["list"][2]["plugins"];
		auto events = json["list"][2]["events"];

		BOOST_TEST(json_util::contains(servers, "s1"));
		BOOST_TEST(json_util::contains(channels, "c1"));
		BOOST_TEST(json_util::contains(plugins, "p1"));
		BOOST_TEST(json_util::contains(events, "onMessage"));
		BOOST_TEST(json["list"][2]["action"].get<std::string>() == "accept");
	}
}

BOOST_AUTO_TEST_CASE(upward)
{
	request({
		{ "command",    "rule-move" },
		{ "from",       0U          },
		{ "to",         2U          }
	});

	stream_->clear();

	const auto json = request({{ "command", "rule-list" }});

	BOOST_TEST(json.size() == 2U);
	BOOST_TEST(json["command"].get<std::string>() == "rule-list");

	// Rule 1.
	{
		auto servers = json["list"][0]["servers"];
		auto channels = json["list"][0]["channels"];
		auto plugins = json["list"][0]["plugins"];
		auto events = json["list"][0]["events"];

		BOOST_TEST(json_util::contains(servers, "s1"));
		BOOST_TEST(json_util::contains(channels, "c1"));
		BOOST_TEST(json_util::contains(plugins, "p1"));
		BOOST_TEST(json_util::contains(events, "onMessage"));
		BOOST_TEST(json["list"][0]["action"].get<std::string>() == "accept");
	}

	// Rule 2.
	{
		auto servers = json["list"][1]["servers"];
		auto channels = json["list"][1]["channels"];
		auto plugins = json["list"][1]["plugins"];
		auto events = json["list"][1]["events"];

		BOOST_TEST(json_util::contains(servers, "s2"));
		BOOST_TEST(json_util::contains(channels, "c2"));
		BOOST_TEST(json_util::contains(plugins, "p2"));
		BOOST_TEST(json_util::contains(events, "onMessage"));
		BOOST_TEST(json["list"][1]["action"].get<std::string>() == "accept");
	}

	// Rule 0.
	{
		auto servers = json["list"][2]["servers"];
		auto channels = json["list"][2]["channels"];
		auto plugins = json["list"][2]["plugins"];
		auto events = json["list"][2]["events"];

		BOOST_TEST(json_util::contains(servers, "s0"));
		BOOST_TEST(json_util::contains(channels, "c0"));
		BOOST_TEST(json_util::contains(plugins, "p0"));
		BOOST_TEST(json_util::contains(events, "onMessage"));
		BOOST_TEST(json["list"][2]["action"].get<std::string>() == "drop");
	}
}

BOOST_AUTO_TEST_CASE(same)
{
	request({
		{ "command",    "rule-move" },
		{ "from",       1U          },
		{ "to",         1U          }
	});

	stream_->clear();

	const auto json = request({{ "command", "rule-list" }});

	BOOST_TEST(json.size() == 2U);
	BOOST_TEST(json["command"].get<std::string>() == "rule-list");

	// Rule 0.
	{
		auto servers = json["list"][0]["servers"];
		auto channels = json["list"][0]["channels"];
		auto plugins = json["list"][0]["plugins"];
		auto events = json["list"][0]["events"];

		BOOST_TEST(json_util::contains(servers, "s0"));
		BOOST_TEST(json_util::contains(channels, "c0"));
		BOOST_TEST(json_util::contains(plugins, "p0"));
		BOOST_TEST(json_util::contains(events, "onMessage"));
		BOOST_TEST(json["list"][0]["action"].get<std::string>() == "drop");
	}

	// Rule 1.
	{
		auto servers = json["list"][1]["servers"];
		auto channels = json["list"][1]["channels"];
		auto plugins = json["list"][1]["plugins"];
		auto events = json["list"][1]["events"];

		BOOST_TEST(json_util::contains(servers, "s1"));
		BOOST_TEST(json_util::contains(channels, "c1"));
		BOOST_TEST(json_util::contains(plugins, "p1"));
		BOOST_TEST(json_util::contains(events, "onMessage"));
		BOOST_TEST(json["list"][1]["action"].get<std::string>() == "accept");
	}

	// Rule 2.
	{
		auto servers = json["list"][2]["servers"];
		auto channels = json["list"][2]["channels"];
		auto plugins = json["list"][2]["plugins"];
		auto events = json["list"][2]["events"];

		BOOST_TEST(json_util::contains(servers, "s2"));
		BOOST_TEST(json_util::contains(channels, "c2"));
		BOOST_TEST(json_util::contains(plugins, "p2"));
		BOOST_TEST(json_util::contains(events, "onMessage"));
		BOOST_TEST(json["list"][2]["action"].get<std::string>() == "accept");
	}
}

BOOST_AUTO_TEST_CASE(beyond)
{
	request({
		{ "command",    "rule-move" },
		{ "from",       0U          },
		{ "to",         123U        }
	});

	stream_->clear();

	const auto json = request({{ "command", "rule-list" }});

	BOOST_TEST(json.size() == 2U);
	BOOST_TEST(json["command"].get<std::string>() == "rule-list");

	// Rule 1.
	{
		auto servers = json["list"][0]["servers"];
		auto channels = json["list"][0]["channels"];
		auto plugins = json["list"][0]["plugins"];
		auto events = json["list"][0]["events"];

		BOOST_TEST(json_util::contains(servers, "s1"));
		BOOST_TEST(json_util::contains(channels, "c1"));
		BOOST_TEST(json_util::contains(plugins, "p1"));
		BOOST_TEST(json_util::contains(events, "onMessage"));
		BOOST_TEST(json["list"][0]["action"].get<std::string>() == "accept");
	}

	// Rule 2.
	{
		auto servers = json["list"][1]["servers"];
		auto channels = json["list"][1]["channels"];
		auto plugins = json["list"][1]["plugins"];
		auto events = json["list"][1]["events"];

		BOOST_TEST(json_util::contains(servers, "s2"));
		BOOST_TEST(json_util::contains(channels, "c2"));
		BOOST_TEST(json_util::contains(plugins, "p2"));
		BOOST_TEST(json_util::contains(events, "onMessage"));
		BOOST_TEST(json["list"][1]["action"].get<std::string>() == "accept");
	}

	// Rule 0.
	{
		auto servers = json["list"][2]["servers"];
		auto channels = json["list"][2]["channels"];
		auto plugins = json["list"][2]["plugins"];
		auto events = json["list"][2]["events"];

		BOOST_TEST(json_util::contains(servers, "s0"));
		BOOST_TEST(json_util::contains(channels, "c0"));
		BOOST_TEST(json_util::contains(plugins, "p0"));
		BOOST_TEST(json_util::contains(events, "onMessage"));
		BOOST_TEST(json["list"][2]["action"].get<std::string>() == "drop");
	}
}

BOOST_AUTO_TEST_SUITE(errors)

BOOST_AUTO_TEST_CASE(invalid_index_1_from)
{
	const auto json = request({
		{ "command",    "rule-move" },
		{ "from",       -100        },
		{ "to",         0U          }
	});

	BOOST_TEST(json.size() == 4U);
	BOOST_TEST(json["command"].get<std::string>() == "rule-move");
	BOOST_TEST(json["error"].get<int>() == daemon::rule_error::invalid_index);
	BOOST_TEST(json["errorCategory"].get<std::string>() == "rule");
}

BOOST_AUTO_TEST_CASE(invalid_index_1_to)
{
	const auto json = request({
		{ "command",    "rule-move" },
		{ "from",       0U          },
		{ "to",         -100        }
	});

	BOOST_TEST(json.size() == 4U);
	BOOST_TEST(json["command"].get<std::string>() == "rule-move");
	BOOST_TEST(json["error"].get<int>() == daemon::rule_error::invalid_index);
	BOOST_TEST(json["errorCategory"].get<std::string>() == "rule");
}

BOOST_AUTO_TEST_CASE(invalid_index_2_from)
{
	const auto json = request({
		{ "command",    "rule-move" },
		{ "from",       100U        },
		{ "to",         0U          }
	});

	BOOST_TEST(json.size() == 4U);
	BOOST_TEST(json["command"].get<std::string>() == "rule-move");
	BOOST_TEST(json["error"].get<int>() == daemon::rule_error::invalid_index);
	BOOST_TEST(json["errorCategory"].get<std::string>() == "rule");
}

BOOST_AUTO_TEST_CASE(invalid_index_3_from)
{
	const auto json = request({
		{ "command",    "rule-move" },
		{ "from",       "notaint"   },
		{ "to",         0U          }
	});

	BOOST_TEST(json.size() == 4U);
	BOOST_TEST(json["command"].get<std::string>() == "rule-move");
	BOOST_TEST(json["error"].get<int>() == daemon::rule_error::invalid_index);
	BOOST_TEST(json["errorCategory"].get<std::string>() == "rule");
}

BOOST_AUTO_TEST_CASE(invalid_index_3_to)
{
	const auto json = request({
		{ "command",    "rule-move" },
		{ "from",       0U          },
		{ "to",         "notaint"   }
	});

	BOOST_TEST(json.size() == 4U);
	BOOST_TEST(json["command"].get<std::string>() == "rule-move");
	BOOST_TEST(json["error"].get<int>() == daemon::rule_error::invalid_index);
	BOOST_TEST(json["errorCategory"].get<std::string>() == "rule");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
