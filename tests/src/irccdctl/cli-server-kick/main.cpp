/*
 * main.cpp -- test irccdctl server-kick
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

#define BOOST_TEST_MODULE "irccdctl server-kick"
#include <boost/test/unit_test.hpp>

#include <irccd/test/cli_fixture.hpp>

using namespace irccd::test;

namespace irccd {

namespace {

BOOST_FIXTURE_TEST_SUITE(server_kick_suite, cli_fixture)

BOOST_AUTO_TEST_CASE(basic)
{
    start();

    const auto [code, out, err] = exec({ "server-kick", "test", "francis", "#staff" });

    BOOST_TEST(!code);
    BOOST_TEST(out.size() == 0U);
    BOOST_TEST(err.size() == 0U);

    const auto cmd = server_->find("kick");

    BOOST_TEST(cmd.size() == 1U);
    BOOST_TEST(std::any_cast<std::string>(cmd[0][0]) == "francis");
    BOOST_TEST(std::any_cast<std::string>(cmd[0][1]) == "#staff");
    BOOST_TEST(std::any_cast<std::string>(cmd[0][2]) == "");
}

BOOST_AUTO_TEST_CASE(with_reason)
{
    start();

    const auto [code, out, err] = exec({ "server-kick", "test", "francis", "#staff", "shhh" });

    BOOST_TEST(!code);
    BOOST_TEST(out.size() == 0U);
    BOOST_TEST(err.size() == 0U);

    const auto cmd = server_->find("kick");

    BOOST_TEST(cmd.size() == 1U);
    BOOST_TEST(std::any_cast<std::string>(cmd[0][0]) == "francis");
    BOOST_TEST(std::any_cast<std::string>(cmd[0][1]) == "#staff");
    BOOST_TEST(std::any_cast<std::string>(cmd[0][2]) == "shhh");
}

BOOST_AUTO_TEST_SUITE(errors)

BOOST_AUTO_TEST_CASE(invalid_identifier_1)
{
    start();

    const auto [code, out, err] = exec({ "server-kick", "+++", "francis", "#staff" });

    BOOST_TEST(code);
    BOOST_TEST(out.size() == 0U);
    BOOST_TEST(err.size() == 1U);
    BOOST_TEST(err[0] == "abort: invalid server identifier");
}

BOOST_AUTO_TEST_CASE(not_found)
{
    start();

    const auto [code, out, err] = exec({ "server-kick", "unknown", "francis", "#staff" });

    BOOST_TEST(code);
    BOOST_TEST(out.size() == 0U);
    BOOST_TEST(err.size() == 1U);
    BOOST_TEST(err[0] == "abort: server not found");
}

BOOST_AUTO_TEST_CASE(invalid_nickname)
{
    start();

    const auto [code, out, err] = exec({ "server-kick", "test", "\"\"", "#staff" });

    BOOST_TEST(code);
    BOOST_TEST(out.size() == 0U);
    BOOST_TEST(err.size() == 1U);
    BOOST_TEST(err[0] == "abort: invalid nickname");
}

BOOST_AUTO_TEST_CASE(invalid_channel)
{
    start();

    const auto [code, out, err] = exec({ "server-kick", "test", "francis", "\"\"" });

    BOOST_TEST(code);
    BOOST_TEST(out.size() == 0U);
    BOOST_TEST(err.size() == 1U);
    BOOST_TEST(err[0] == "abort: invalid or empty channel");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
