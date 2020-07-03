/*
 * main.cpp -- test plugin-info remote command
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

#define BOOST_TEST_MODULE "plugin-info"
#include <boost/test/unit_test.hpp>

#include <irccd/test/command_fixture.hpp>

namespace irccd {

namespace {

BOOST_FIXTURE_TEST_SUITE(plugin_info_test_suite, test::command_fixture)

BOOST_AUTO_TEST_CASE(basic)
{
	const auto json = request({
		{ "command",    "plugin-info"   },
		{ "plugin",     "test"          },
	});

	BOOST_TEST(json.size() == 5U);
	BOOST_TEST(json["command"].get<std::string>() == "plugin-info");
	BOOST_TEST(json["author"].get<std::string>() == "David Demelier <markand@malikania.fr>");
	BOOST_TEST(json["license"].get<std::string>() == "ISC");
	BOOST_TEST(json["summary"].get<std::string>() == "mock plugin");
	BOOST_TEST(json["version"].get<std::string>() == "1.0");
}

BOOST_AUTO_TEST_SUITE(errors)

BOOST_AUTO_TEST_CASE(invalid_identifier)
{
	const auto json = request({
		{ "command", "plugin-info" }
	});

	BOOST_TEST(json.size() == 4U);
	BOOST_TEST(json["command"].get<std::string>() == "plugin-info");
	BOOST_TEST(json["error"].get<int>() == daemon::plugin_error::invalid_identifier);
	BOOST_TEST(json["errorCategory"].get<std::string>() == "plugin");
}

BOOST_AUTO_TEST_CASE(not_found)
{
	const auto json = request({
		{ "command",    "plugin-info"   },
		{ "plugin",     "unknown"       }
	});

	BOOST_TEST(json.size() == 4U);
	BOOST_TEST(json["command"].get<std::string>() == "plugin-info");
	BOOST_TEST(json["error"].get<int>() == daemon::plugin_error::not_found);
	BOOST_TEST(json["errorCategory"].get<std::string>() == "plugin");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
