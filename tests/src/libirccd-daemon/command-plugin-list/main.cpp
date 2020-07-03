/*
 * main.cpp -- test plugin-list remote command
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

#define BOOST_TEST_MODULE "plugin-list"
#include <boost/test/unit_test.hpp>

#include <irccd/test/command_fixture.hpp>

namespace irccd {

namespace {

class plugin_list_fixture : public test::command_fixture {
public:
	plugin_list_fixture()
	{
		bot_.get_plugins().clear();
		bot_.get_plugins().add(std::make_unique<test::mock_plugin>("t1"));
		bot_.get_plugins().add(std::make_unique<test::mock_plugin>("t2"));
	}
};

BOOST_FIXTURE_TEST_SUITE(plugin_list_fixture_suite, plugin_list_fixture)

BOOST_AUTO_TEST_CASE(basic)
{
	const auto json = request({
		{ "command", "plugin-list" }
	});

	BOOST_TEST(json.size() == 2U);
	BOOST_TEST(json["command"].get<std::string>() == "plugin-list");
	BOOST_TEST(json["list"][0].get<std::string>() == "t1");
	BOOST_TEST(json["list"][1].get<std::string>() == "t2");
}

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
