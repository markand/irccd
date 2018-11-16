/*
 * main.cpp -- test irccdctl plugin-config
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

#define BOOST_TEST_MODULE "irccdctl plugin-config"
#include <boost/test/unit_test.hpp>

#include <irccd/test/cli_fixture.hpp>
#include <irccd/test/mock_plugin.hpp>

using namespace irccd::test;

namespace irccd {

namespace {

class plugin_config_fixture : public cli_fixture {
public:
	plugin_config_fixture()
		: cli_fixture(IRCCDCTL_EXECUTABLE)
	{
		auto conf1 = std::make_unique<mock_plugin>("conf1");
		auto conf2 = std::make_unique<mock_plugin>("conf2");

		conf1->set_options({
			{ "v1", "123" },
			{ "v2", "456" }
		});

		bot_.plugins().add(std::move(conf1));
		bot_.plugins().add(std::move(conf2));
	}
};

BOOST_FIXTURE_TEST_SUITE(plugin_config_suite, plugin_config_fixture)

BOOST_AUTO_TEST_CASE(set_and_get)
{
	start();

	// First, configure. No output yet
	{
		const auto [code, out, err] = exec({ "plugin-config", "conf2", "verbose", "false" });

		// no output yet.
		BOOST_TEST(!code);
		BOOST_TEST(out.size() == 0U);
		BOOST_TEST(err.size() == 0U);
	}

	// Get the newly created value.
	{
		const auto [code, out, err] = exec({ "plugin-config", "conf2", "verbose" });

		BOOST_TEST(!code);
		BOOST_TEST(out.size() == 1U);
		BOOST_TEST(err.size() == 0U);
		BOOST_TEST(out[0] == "false");
	}
}

BOOST_AUTO_TEST_CASE(getall)
{
	start();

	const auto [code, out, err] = exec({ "plugin-config", "conf1" });

	BOOST_TEST(!code);
	BOOST_TEST(out.size() == 2U);
	BOOST_TEST(err.size() == 0U);
	BOOST_TEST(out[0] == "v1               : 123");
	BOOST_TEST(out[1] == "v2               : 456");
}

BOOST_AUTO_TEST_SUITE(errors)

BOOST_AUTO_TEST_CASE(invalid_identifier)
{
	start();

	const auto [code, out, err] = exec({ "plugin-config", "+++" });

	BOOST_TEST(code);
	BOOST_TEST(out.size() == 0U);
	BOOST_TEST(err.size() == 1U);
	BOOST_TEST(err[0] == "abort: invalid plugin identifier");
}

BOOST_AUTO_TEST_CASE(not_found)
{
	start();

	const auto [code, out, err] = exec({ "plugin-config", "unknown" });

	BOOST_TEST(code);
	BOOST_TEST(out.size() == 0U);
	BOOST_TEST(err.size() == 1U);
	BOOST_TEST(err[0] == "abort: plugin not found");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
