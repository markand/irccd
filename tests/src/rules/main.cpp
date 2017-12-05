/*
 * main.cpp -- test irccd rules
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

#define BOOST_TEST_MODULE "Rules"
#include <boost/test/unit_test.hpp>

#include <irccd/logger.hpp>
#include <irccd/rule_service.hpp>

namespace irccd {

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
class rules_test {
protected:
    rule_service rules_;

    rules_test()
    {
        log::set_logger(std::make_unique<log::silent_logger>());

        // #1
        {
            rules_.add({
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
            rules_.add({
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
            rules_.add({
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
            rules_.add({
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

BOOST_FIXTURE_TEST_SUITE(rules_test_suite, rules_test)

BOOST_AUTO_TEST_CASE(basic_match1)
{
    rule m;

    /*
     * [rule]
     */
    BOOST_REQUIRE(m.match("freenode", "#test", "a", "", ""));
    BOOST_REQUIRE(m.match("", "", "", "", ""));
}

BOOST_AUTO_TEST_CASE(basic_match2)
{
    rule m(rule::set{"freenode"});

    /*
     * [rule]
     * servers    = "freenode"
     */

    BOOST_REQUIRE(m.match("freenode", "#test", "a", "", ""));
    BOOST_REQUIRE(!m.match("malikania", "#test", "a", "", ""));
    BOOST_REQUIRE(m.match("freenode", "", "jean", "", "onMessage"));
}

BOOST_AUTO_TEST_CASE(basic_match3)
{
    rule m(rule::set{"freenode"}, rule::set{"#staff"});

    /*
     * [rule]
     * servers    = "freenode"
     * channels    = "#staff"
     */

    BOOST_REQUIRE(m.match("freenode", "#staff", "a", "", ""));
    BOOST_REQUIRE(!m.match("freenode", "#test", "a", "", ""));
    BOOST_REQUIRE(!m.match("malikania", "#staff", "a", "", ""));
}

BOOST_AUTO_TEST_CASE(basic_match4)
{
    rule m(rule::set{"malikania"}, rule::set{"#staff"}, rule::set{"a"});

    /*
     * [rule]
     * servers    = "malikania"
     * channels    = "#staff"
     * plugins    = "a"
     */

    BOOST_REQUIRE(m.match("malikania", "#staff", "a", "",""));
    BOOST_REQUIRE(!m.match("malikania", "#staff", "b", "", ""));
    BOOST_REQUIRE(!m.match("freenode", "#staff", "a", "", ""));
}

BOOST_AUTO_TEST_CASE(complex_match1)
{
    rule m(rule::set{"malikania", "freenode"});

    /*
     * [rule]
     * servers    = "malikania freenode"
     */

    BOOST_REQUIRE(m.match("malikania", "", "", "", ""));
    BOOST_REQUIRE(m.match("freenode", "", "", "", ""));
    BOOST_REQUIRE(!m.match("no", "", "", "", ""));
}

BOOST_AUTO_TEST_CASE(basic_solve)
{
    /* Allowed */
    BOOST_REQUIRE(rules_.solve("malikania", "#staff", "", "a", "onMessage"));

    /* Allowed */
    BOOST_REQUIRE(rules_.solve("freenode", "#staff", "", "b", "onTopic"));

    /* Not allowed */
    BOOST_REQUIRE(!rules_.solve("malikania", "#staff", "", "", "onCommand"));

    /* Not allowed */
    BOOST_REQUIRE(!rules_.solve("freenode", "#staff", "", "c", "onCommand"));

    /* Allowed */
    BOOST_REQUIRE(rules_.solve("unsafe", "#staff", "", "c", "onCommand"));
}

BOOST_AUTO_TEST_CASE(games_solve)
{
    /* Allowed */
    BOOST_REQUIRE(rules_.solve("malikania", "#games", "", "game", "onMessage"));

    /* Allowed */
    BOOST_REQUIRE(rules_.solve("localhost", "#games", "", "game", "onMessage"));

    /* Allowed */
    BOOST_REQUIRE(rules_.solve("malikania", "#games", "", "game", "onCommand"));

    /* Not allowed */
    BOOST_REQUIRE(!rules_.solve("malikania", "#games", "", "game", "onQuery"));

    /* Not allowed */
    BOOST_REQUIRE(!rules_.solve("freenode", "#no", "", "game", "onMessage"));

    /* Not allowed */
    BOOST_REQUIRE(!rules_.solve("malikania", "#test", "", "game", "onMessage"));
}

BOOST_AUTO_TEST_CASE(fix_645)
{
    BOOST_REQUIRE(!rules_.solve("MALIKANIA", "#STAFF", "", "SYSTEM", "onCommand"));
}

BOOST_AUTO_TEST_SUITE_END()

} // !irccd