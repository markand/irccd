/*
 * main.cpp -- test irccdctl plugin-list
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

#define BOOST_TEST_MODULE "irccdctl plugin-list"
#include <boost/test/unit_test.hpp>

#include <irccd/test/cli_fixture.hpp>
#include <irccd/test/mock_plugin.hpp>

using namespace irccd::test;

namespace irccd {

namespace {

class plugin_list_fixture : public cli_fixture {
public:
	plugin_list_fixture()
		: cli_fixture(IRCCDCTL_EXECUTABLE)
	{
	}
};

BOOST_FIXTURE_TEST_SUITE(plugin_list_suite, plugin_list_fixture)

BOOST_AUTO_TEST_CASE(output)
{
	bot_.get_plugins().add(std::make_unique<mock_plugin>("p1"));
	bot_.get_plugins().add(std::make_unique<mock_plugin>("p2"));
	start();

	const auto [code, out, err] = exec({ "plugin-list" });

	BOOST_TEST(!code);
	BOOST_TEST(out.size() == 2U);
	BOOST_TEST(err.size() == 0U);
	BOOST_TEST(out[0] == "p1");
	BOOST_TEST(out[1] == "p2");
}

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
