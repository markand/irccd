/*
 * main.cpp -- test irccdctl server-info
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

#define BOOST_TEST_MODULE "irccdctl server-info"
#include <boost/test/unit_test.hpp>

#include <irccd/test/cli_fixture.hpp>

using namespace irccd::test;

namespace irccd {

namespace {

BOOST_FIXTURE_TEST_SUITE(server_info_suite, cli_fixture)

BOOST_AUTO_TEST_CASE(output)
{
	server_->set_username("francis");
	server_->set_nickname("francis");
	start();

	const auto [code, out, err] = exec({ "server-info", "test" });

	BOOST_TEST(!code);
	BOOST_TEST(out.size() == 10U);
	BOOST_TEST(err.size() == 0U);
	BOOST_TEST(out[0] == "Name           : test");
	BOOST_TEST(out[1] == "Hostname       : localhost");
	BOOST_TEST(out[2] == "Port           : 6667");
	BOOST_TEST(out[3] == "Ipv6           : null");
	BOOST_TEST(out[4] == "SSL            : null");
	BOOST_TEST(out[5] == "SSL verified   : null");
	BOOST_TEST(out[6] == "Channels       : ");
	BOOST_TEST(out[7] == "Nickname       : francis");
	BOOST_TEST(out[8] == "User name      : francis");
	BOOST_TEST(out[9] == "Real name      : IRC Client Daemon");
}

BOOST_AUTO_TEST_SUITE(errors)

BOOST_AUTO_TEST_CASE(invalid_identifier)
{
	start();

	const auto [code, out, err] = exec({ "server-info", "+++" });

	BOOST_TEST(code);
	BOOST_TEST(out.size() == 0U);
	BOOST_TEST(err.size() == 1U);
	BOOST_TEST(err[0] == "abort: invalid server identifier");
}

BOOST_AUTO_TEST_CASE(not_found)
{
	start();

	const auto [code, out, err] = exec({ "server-info", "unknown" });

	BOOST_TEST(code);
	BOOST_TEST(out.size() == 0U);
	BOOST_TEST(err.size() == 1U);
	BOOST_TEST(err[0] == "abort: server not found");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd