/*
 * main.cpp -- test plugin-reload remote command
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

#define BOOST_TEST_MODULE "plugin-reload"
#include <boost/test/unit_test.hpp>

#include <irccd/test/broken_plugin.hpp>
#include <irccd/test/command_fixture.hpp>

namespace irccd {

namespace {

class plugin_reload_fixture : public test::command_fixture {
protected:
	std::shared_ptr<test::mock_plugin> plugin_{new test::mock_plugin("test")};

	plugin_reload_fixture()
	{
		bot_.get_plugins().clear();
		bot_.get_plugins().add(plugin_);
		bot_.get_plugins().add(std::make_unique<test::broken_plugin>("broken"));
	}
};

BOOST_FIXTURE_TEST_SUITE(plugin_reload_fixture_suite, plugin_reload_fixture)

BOOST_AUTO_TEST_CASE(basic)
{
	const auto json = request({
		{ "command",    "plugin-reload" },
		{ "plugin",     "test"          }
	});

	BOOST_TEST(json.size() == 1U);
	BOOST_TEST(json["command"].get<std::string>() == "plugin-reload");
	BOOST_TEST(plugin_->find("handle_reload").size() == 1U);
}

BOOST_AUTO_TEST_SUITE(errors)

BOOST_AUTO_TEST_CASE(invalid_identifier)
{
	const auto json = request({
		{ "command", "plugin-reload" }
	});

	BOOST_TEST(json.size() == 4U);
	BOOST_TEST(json["command"].get<std::string>() == "plugin-reload");
	BOOST_TEST(json["error"].get<int>() == daemon::plugin_error::invalid_identifier);
	BOOST_TEST(json["errorCategory"].get<std::string>() == "plugin");
}

BOOST_AUTO_TEST_CASE(not_found)
{
	const auto json = request({
		{ "command",    "plugin-reload" },
		{ "plugin",     "unknown"       }
	});

	BOOST_TEST(json.size() == 4U);
	BOOST_TEST(json["command"].get<std::string>() == "plugin-reload");
	BOOST_TEST(json["error"].get<int>() == daemon::plugin_error::not_found);
	BOOST_TEST(json["errorCategory"].get<std::string>() == "plugin");
}

BOOST_AUTO_TEST_CASE(exec_error)
{
	const auto json = request({
		{ "command",    "plugin-reload" },
		{ "plugin",     "broken"        }
	});

	BOOST_TEST(json.size() == 4U);
	BOOST_TEST(json["command"].get<std::string>() == "plugin-reload");
	BOOST_TEST(json["error"].get<int>() == daemon::plugin_error::exec_error);
	BOOST_TEST(json["errorCategory"].get<std::string>() == "plugin");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
