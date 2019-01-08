/*
 * main.cpp -- test server-list remote command
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

#define BOOST_TEST_MODULE "server-list"
#include <boost/test/unit_test.hpp>

#include <irccd/test/command_fixture.hpp>

using irccd::test::command_fixture;
using irccd::test::mock_server;

namespace irccd {

namespace {

class server_list_fixture : public command_fixture {
protected:
	server_list_fixture()
	{
		bot_.servers().clear();
		bot_.servers().add(std::make_unique<mock_server>(ctx_, "s1", "localhost"));
		bot_.servers().add(std::make_unique<mock_server>(ctx_, "s2", "localhost"));
	}
};

BOOST_FIXTURE_TEST_SUITE(server_list_fixture_suite, server_list_fixture)

BOOST_AUTO_TEST_CASE(basic)
{
	const auto [json, code] = request({
		{ "command", "server-list" }
	});

	BOOST_TEST(!code);
	BOOST_TEST(json.is_object());
	BOOST_TEST(json["list"].is_array());
	BOOST_TEST(json["list"].size() == 2U);
	BOOST_TEST(json["list"][0].get<std::string>() == "s1");
	BOOST_TEST(json["list"][1].get<std::string>() == "s2");
}

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
