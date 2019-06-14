/*
 * main.cpp -- test plugin-load remote command
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

#define BOOST_TEST_MODULE "plugin-load"
#include <boost/test/unit_test.hpp>

#include <irccd/test/command_fixture.hpp>

namespace irccd {

namespace {

class plugin_load_fixture : public test::command_fixture {
public:
	plugin_load_fixture()
	{
		bot_.get_plugins().clear();
		bot_.get_plugins().add(std::make_unique<test::mock_plugin>("already"));
	}
};

} // !namespace

BOOST_FIXTURE_TEST_SUITE(plugin_load_fixture_suite, plugin_load_fixture)

BOOST_AUTO_TEST_CASE(basic)
{
	const auto json = request({
		{ "command",    "plugin-load"   },
		{ "plugin",     "mock"          }
	});

	BOOST_TEST(json.size() == 1U);
	BOOST_TEST(json["command"].get<std::string>() == "plugin-load");
	BOOST_TEST(bot_.get_plugins().has("mock"));
}

BOOST_AUTO_TEST_SUITE(errors)

BOOST_AUTO_TEST_CASE(invalid_identifier)
{
	const auto json = request({
		{ "command", "plugin-load" }
	});

	BOOST_TEST(json.size() == 4U);
	BOOST_TEST(json["command"].get<std::string>() == "plugin-load");
	BOOST_TEST(json["error"].get<int>() == daemon::plugin_error::invalid_identifier);
	BOOST_TEST(json["errorCategory"].get<std::string>() == "plugin");
}

BOOST_AUTO_TEST_CASE(not_found)
{
	const auto json = request({
		{ "command",    "plugin-load"   },
		{ "plugin",     "unknown"       }
	});

	BOOST_TEST(json.size() == 4U);
	BOOST_TEST(json["command"].get<std::string>() == "plugin-load");
	BOOST_TEST(json["error"].get<int>() == daemon::plugin_error::not_found);
	BOOST_TEST(json["errorCategory"].get<std::string>() == "plugin");
}

BOOST_AUTO_TEST_CASE(already_exists)
{
	const auto json = request({
		{ "command",    "plugin-load"   },
		{ "plugin",     "already"       }
	});

	BOOST_TEST(json.size() == 4U);
	BOOST_TEST(json["command"].get<std::string>() == "plugin-load");
	BOOST_TEST(json["error"].get<int>() == daemon::plugin_error::already_exists);
	BOOST_TEST(json["errorCategory"].get<std::string>() == "plugin");
}

BOOST_AUTO_TEST_CASE(exec_error)
{
	const auto json = request({
		{ "command",    "plugin-load"   },
		{ "plugin",     "broken"        }
	});

	BOOST_TEST(json.size() == 4U);
	BOOST_TEST(json["command"].get<std::string>() == "plugin-load");
	BOOST_TEST(json["error"].get<int>() == daemon::plugin_error::exec_error);
	BOOST_TEST(json["errorCategory"].get<std::string>() == "plugin");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

} // !irccd
