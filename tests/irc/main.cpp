/*
 * main.cpp -- test irc functions
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

#define BOOST_TEST_MODULE "irc"
#include <boost/test/unit_test.hpp>

#include <irccd/irc.hpp>

namespace irccd {

BOOST_AUTO_TEST_SUITE(message_parse)

BOOST_AUTO_TEST_CASE(no_prefix)
{
    irc::message m;

    BOOST_TEST(!m);

    m = irc::message::parse("PRIVMSG jean :bonjour à toi");
    BOOST_TEST(m);
    BOOST_TEST(m.prefix().empty());
    BOOST_TEST(m.command() == "PRIVMSG");
    BOOST_TEST(m.args().size() == 2U);
    BOOST_TEST(m.args()[0] == "jean");
    BOOST_TEST(m.args()[1] == "bonjour à toi");
}

BOOST_AUTO_TEST_CASE(prefix)
{
    irc::message m;

    BOOST_TEST(!m);

    m = irc::message::parse(":127.0.0.1 PRIVMSG jean :bonjour à toi");
    BOOST_TEST(m);
    BOOST_TEST(m.prefix() == "127.0.0.1");
    BOOST_TEST(m.command() == "PRIVMSG");
    BOOST_TEST(m.args().size() == 2U);
    BOOST_TEST(m.args()[0] == "jean");
    BOOST_TEST(m.args()[1] == "bonjour à toi");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(user_parse)

BOOST_AUTO_TEST_CASE(basics)
{
    auto user = irc::user::parse("jean!~jean@127.0.0.1");

    BOOST_TEST(user.nick() == "jean");
    BOOST_TEST(user.host() == "~jean@127.0.0.1");

    auto usersimple = irc::user::parse("jean");

    BOOST_TEST(usersimple.nick() == "jean");
    BOOST_TEST(usersimple.host().empty());
}

BOOST_AUTO_TEST_CASE(empty)
{
    auto user = irc::user::parse("");

    BOOST_TEST(user.nick().empty());
    BOOST_TEST(user.host().empty());
}

BOOST_AUTO_TEST_SUITE_END()

} // !irccd
