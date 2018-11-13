/*
 * main.cpp -- test irccdctl rule-info
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

#define BOOST_TEST_MODULE "irccdctl rule-info"
#include <boost/test/unit_test.hpp>

#include <irccd/test/cli_fixture.hpp>

using namespace irccd::test;

namespace irccd {

namespace {

class rule_info_fixture : public cli_fixture {
public:
	rule_info_fixture()
		: cli_fixture(IRCCDCTL_EXECUTABLE)
	{
	}
};

BOOST_FIXTURE_TEST_SUITE(rule_info_suite, rule_info_fixture)

BOOST_AUTO_TEST_CASE(info)
{
	irccd_.rules().add({
		{ "s1", "s2" },
		{ "c1", "c2" },
		{ "o1", "o2" },
		{ "p1", "p2" },
		{ "onCommand", "onMessage" },
		rule::action_type::drop
	});
	start();

	const auto [code, out, err] = exec({ "rule-info", "0" });

	BOOST_TEST(!code);
	BOOST_TEST(out.size() == 7U);
	BOOST_TEST(err.size() == 0U);
	BOOST_TEST(out[0]  == "rule:        0");
	BOOST_TEST(out[1]  == "servers:     s1 s2 ");
	BOOST_TEST(out[2]  == "channels:    c1 c2 ");
	BOOST_TEST(out[3]  == "plugins:     p1 p2 ");
	BOOST_TEST(out[4]  == "events:      onCommand onMessage ");
	BOOST_TEST(out[5]  == "action:      drop");
}

BOOST_AUTO_TEST_SUITE(errors)

BOOST_AUTO_TEST_CASE(invalid_index_1)
{
	start();

	const auto [code, out, err] = exec({ "rule-info", "100" });

	BOOST_TEST(code);
	BOOST_TEST(out.size() == 0U);
	BOOST_TEST(err.size() == 1U);
	BOOST_TEST(err[0] == "abort: invalid rule index");
}

BOOST_AUTO_TEST_CASE(invalid_index_2)
{
	start();

	const auto [code, out, err] = exec({ "rule-info", "notaint" });

	BOOST_TEST(code);
	BOOST_TEST(out.size() == 0U);
	BOOST_TEST(err.size() == 1U);
	BOOST_TEST(err[0] == "abort: invalid rule index");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
