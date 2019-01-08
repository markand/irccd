/*
 * main.cpp -- test irccdctl server-reconnect
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

#define BOOST_TEST_MODULE "irccdctl server-reconnect"
#include <boost/test/unit_test.hpp>

#include <irccd/test/cli_fixture.hpp>

using namespace irccd::test;

namespace irccd {

namespace {

class server_reconnect_fixture : public cli_fixture {
public:
	server_reconnect_fixture()
		: cli_fixture(IRCCDCTL_EXECUTABLE)
	{
	}
};

BOOST_FIXTURE_TEST_SUITE(server_reconnect_suite, server_reconnect_fixture)

BOOST_AUTO_TEST_CASE(one)
{
	const auto s1 = std::make_shared<mock_server>(bot_.get_service(), "s1", "localhost");
	const auto s2 = std::make_shared<mock_server>(bot_.get_service(), "s2", "localhost");

	bot_.servers().add(s1);
	bot_.servers().add(s2);
	s1->clear();
	s2->clear();
	start();

	const auto [code, out, err] = exec({ "server-reconnect", "test" });

	BOOST_TEST(!code);
	BOOST_TEST(out.size() == 0U);
	BOOST_TEST(err.size() == 0U);
	BOOST_TEST(server_->find("disconnect").size() == 1U);
	BOOST_TEST(server_->find("connect").size() == 1U);
	BOOST_TEST(s1->find("disconnect").size() == 0U);
	BOOST_TEST(s2->find("disconnect").size() == 0U);
	BOOST_TEST(s1->find("connect").size() == 0U);
	BOOST_TEST(s2->find("connect").size() == 0U);
}

BOOST_AUTO_TEST_CASE(all)
{
	const auto s1 = std::make_shared<mock_server>(bot_.get_service(), "s1", "localhost");
	const auto s2 = std::make_shared<mock_server>(bot_.get_service(), "s2", "localhost");

	bot_.servers().add(s1);
	bot_.servers().add(s2);
	s1->clear();
	s2->clear();
	start();

	const auto [code, out, err] = exec({ "server-reconnect" });

	BOOST_TEST(!code);
	BOOST_TEST(out.size() == 0U);
	BOOST_TEST(err.size() == 0U);
	BOOST_TEST(server_->find("disconnect").size() == 1U);
	BOOST_TEST(server_->find("connect").size() == 1U);
	BOOST_TEST(s1->find("disconnect").size() == 1U);
	BOOST_TEST(s2->find("disconnect").size() == 1U);
	BOOST_TEST(s1->find("connect").size() == 1U);
	BOOST_TEST(s2->find("connect").size() == 1U);
}

BOOST_AUTO_TEST_SUITE(errors)

BOOST_AUTO_TEST_CASE(invalid_identifier)
{
	start();

	const auto [code, out, err] = exec({ "server-reconnect", "+++" });

	BOOST_TEST(code);
	BOOST_TEST(out.size() == 0U);
	BOOST_TEST(err.size() == 1U);
	BOOST_TEST(err[0] == "abort: invalid server identifier");
}

BOOST_AUTO_TEST_CASE(not_found)
{
	start();

	const auto [code, out, err] = exec({ "server-reconnect", "unknown" });

	BOOST_TEST(code);
	BOOST_TEST(out.size() == 0U);
	BOOST_TEST(err.size() == 1U);
	BOOST_TEST(err[0] == "abort: server not found");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
