/*
 * main.cpp -- test irccdctl plugin-load
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

#define BOOST_TEST_MODULE "irccdctl plugin-load"
#include <boost/test/unit_test.hpp>

#include <irccd/test/cli_fixture.hpp>
#include <irccd/test/mock_plugin.hpp>

using irccd::daemon::plugin;
using irccd::daemon::plugin_loader;

using irccd::test::mock_plugin;
using irccd::test::cli_fixture;

namespace irccd {

namespace {

class custom_plugin_loader : public plugin_loader {
public:
	custom_plugin_loader()
		: plugin_loader({}, { "none" })
	{
	}

	auto find(std::string_view id) -> std::shared_ptr<plugin> override
	{
		return std::make_unique<mock_plugin>(std::string(id));
	}

	auto open(std::string_view id, std::string_view) -> std::shared_ptr<plugin> override
	{
		return std::make_unique<mock_plugin>(std::string(id));
	}
};

class plugin_list_fixture : public cli_fixture {
public:
	plugin_list_fixture()
		: cli_fixture(IRCCDCTL_EXECUTABLE)
	{
	}
};

} // !namespace

BOOST_FIXTURE_TEST_SUITE(plugin_load_suite, plugin_list_fixture)

BOOST_AUTO_TEST_CASE(simple)
{
	bot_.plugins().add(std::make_unique<mock_plugin>("p1"));
	bot_.plugins().add(std::make_unique<mock_plugin>("p2"));
	bot_.plugins().add_loader(std::make_unique<custom_plugin_loader>());
	start();

	// Load a plugin first.
	{
		const auto [code, out, err] = exec({ "plugin-load", "test" });

		BOOST_TEST(!code);
		BOOST_TEST(out.size() == 0U);
		BOOST_TEST(err.size() == 0U);
	}

	// Get the new list of plugins.
	{
		const auto [code, out, err] = exec({ "plugin-list" });

		BOOST_TEST(!code);
		BOOST_TEST(out.size() == 3U);
		BOOST_TEST(err.size() == 0U);
		BOOST_TEST(out[0] == "p1");
		BOOST_TEST(out[1] == "p2");
		BOOST_TEST(out[2] == "test");
	}
}

BOOST_AUTO_TEST_SUITE_END()

} // !irccd
