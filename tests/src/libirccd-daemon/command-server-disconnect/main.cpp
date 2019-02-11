/*
 * main.cpp -- test server-disconnect remote command
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

#define BOOST_TEST_MODULE "server-disconnect"
#include <boost/test/unit_test.hpp>

#include <irccd/test/command_fixture.hpp>

namespace irccd {

namespace {

class server_disconnect_fixture : public test::command_fixture {
protected:
	std::shared_ptr<test::mock_server> s1_;
	std::shared_ptr<test::mock_server> s2_;

	server_disconnect_fixture()
		: s1_(new test::mock_server(ctx_, "s1", "localhost"))
		, s2_(new test::mock_server(ctx_, "s2", "localhost"))
	{
		bot_.servers().add(s1_);
		bot_.servers().add(s2_);
	}
};

BOOST_FIXTURE_TEST_SUITE(server_disconnect_fixture_suite, server_disconnect_fixture)

BOOST_AUTO_TEST_CASE(one)
{
	const auto json = request({
		{ "command",    "server-disconnect"     },
		{ "server",     "s1"                    }
	});

	BOOST_TEST(json.size() == 1U);
	BOOST_TEST(json["command"].get<std::string>() == "server-disconnect");
	BOOST_TEST(s1_->find("disconnect").size() == 1U);
	BOOST_TEST(!bot_.servers().has("s1"));
	BOOST_TEST(bot_.servers().has("s2"));
}

BOOST_AUTO_TEST_CASE(all)
{
	const auto json = request({{ "command", "server-disconnect" }});

	BOOST_TEST(json.size() == 1U);
	BOOST_TEST(json["command"].get<std::string>() == "server-disconnect");
	BOOST_TEST(s1_->find("disconnect").size() == 1U);
	BOOST_TEST(s2_->find("disconnect").size() == 1U);
	BOOST_TEST(!bot_.servers().has("s1"));
	BOOST_TEST(!bot_.servers().has("s2"));
}

BOOST_AUTO_TEST_SUITE(errors)

BOOST_AUTO_TEST_CASE(invalid_identifier)
{
	const auto json = request({
		{ "command",    "server-disconnect"     },
		{ "server",     123456                  }
	});

	BOOST_TEST(json.size() == 4U);
	BOOST_TEST(json["command"].get<std::string>() == "server-disconnect");
	BOOST_TEST(json["error"].get<int>() == daemon::server_error::invalid_identifier);
	BOOST_TEST(json["errorCategory"].get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(not_found)
{
	const auto json = request({
		{ "command",    "server-disconnect"     },
		{ "server",     "unknown"               }
	});

	BOOST_TEST(json.size() == 4U);
	BOOST_TEST(json["command"].get<std::string>() == "server-disconnect");
	BOOST_TEST(json["error"].get<int>() == daemon::server_error::not_found);
	BOOST_TEST(json["errorCategory"].get<std::string>() == "server");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
