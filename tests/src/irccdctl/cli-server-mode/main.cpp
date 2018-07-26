/*
 * main.cpp -- test irccdctl server-mode
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

#define BOOST_TEST_MODULE "irccdctl server-mode"
#include <boost/test/unit_test.hpp>

#include <irccd/test/server_cli_test.hpp>

namespace irccd {

namespace {

BOOST_FIXTURE_TEST_SUITE(server_mode_suite, server_cli_test)

BOOST_AUTO_TEST_CASE(user)
{
    start();

    const auto [code, out, err] = exec({ "server-mode", "test", "irccd", "+i" });

    BOOST_TEST(!code);
    BOOST_TEST(out.size() == 0U);
    BOOST_TEST(err.size() == 0U);

    const auto cmd = server_->find("mode");

    BOOST_TEST(cmd.size() == 1U);
    BOOST_TEST(std::any_cast<std::string>(cmd[0][0]) == "irccd");
    BOOST_TEST(std::any_cast<std::string>(cmd[0][1]) == "+i");
    BOOST_TEST(std::any_cast<std::string>(cmd[0][2]) == "");
    BOOST_TEST(std::any_cast<std::string>(cmd[0][3]) == "");
    BOOST_TEST(std::any_cast<std::string>(cmd[0][4]) == "");
}

BOOST_AUTO_TEST_CASE(channel)
{
    start();

    const auto [code, out, err] = exec({ "server-mode", "test", "#staff", "+b", "francis" });

    BOOST_TEST(!code);
    BOOST_TEST(out.size() == 0U);
    BOOST_TEST(err.size() == 0U);

    const auto cmd = server_->find("mode");

    BOOST_TEST(cmd.size() == 1U);
    BOOST_TEST(std::any_cast<std::string>(cmd[0][0]) == "#staff");
    BOOST_TEST(std::any_cast<std::string>(cmd[0][1]) == "+b");
    BOOST_TEST(std::any_cast<std::string>(cmd[0][2]) == "francis");
    BOOST_TEST(std::any_cast<std::string>(cmd[0][3]) == "");
    BOOST_TEST(std::any_cast<std::string>(cmd[0][4]) == "");
}

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
