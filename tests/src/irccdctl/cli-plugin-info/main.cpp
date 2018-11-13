/*
 * main.cpp -- test irccdctl plugin-info
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

#define BOOST_TEST_MODULE "irccdctl plugin-info"
#include <boost/test/unit_test.hpp>

#include <irccd/test/cli_fixture.hpp>
#include <irccd/test/mock_plugin.hpp>

using namespace irccd::test;

namespace irccd {

namespace {

class plugin_info_fixture : public cli_fixture {
public:
	plugin_info_fixture()
		: cli_fixture(IRCCDCTL_EXECUTABLE)
	{
	}
};

BOOST_FIXTURE_TEST_SUITE(plugin_info_suite, plugin_info_fixture)

BOOST_AUTO_TEST_CASE(simple)
{
	irccd_.plugins().add(std::make_unique<mock_plugin>("test"));
	start();

	const auto [code, out, err] = exec({ "plugin-info", "test" });

	BOOST_TEST(!code);
	BOOST_TEST(out.size() == 4U);
	BOOST_TEST(err.size() == 0U);
	BOOST_TEST(out[0] == "Author         : David Demelier <markand@malikania.fr>");
	BOOST_TEST(out[1] == "License        : ISC");
	BOOST_TEST(out[2] == "Summary        : mock plugin");
	BOOST_TEST(out[3] == "Version        : 1.0");
}

BOOST_AUTO_TEST_SUITE(errors)

BOOST_AUTO_TEST_CASE(invalid_identifier)
{
	start();

	const auto [code, out, err] = exec({ "plugin-info", "+++" });

	BOOST_TEST(code);
	BOOST_TEST(out.size() == 0U);
	BOOST_TEST(err.size() == 1U);
	BOOST_TEST(err[0] == "abort: invalid plugin identifier");
}

BOOST_AUTO_TEST_CASE(not_found)
{
	start();

	const auto [code, out, err] = exec({ "plugin-info", "unknown" });

	BOOST_TEST(code);
	BOOST_TEST(out.size() == 0U);
	BOOST_TEST(err.size() == 1U);
	BOOST_TEST(err[0] == "abort: plugin not found");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
