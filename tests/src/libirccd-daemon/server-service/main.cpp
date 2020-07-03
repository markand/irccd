/*
 * main.cpp -- test server_service object
 *
 * Copyright (c) 2013-2020 David Demelier <markand@malikania.fr>
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

#define BOOST_TEST_MODULE "server_service"
#include <boost/test/unit_test.hpp>

#include <irccd/daemon/server_service.hpp>

#include <irccd/test/mock_server.hpp>
#include <irccd/test/irccd_fixture.hpp>

namespace test = irccd::test;

namespace irccd::daemon {

namespace {

BOOST_FIXTURE_TEST_SUITE(server_service_test_suite, test::irccd_fixture)

BOOST_AUTO_TEST_CASE(add)
{
	auto sv1 = std::make_shared<test::mock_server>(ctx_, "m1", "local");
	auto sv2 = std::make_shared<test::mock_server>(ctx_, "m2", "local");

	bot_.get_servers().add(sv1);
	bot_.get_servers().add(sv2);

	BOOST_TEST(sv1->find("connect").size() == 1U);
	BOOST_TEST(sv2->find("connect").size() == 1U);
	BOOST_TEST(static_cast<int>(sv1->get_state()) == static_cast<int>(server::state::connected));
	BOOST_TEST(static_cast<int>(sv2->get_state()) == static_cast<int>(server::state::connected));
}

BOOST_AUTO_TEST_CASE(remove)
{
	auto sv1 = std::make_shared<test::mock_server>(ctx_, "m1", "local");
	auto sv2 = std::make_shared<test::mock_server>(ctx_, "m2", "local");

	bot_.get_servers().add(sv1);
	bot_.get_servers().add(sv2);

	sv1->clear();
	sv2->clear();

	bot_.get_servers().remove("m2");

	BOOST_TEST(bot_.get_servers().has("m1"));
	BOOST_TEST(!bot_.get_servers().has("m2"));
	BOOST_TEST(sv2->find("disconnect").size() == 1U);
	BOOST_TEST(static_cast<int>(sv1->get_state()) == static_cast<int>(server::state::connected));
	BOOST_TEST(static_cast<int>(sv2->get_state()) == static_cast<int>(server::state::disconnected));
}

BOOST_AUTO_TEST_CASE(reconnect_one)
{
	auto sv1 = std::make_shared<test::mock_server>(ctx_, "m1", "local");
	auto sv2 = std::make_shared<test::mock_server>(ctx_, "m2", "local");

	bot_.get_servers().add(sv1);
	bot_.get_servers().add(sv2);

	sv1->clear();
	sv2->clear();

	bot_.get_servers().reconnect("m1");

	BOOST_TEST(sv1->find("disconnect").size() == 1U);
	BOOST_TEST(sv2->find("disconnect").size() == 0U);
	BOOST_TEST(sv1->find("connect").size() == 1U);
	BOOST_TEST(sv2->find("connect").size() == 0U);
	BOOST_TEST(static_cast<int>(sv1->get_state()) == static_cast<int>(server::state::connected));
	BOOST_TEST(static_cast<int>(sv2->get_state()) == static_cast<int>(server::state::connected));
}

BOOST_AUTO_TEST_CASE(reconnect_all)
{
	auto sv1 = std::make_shared<test::mock_server>(ctx_, "m1", "local");
	auto sv2 = std::make_shared<test::mock_server>(ctx_, "m2", "local");

	bot_.get_servers().add(sv1);
	bot_.get_servers().add(sv2);

	sv1->clear();
	sv2->clear();

	bot_.get_servers().reconnect();

	BOOST_TEST(sv1->find("disconnect").size() == 1U);
	BOOST_TEST(sv2->find("disconnect").size() == 1U);
	BOOST_TEST(sv1->find("connect").size() == 1U);
	BOOST_TEST(sv2->find("connect").size() == 1U);
	BOOST_TEST(static_cast<int>(sv1->get_state()) == static_cast<int>(server::state::connected));
	BOOST_TEST(static_cast<int>(sv2->get_state()) == static_cast<int>(server::state::connected));
}

BOOST_AUTO_TEST_CASE(disconnect_one)
{
	auto sv1 = std::make_shared<test::mock_server>(ctx_, "m1", "local");
	auto sv2 = std::make_shared<test::mock_server>(ctx_, "m2", "local");

	bot_.get_servers().add(sv1);
	bot_.get_servers().add(sv2);

	sv1->clear();
	sv2->clear();

	bot_.get_servers().disconnect("m1");

	BOOST_TEST(sv1->find("disconnect").size() == 1U);
	BOOST_TEST(sv2->find("disconnect").size() == 0U);
	BOOST_TEST(static_cast<int>(sv1->get_state()) == static_cast<int>(server::state::disconnected));
	BOOST_TEST(static_cast<int>(sv2->get_state()) == static_cast<int>(server::state::connected));
}

BOOST_AUTO_TEST_CASE(disconnect_all)
{
	auto sv1 = std::make_shared<test::mock_server>(ctx_, "m1", "local");
	auto sv2 = std::make_shared<test::mock_server>(ctx_, "m2", "local");

	bot_.get_servers().add(sv1);
	bot_.get_servers().add(sv2);

	sv1->clear();
	sv2->clear();

	bot_.get_servers().disconnect();

	BOOST_TEST(sv1->find("disconnect").size() == 1U);
	BOOST_TEST(sv2->find("disconnect").size() == 1U);
	BOOST_TEST(static_cast<int>(sv1->get_state()) == static_cast<int>(server::state::disconnected));
	BOOST_TEST(static_cast<int>(sv2->get_state()) == static_cast<int>(server::state::disconnected));
}

BOOST_AUTO_TEST_CASE(clear)
{
	auto sv1 = std::make_shared<test::mock_server>(ctx_, "m1", "local");
	auto sv2 = std::make_shared<test::mock_server>(ctx_, "m2", "local");

	bot_.get_servers().add(sv1);
	bot_.get_servers().add(sv2);

	sv1->clear();
	sv2->clear();

	bot_.get_servers().clear();

	BOOST_TEST(bot_.get_servers().list().empty());
	BOOST_TEST(sv1->find("disconnect").size() == 1U);
	BOOST_TEST(sv2->find("disconnect").size() == 1U);
	BOOST_TEST(static_cast<int>(sv1->get_state()) == static_cast<int>(server::state::disconnected));
	BOOST_TEST(static_cast<int>(sv2->get_state()) == static_cast<int>(server::state::disconnected));
}

BOOST_AUTO_TEST_CASE(get)
{
	bot_.get_servers().add(std::make_shared<test::mock_server>(ctx_, "m1", "local"));

	BOOST_TEST(bot_.get_servers().get("m1"));
	BOOST_TEST(!bot_.get_servers().get("none"));
}

BOOST_AUTO_TEST_CASE(require)
{
	bot_.get_servers().add(std::make_shared<test::mock_server>(ctx_, "m1", "local"));

	BOOST_TEST(bot_.get_servers().require("m1"));
	BOOST_REQUIRE_THROW(bot_.get_servers().require("none"), server_error);
}

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd::daemon
