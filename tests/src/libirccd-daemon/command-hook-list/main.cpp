/*
 * main.cpp -- test hook-list remote command
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

#define BOOST_TEST_MODULE "hook-list"
#include <boost/test/unit_test.hpp>

#include <irccd/test/command_fixture.hpp>

using std::string;

using irccd::daemon::hook;

namespace irccd {

namespace {

BOOST_FIXTURE_TEST_SUITE(hook_list_fixture_suite, test::command_fixture)

BOOST_AUTO_TEST_CASE(basic)
{
	bot_.get_hooks().add(hook("true", "/bin/true"));
	bot_.get_hooks().add(hook("false", "/bin/false"));

	const auto json = request({{"command", "hook-list"}});

	BOOST_TEST(json.size() == 2U);
	BOOST_TEST(json["command"].get<string>() == "hook-list");
	BOOST_TEST(json["list"].size() == 2U);
	BOOST_TEST(json["list"][0]["id"].get<string>() == "true");
	BOOST_TEST(json["list"][0]["path"].get<string>() == "/bin/true");
	BOOST_TEST(json["list"][1]["id"].get<string>() == "false");
	BOOST_TEST(json["list"][1]["path"].get<string>() == "/bin/false");
}

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
