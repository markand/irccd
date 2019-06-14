/*
 * main.cpp -- test irccdctl plugin-unload
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

#define BOOST_TEST_MODULE "irccdctl plugin-unload"
#include <boost/test/unit_test.hpp>

#include <irccd/test/cli_fixture.hpp>
#include <irccd/test/mock_plugin.hpp>

using irccd::daemon::bot;
using irccd::daemon::plugin;

using irccd::test::cli_fixture;
using irccd::test::mock_plugin;

namespace irccd {

namespace {

class plugin_unload_fixture : public cli_fixture {
protected:
	std::shared_ptr<mock_plugin> plugin_{new mock_plugin("test")};

	plugin_unload_fixture()
		: cli_fixture(IRCCDCTL_EXECUTABLE)
	{
		bot_.get_plugins().add(plugin_);
	}
};

BOOST_FIXTURE_TEST_SUITE(plugin_unload_suite, plugin_unload_fixture)

BOOST_AUTO_TEST_CASE(simple)
{
	start();

	const auto [code, out, err] = exec({ "plugin-unload", "test" });

	BOOST_TEST(!code);
	BOOST_TEST(out.size() == 0U);
	BOOST_TEST(err.size() == 0U);
	BOOST_TEST(plugin_->find("handle_unload").size() == 1U);
	BOOST_TEST(!bot_.get_plugins().has("p"));
}

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
