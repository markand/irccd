/*
 * main.cpp -- test irccdctl rule-edit
 *
 * Copyright (c) 2013-2019 David Demelier <markand@malikania.fr>
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

#define BOOST_TEST_MODULE "irccdctl rule-edit"
#include <boost/test/unit_test.hpp>

#include <irccd/test/cli_fixture.hpp>

using irccd::test::cli_fixture;

using irccd::daemon::rule;

namespace irccd {

namespace {

class rule_edit_fixture : public cli_fixture {
public:
	rule_edit_fixture()
		: cli_fixture(IRCCDCTL_EXECUTABLE)
	{
		bot_.rules().add({
			{ "s1", "s2" },
			{ "c1", "c2" },
			{ "o1", "o2" },
			{ "p1", "p2" },
			{ "onCommand", "onMessage" },
			rule::action_type::drop
		});
	}
};

BOOST_FIXTURE_TEST_SUITE(rule_edit_suite, rule_edit_fixture)

BOOST_AUTO_TEST_CASE(server)
{
	start();

	{
		const auto [code, out, err] = exec({ "rule-edit",
			"-s ts1",   "--add-server ts2",
			"-S s1",    "--remove-server s2",
			"0"
		});

		BOOST_TEST(!code);
		BOOST_TEST(out.size() == 0U);
		BOOST_TEST(err.size() == 0U);
	}

	{
		const auto [code, out, err] = exec({ "rule-list" });

		BOOST_TEST(!code);
		BOOST_TEST(out.size() == 6U);
		BOOST_TEST(err.size() == 0U);
		BOOST_TEST(out[0]  == "rule:           0");
		BOOST_TEST(out[1]  == "servers:        ts1 ts2 ");
		BOOST_TEST(out[2]  == "channels:       c1 c2 ");
		BOOST_TEST(out[3]  == "plugins:        p1 p2 ");
		BOOST_TEST(out[4]  == "events:         onCommand onMessage ");
		BOOST_TEST(out[5]  == "action:         drop");
	}
}

BOOST_AUTO_TEST_CASE(channel)
{
	start();

	{
		const auto [code, out, err] = exec({ "rule-edit",
			"-c tc1",   "--add-channel tc2",
			"-C c1",    "--remove-channel c2",
			"0"
		});

		BOOST_TEST(!code);
		BOOST_TEST(out.size() == 0U);
		BOOST_TEST(err.size() == 0U);
	}

	{
		const auto [code, out, err] = exec({ "rule-list" });

		BOOST_TEST(!code);
		BOOST_TEST(out.size() == 6U);
		BOOST_TEST(err.size() == 0U);
		BOOST_TEST(out[0]  == "rule:           0");
		BOOST_TEST(out[1]  == "servers:        s1 s2 ");
		BOOST_TEST(out[2]  == "channels:       tc1 tc2 ");
		BOOST_TEST(out[3]  == "plugins:        p1 p2 ");
		BOOST_TEST(out[4]  == "events:         onCommand onMessage ");
		BOOST_TEST(out[5]  == "action:         drop");
	}
}

BOOST_AUTO_TEST_CASE(plugin)
{
	start();

	{
		const auto [code, out, err] = exec({ "rule-edit",
			"-p tp1",   "--add-plugin tp2",
			"-P p1",    "--remove-plugin p2",
			"0"
		});

		BOOST_TEST(!code);
		BOOST_TEST(out.size() == 0U);
		BOOST_TEST(err.size() == 0U);
	}

	{
		const auto [code, out, err] = exec({ "rule-list" });

		BOOST_TEST(!code);
		BOOST_TEST(out.size() == 6U);
		BOOST_TEST(err.size() == 0U);
		BOOST_TEST(out[0]  == "rule:           0");
		BOOST_TEST(out[1]  == "servers:        s1 s2 ");
		BOOST_TEST(out[2]  == "channels:       c1 c2 ");
		BOOST_TEST(out[3]  == "plugins:        tp1 tp2 ");
		BOOST_TEST(out[4]  == "events:         onCommand onMessage ");
		BOOST_TEST(out[5]  == "action:         drop");
	}
}

BOOST_AUTO_TEST_CASE(event)
{
	start();

	{
		const auto [code, out, err] = exec({ "rule-edit",
			"-e onKick",    "--add-event onNickname",
			"-E onMessage", "--remove-event onCommand",
			"0"
		});

		BOOST_TEST(!code);
		BOOST_TEST(out.size() == 0U);
		BOOST_TEST(err.size() == 0U);
	}

	{
		const auto [code, out, err] = exec({ "rule-list" });

		BOOST_TEST(!code);
		BOOST_TEST(out.size() == 6U);
		BOOST_TEST(err.size() == 0U);
		BOOST_TEST(out[0]  == "rule:           0");
		BOOST_TEST(out[1]  == "servers:        s1 s2 ");
		BOOST_TEST(out[2]  == "channels:       c1 c2 ");
		BOOST_TEST(out[3]  == "plugins:        p1 p2 ");
		BOOST_TEST(out[4]  == "events:         onKick onNickname ");
		BOOST_TEST(out[5]  == "action:         drop");
	}
}

BOOST_AUTO_TEST_CASE(action_1)
{
	start();

	{
		const auto [code, out, err] = exec({ "rule-edit", "-a accept", "0" });

		BOOST_TEST(!code);
		BOOST_TEST(out.size() == 0U);
		BOOST_TEST(err.size() == 0U);
	}

	{
		const auto [code, out, err] = exec({ "rule-list" });

		BOOST_TEST(!code);
		BOOST_TEST(out.size() == 6U);
		BOOST_TEST(err.size() == 0U);
		BOOST_TEST(out[0]  == "rule:           0");
		BOOST_TEST(out[1]  == "servers:        s1 s2 ");
		BOOST_TEST(out[2]  == "channels:       c1 c2 ");
		BOOST_TEST(out[3]  == "plugins:        p1 p2 ");
		BOOST_TEST(out[4]  == "events:         onCommand onMessage ");
		BOOST_TEST(out[5]  == "action:         accept");
	}
}

BOOST_AUTO_TEST_CASE(action_2)
{
	start();

	{
		const auto [code, out, err] = exec({ "rule-edit", "--action accept", "0" });

		BOOST_TEST(out.size() == 0U);
		BOOST_TEST(err.size() == 0U);
	}

	{
		const auto [code, out, err] = exec({ "rule-list" });

		BOOST_TEST(!code);
		BOOST_TEST(out.size() == 6U);
		BOOST_TEST(err.size() == 0U);
		BOOST_TEST(out[0]  == "rule:           0");
		BOOST_TEST(out[1]  == "servers:        s1 s2 ");
		BOOST_TEST(out[2]  == "channels:       c1 c2 ");
		BOOST_TEST(out[3]  == "plugins:        p1 p2 ");
		BOOST_TEST(out[4]  == "events:         onCommand onMessage ");
		BOOST_TEST(out[5]  == "action:         accept");
	}
}

BOOST_AUTO_TEST_SUITE(errors)

BOOST_AUTO_TEST_CASE(invalid_index_1)
{
	start();

	const auto [code, out, err] = exec({ "rule-edit", "-p p1", "100" });

	BOOST_TEST(code);
	BOOST_TEST(out.size() == 0U);
	BOOST_TEST(err.size() == 1U);
	BOOST_TEST(err[0] == "abort: invalid rule index");
}

BOOST_AUTO_TEST_CASE(invalid_index_2)
{
	start();

	const auto [code, out, err] = exec({ "rule-edit", "-p p1", "notaint" });

	BOOST_TEST(code);
	BOOST_TEST(out.size() == 0U);
	BOOST_TEST(err.size() == 1U);
	BOOST_TEST(err[0] == "abort: invalid rule index");
}

BOOST_AUTO_TEST_CASE(invalid_action)
{
	start();

	const auto [code, out, err] = exec({ "rule-edit", "--action break", "0" });

	BOOST_TEST(code);
	BOOST_TEST(out.size() == 0U);
	BOOST_TEST(err.size() == 1U);
	BOOST_TEST(err[0] == "abort: invalid rule action");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
