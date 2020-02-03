/*
 * main.cpp -- test rule-remove remote command
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

#define BOOST_TEST_MODULE "rule-remove"
#include <boost/test/unit_test.hpp>

#include <irccd/json_util.hpp>

#include <irccd/test/command_fixture.hpp>

namespace irccd {

namespace {

class rule_remove_fixture : public test::command_fixture {
public:
	rule_remove_fixture()
	{
		bot_.get_rules().add(daemon::rule{
			{ "s1", "s2" },
			{ "c1", "c2" },
			{ "o1", "o2" },
			{ "p1", "p2" },
			{ "onMessage", "onCommand" },
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
	}
};

BOOST_FIXTURE_TEST_SUITE(rule_remove_fixture_suite, rule_remove_fixture)

BOOST_AUTO_TEST_CASE(basic)
{
	request({
		{ "command",    "rule-remove"   },
		{ "index",      1U              }
	});

	stream_->clear();

	const auto json = request({{ "command", "rule-list" }});

	BOOST_TEST(json.size() == 2U);
	BOOST_TEST(json["command"].get<std::string>() == "rule-list");
	BOOST_TEST(json["list"].is_array());
	BOOST_TEST(json["list"].size() == 1U);

	auto servers = json["list"][0]["servers"];
	auto channels = json["list"][0]["channels"];
	auto plugins = json["list"][0]["plugins"];
	auto events = json["list"][0]["events"];

	BOOST_TEST(json_util::contains(servers, "s1"));
	BOOST_TEST(json_util::contains(servers, "s2"));
	BOOST_TEST(json_util::contains(channels, "c1"));
	BOOST_TEST(json_util::contains(channels, "c2"));
	BOOST_TEST(json_util::contains(plugins, "p1"));
	BOOST_TEST(json_util::contains(plugins, "p2"));
	BOOST_TEST(json_util::contains(events, "onMessage"));
	BOOST_TEST(json_util::contains(events, "onCommand"));
	BOOST_TEST(json["list"][0]["action"].get<std::string>() == "drop");
}

BOOST_AUTO_TEST_SUITE(errors)

BOOST_AUTO_TEST_CASE(invalid_index_1)
{
	const auto json = request({
		{ "command",    "rule-remove"   },
		{ "index",      -100            }
	});

	BOOST_TEST(json.size() == 4U);
	BOOST_TEST(json["command"].get<std::string>() == "rule-remove");
	BOOST_TEST(json["error"].get<int>() == daemon::rule_error::invalid_index);
	BOOST_TEST(json["errorCategory"].get<std::string>() == "rule");
}

BOOST_AUTO_TEST_CASE(invalid_index_2)
{
	const auto json = request({
		{ "command",    "rule-remove"   },
		{ "index",      100U            }
	});

	BOOST_TEST(json.size() == 4U);
	BOOST_TEST(json["command"].get<std::string>() == "rule-remove");
	BOOST_TEST(json["error"].get<int>() == daemon::rule_error::invalid_index);
	BOOST_TEST(json["errorCategory"].get<std::string>() == "rule");
}

BOOST_AUTO_TEST_CASE(invalid_index_3)
{
	const auto json = request({
		{ "command",    "rule-remove"   },
		{ "index",      "notaint"       }
	});

	BOOST_TEST(json.size() == 4U);
	BOOST_TEST(json["command"].get<std::string>() == "rule-remove");
	BOOST_TEST(json["error"].get<int>() == daemon::rule_error::invalid_index);
	BOOST_TEST(json["errorCategory"].get<std::string>() == "rule");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
