/*
 * main.cpp -- test server-info remote command
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

#define BOOST_TEST_MODULE "server-info"
#include <boost/test/unit_test.hpp>

#include <irccd/test/command_fixture.hpp>

namespace irccd {

namespace {

BOOST_FIXTURE_TEST_SUITE(server_info_fixture_suite, test::command_fixture)

BOOST_AUTO_TEST_CASE(basic)
{
	auto server = std::make_unique<test::mock_server>(ctx_, "test", "example.org");

	server->set_port(8765);
	server->set_password("none");
	server->set_nickname("pascal");
	server->set_username("psc");
	server->set_realname("Pascal le grand frere");
	server->set_ctcp_version("yeah");
	server->set_command_char("@");
	server->set_ping_timeout(20000);

	bot_.get_servers().clear();
	bot_.get_servers().add(std::move(server));

	const auto json = request({
		{ "command",    "server-info"   },
		{ "server",     "test"          },
	});

	BOOST_TEST(json.size() == 11U);
	BOOST_TEST(json["command"].get<std::string>() == "server-info");
	BOOST_TEST(json["hostname"].get<std::string>() == "example.org");
	BOOST_TEST(json["name"].get<std::string>() == "test");
	BOOST_TEST(json["nickname"].get<std::string>() == "pascal");
	BOOST_TEST(json["port"].get<int>() == 8765);
	BOOST_TEST(json["realname"].get<std::string>() == "Pascal le grand frere");
	BOOST_TEST(json["username"].get<std::string>() == "psc");
}

BOOST_AUTO_TEST_SUITE(errors)

BOOST_AUTO_TEST_CASE(invalid_identifier_1)
{
	const auto json = request({
		{ "command",    "server-info"   },
		{ "server",     123456          }
	});

	BOOST_TEST(json.size() == 4U);
	BOOST_TEST(json["command"].get<std::string>() == "server-info");
	BOOST_TEST(json["error"].get<int>() == daemon::server_error::invalid_identifier);
	BOOST_TEST(json["errorCategory"].get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(invalid_identifier_2)
{
	const auto json = request({
		{ "command",    "server-info"   },
		{ "server",     ""              }
	});

	BOOST_TEST(json.size() == 4U);
	BOOST_TEST(json["command"].get<std::string>() == "server-info");
	BOOST_TEST(json["error"].get<int>() == daemon::server_error::invalid_identifier);
	BOOST_TEST(json["errorCategory"].get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(not_found)
{
	const auto json = request({
		{ "command",    "server-info"   },
		{ "server",     "unknown"       }
	});

	BOOST_TEST(json.size() == 4U);
	BOOST_TEST(json["command"].get<std::string>() == "server-info");
	BOOST_TEST(json["error"].get<int>() == daemon::server_error::not_found);
	BOOST_TEST(json["errorCategory"].get<std::string>() == "server");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
