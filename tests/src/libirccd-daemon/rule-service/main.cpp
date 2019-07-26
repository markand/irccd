/*
 * main.cpp -- test rule_service object
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

#define BOOST_TEST_MODULE "rule_service"
#include <boost/test/unit_test.hpp>

#include <irccd/daemon/logger.hpp>
#include <irccd/daemon/rule_service.hpp>

#include <irccd/test/irccd_fixture.hpp>

namespace test = irccd::test;

BOOST_TEST_DONT_PRINT_LOG_VALUE(irccd::daemon::rule)

namespace irccd::daemon {

namespace {

BOOST_FIXTURE_TEST_SUITE(rule_service_test_suite, test::irccd_fixture)

BOOST_AUTO_TEST_CASE(add)
{
	rule r1{{"s1"}};
	rule r2{{"s2"}};

	bot_.get_rules().add(r1);
	bot_.get_rules().add(r2);

	BOOST_TEST(bot_.get_rules().list().size() == 2U);
	BOOST_TEST(bot_.get_rules().list()[0] == r1);
	BOOST_TEST(bot_.get_rules().list()[1] == r2);
}

BOOST_AUTO_TEST_CASE(insert)
{
	rule r1{{"s1"}};
	rule r2{{"s2"}};

	bot_.get_rules().insert(r1, 0);
	bot_.get_rules().insert(r2, 0);

	BOOST_TEST(bot_.get_rules().list().size() == 2U);
	BOOST_TEST(bot_.get_rules().list()[0] == r2);
	BOOST_TEST(bot_.get_rules().list()[1] == r1);
}

BOOST_AUTO_TEST_CASE(remove)
{
	rule r1{{"s1"}};
	rule r2{{"s2"}};

	bot_.get_rules().add(r1);
	bot_.get_rules().add(r2);
	bot_.get_rules().remove(1);

	BOOST_TEST(bot_.get_rules().list().size() == 1U);
	BOOST_TEST(bot_.get_rules().list()[0] == r1);
}

BOOST_AUTO_TEST_CASE(require)
{
	rule r1{{"s1"}};
	rule r2{{"s2"}};

	bot_.get_rules().add(r1);
	bot_.get_rules().add(r2);

	BOOST_TEST(bot_.get_rules().require(0) == r1);
	BOOST_TEST(bot_.get_rules().require(1) == r2);
	BOOST_REQUIRE_THROW(bot_.get_rules().require(500), rule_error);
}

BOOST_AUTO_TEST_SUITE_END()

/*
 * Simulate the following rules configuration:
 *
 * #
 * # On all servers, each channel #staff can't use the onCommand event,
 * # everything else is allowed.
 * #
 * [rule]       #1
 * servers      = ""
 * channels     = "#staff"
 * events       = "onCommand"
 * action       = drop
 *
 * #
 * # However, the same onCommand on #staff is allowed on server "unsafe"
 * #
 * [rule]       #2
 * servers      = "unsafe"
 * channels     = "#staff"
 * events       = "onCommand"
 * action       = accept
 *
 * #
 * # Plugin game is only allowed on server "malikania" and "localhost",
 * # channel "#games" and events "onMessage, onCommand".
 * #
 * # The first rule #3-1 disable the plugin game for every server, it is
 * # reenabled again with the #3-2.
 * #
 * [rule]       #3-1
 * plugins      = "game"
 * action       = drop
 *
 * [rule]       #3-2
 * servers      = "malikania localhost"
 * channels     = "#games"
 * plugins      = "game"
 * events       = "onMessage onCommand"
 * action       = accept
 */
class solving_test : public test::irccd_fixture {
protected:
	solving_test()
	{
		bot_.set_log(std::make_unique<logger::silent_sink>());

		// #1
		{
			bot_.get_rules().add({
				rule::set{                }, // Servers
				rule::set{ "#staff"       }, // Channels
				rule::set{                }, // Origins
				rule::set{                }, // Plugins
				rule::set{ "onCommand"    }, // Events
				rule::action_type::drop
			});
		}

		// #2
		{
			bot_.get_rules().add({
				rule::set{ "unsafe"       },
				rule::set{ "#staff"       },
				rule::set{                },
				rule::set{                },
				rule::set{ "onCommand"    },
				rule::action_type::accept
			});
		}

		// #3-1
		{
			bot_.get_rules().add({
				rule::set{},
				rule::set{},
				rule::set{},
				rule::set{"game"},
				rule::set{},
				rule::action_type::drop
			});
		}

		// #3-2
		{
			bot_.get_rules().add({
				rule::set{ "malikania", "localhost"   },
				rule::set{ "#games"                   },
				rule::set{                            },
				rule::set{ "game"                     },
				rule::set{ "onCommand", "onMessage"   },
				rule::action_type::accept
			});
		}
	}
};

