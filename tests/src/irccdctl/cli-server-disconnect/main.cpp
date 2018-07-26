/*
 * main.cpp -- test irccdctl server-disconnect
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

#define BOOST_TEST_MODULE "irccdctl server-disconnect"
#include <boost/test/unit_test.hpp>

#include <irccd/test/server_cli_test.hpp>

namespace irccd {

namespace {

BOOST_FIXTURE_TEST_SUITE(server_disconnect_suite, server_cli_test)

BOOST_AUTO_TEST_CASE(one)
{
    const auto s1 = std::make_shared<mock_server>(irccd_.get_service(), "s1", "localhost");
    const auto s2 = std::make_shared<mock_server>(irccd_.get_service(), "s2", "localhost");

    irccd_.servers().add(s1);
    irccd_.servers().add(s2);
    s1->clear();
    s2->clear();
    start();

    const auto [code, out, err] = exec({ "server-disconnect", "test" });

    BOOST_TEST(!code);
    BOOST_TEST(out.size() == 0U);
    BOOST_TEST(err.size() == 0U);
    BOOST_TEST(server_->find("disconnect").size() == 1U);
    BOOST_TEST(s1->find("disconnect").size() == 0U);
    BOOST_TEST(s2->find("disconnect").size() == 0U);
}

BOOST_AUTO_TEST_CASE(all)
{
    const auto s1 = std::make_shared<mock_server>(irccd_.get_service(), "s1", "localhost");
    const auto s2 = std::make_shared<mock_server>(irccd_.get_service(), "s2", "localhost");

    irccd_.servers().add(s1);
    irccd_.servers().add(s2);
    s1->clear();
    s2->clear();
    start();

    const auto [code, out, err] = exec({ "server-disconnect" });

    BOOST_TEST(!code);
    BOOST_TEST(out.size() == 0U);
    BOOST_TEST(err.size() == 0U);
    BOOST_TEST(server_->find("disconnect").size() == 1U);
    BOOST_TEST(s1->find("disconnect").size() == 1U);
    BOOST_TEST(s2->find("disconnect").size() == 1U);
}

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
