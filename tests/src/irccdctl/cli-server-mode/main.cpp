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

#include <irccd/test/cli_fixture.hpp>

using namespace irccd::test;

namespace irccd {

namespace {

BOOST_FIXTURE_TEST_SUITE(server_mode_suite, cli_fixture)

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

BOOST_AUTO_TEST_SUITE(errors)

BOOST_AUTO_TEST_CASE(invalid_identifier_1)
{
	start();

	const auto [code, out, err] = exec({ "server-mode", "+++", "#staff", "+t" });

	BOOST_TEST(code);
	BOOST_TEST(out.size() == 0U);
	BOOST_TEST(err.size() == 1U);
	BOOST_TEST(err[0] == "abort: invalid server identifier");
}

BOOST_AUTO_TEST_CASE(not_found)
{
	start();

	const auto [code, out, err] = exec({ "server-mode", "unknown", "#staff", "+t" });

	BOOST_TEST(code);
	BOOST_TEST(out.size() == 0U);
	BOOST_TEST(err.size() == 1U);
	BOOST_TEST(err[0] == "abort: server not found");
}

BOOST_AUTO_TEST_CASE(invalid_channel)
{
	start();

	const auto [code, out, err] = exec({ "server-mode", "test", "\"\"", "+t" });

	BOOST_TEST(code);
	BOOST_TEST(out.size() == 0U);
	BOOST_TEST(err.size() == 1U);
	BOOST_TEST(err[0] == "abort: invalid or empty channel");
}

BOOST_AUTO_TEST_CASE(invalid_mode)
{
	start();

	const auto [code, out, err] = exec({ "server-mode", "test", "#staff", "\"\"" });

	BOOST_TEST(code);
	BOOST_TEST(out.size() == 0U);
	BOOST_TEST(err.size() == 1U);
	BOOST_TEST(err[0] == "abort: invalid or empty mode");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