BOOST_FIXTURE_TEST_SUITE(solving_test_suite, solving_test)

BOOST_AUTO_TEST_CASE(basic_match1)
{
	rule m;

	/*
	 * [rule]
	 */
	BOOST_TEST(m.match("freenode", "#test", "a", "", ""));
	BOOST_TEST(m.match("", "", "", "", ""));
}

BOOST_AUTO_TEST_CASE(basic_match2)
{
	rule m{rule::set{"freenode"}};

	/*
	 * [rule]
	 * servers	= "freenode"
	 */

	BOOST_TEST(m.match("freenode", "#test", "a", "", ""));
	BOOST_TEST(!m.match("malikania", "#test", "a", "", ""));
	BOOST_TEST(m.match("freenode", "", "jean", "", "onMessage"));
}

BOOST_AUTO_TEST_CASE(basic_match3)
{
	rule m{rule::set{"freenode"}, rule::set{"#staff"}};

	/*
	 * [rule]
	 * servers	= "freenode"
	 * channels	= "#staff"
	 */

	BOOST_TEST(m.match("freenode", "#staff", "a", "", ""));
	BOOST_TEST(!m.match("freenode", "#test", "a", "", ""));
	BOOST_TEST(!m.match("malikania", "#staff", "a", "", ""));
}

BOOST_AUTO_TEST_CASE(basic_match4)
{
	rule m{rule::set{"malikania"}, rule::set{"#staff"}, rule::set{"a"}};

	/*
	 * [rule]
	 * servers	= "malikania"
	 * channels	= "#staff"
	 * plugins	= "a"
	 */

	BOOST_TEST(m.match("malikania", "#staff", "a", "",""));
	BOOST_TEST(!m.match("malikania", "#staff", "b", "", ""));
	BOOST_TEST(!m.match("freenode", "#staff", "a", "", ""));
}

BOOST_AUTO_TEST_CASE(complex_match1)
{
	rule m{rule::set{"malikania", "freenode"}};

	/*
	 * [rule]
	 * servers	= "malikania freenode"
	 */

	BOOST_TEST(m.match("malikania", "", "", "", ""));
	BOOST_TEST(m.match("freenode", "", "", "", ""));
	BOOST_TEST(!m.match("no", "", "", "", ""));
}

BOOST_AUTO_TEST_CASE(origin_match)
{
	rule m{
		rule::set{"malikania"},
		rule::set{},
		rule::set{"markand"},
		rule::set{},
		rule::set{},
		rule::action_type::accept
	};

	/*
	 * [rule]
	 * servers = "malikania"
	 * origins = "markand"
	 */
	BOOST_TEST(m.match("malikania", "#staff", "markand", "system", "onCommand"));
	BOOST_TEST(!m.match("malikania", "#staff", "", "system", "onNames"));
	BOOST_TEST(!m.match("malikania", "#staff", "jean", "system", "onMessage"));
}

BOOST_AUTO_TEST_CASE(basic_solve)
{
	/* Allowed */
	BOOST_TEST(bot_.get_rules().solve("malikania", "#staff", "", "a", "onMessage"));

	/* Allowed */
	BOOST_TEST(bot_.get_rules().solve("freenode", "#staff", "", "b", "onTopic"));

	/* Not allowed */
	BOOST_TEST(!bot_.get_rules().solve("malikania", "#staff", "", "", "onCommand"));

	/* Not allowed */
	BOOST_TEST(!bot_.get_rules().solve("freenode", "#staff", "", "c", "onCommand"));

	/* Allowed */
	BOOST_TEST(bot_.get_rules().solve("unsafe", "#staff", "", "c", "onCommand"));
}

BOOST_AUTO_TEST_CASE(games_solve)
{
	/* Allowed */
	BOOST_TEST(bot_.get_rules().solve("malikania", "#games", "", "game", "onMessage"));

	/* Allowed */
	BOOST_TEST(bot_.get_rules().solve("localhost", "#games", "", "game", "onMessage"));

	/* Allowed */
	BOOST_TEST(bot_.get_rules().solve("malikania", "#games", "", "game", "onCommand"));

	/* Not allowed */
	BOOST_TEST(!bot_.get_rules().solve("malikania", "#games", "", "game", "onQuery"));

	/* Not allowed */
	BOOST_TEST(!bot_.get_rules().solve("freenode", "#no", "", "game", "onMessage"));

	/* Not allowed */
	BOOST_TEST(!bot_.get_rules().solve("malikania", "#test", "", "game", "onMessage"));
}

BOOST_AUTO_TEST_CASE(fix_645)
{
	BOOST_TEST(!bot_.get_rules().solve("MALIKANIA", "#STAFF", "", "SYSTEM", "onCommand"));
}

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd::daemon
