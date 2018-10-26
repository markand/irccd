/*
 * main.cpp -- test rule-list remote command
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

#define BOOST_TEST_MODULE "rule-list"
#include <boost/test/unit_test.hpp>

#include <irccd/json_util.hpp>

#include <irccd/test/command_fixture.hpp>

using namespace irccd::test;

namespace irccd {

namespace {

class rule_list_fixture : public command_fixture {
public:
	rule_list_fixture()
	{
		irccd_.rules().add(rule{
			{ "s1", "s2" },
			{ "c1", "c2" },
			{ "o1", "o2" },
			{ "p1", "p2" },
			{ "onMessage", "onCommand" },
			rule::action_type::drop
		});
		irccd_.rules().add(rule{
			{ "s1", },
			{ "c1", },
			{ "o1", },
			{ "p1", },
			{ "onMessage", },
			rule::action_type::accept
		});
	}
};

BOOST_FIXTURE_TEST_SUITE(rule_list_fixture_suite, rule_list_fixture)

BOOST_AUTO_TEST_CASE(basic)
{
	const auto [json, code] = request({{ "command", "rule-list" }});

	BOOST_TEST(!code);
	BOOST_TEST(json.is_object());
	BOOST_TEST(json["list"].is_array());
	BOOST_TEST(json["list"].size() == 2U);

	// Rule 0.
	{
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
}

BOOST_AUTO_TEST_CASE(empty)
{
	irccd_.rules().remove(0);
	irccd_.rules().remove(0);

	const auto [json, code] = request({{ "command", "rule-list" }});

	BOOST_TEST(!code);
	BOOST_TEST(json.is_object());
	BOOST_TEST(json["list"].is_array());
	BOOST_TEST(json["list"].empty());
}

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
