/*
 * main.cpp -- test hook-remove remote command
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

#define BOOST_TEST_MODULE "hook-remove"
#include <boost/test/unit_test.hpp>

#include <irccd/test/command_fixture.hpp>

using std::string;

using irccd::daemon::hook;
using irccd::daemon::hook_error;

namespace irccd {

namespace {

BOOST_FIXTURE_TEST_SUITE(hook_remove_fixture_suite, test::command_fixture)

BOOST_AUTO_TEST_CASE(basic)
{
	bot_.get_hooks().add(hook("true", "/bin/true"));
	bot_.get_hooks().add(hook("false", "/bin/false"));

	const auto json = request({
		{ "command",    "hook-remove"   },
		{ "id",         "false"         },
	});

	BOOST_TEST(json.size() == 1U);
	BOOST_TEST(json["command"].get<string>() == "hook-remove");
	BOOST_TEST(bot_.get_hooks().list().size() == 1U);
	BOOST_TEST(bot_.get_hooks().list()[0].get_id() == "true");
	BOOST_TEST(bot_.get_hooks().list()[0].get_path() == "/bin/true");
}

BOOST_AUTO_TEST_SUITE(errors)

BOOST_AUTO_TEST_CASE(invalid_identifier)
{
	const auto json = request({
		{ "command",    "hook-remove"   },
		{ "action",     "#@#@"          }
	});

	BOOST_TEST(json.size() == 4U);
	BOOST_TEST(json["command"].get<string>() == "hook-remove");
	BOOST_TEST(json["error"].get<int>() == hook_error::invalid_identifier);
	BOOST_TEST(json["errorCategory"].get<string>() == "hook");
}

BOOST_AUTO_TEST_CASE(not_found)
{
	request({
		{ "command",    "hook-add"      },
		{ "id",         "true"          },
		{ "path",       "/bin/true"     }
	});

	stream_->clear();

	const auto json = request({
		{ "command",    "hook-remove"   },
		{ "id",         "nonexistent"   },
	});

	BOOST_TEST(json.size() == 4U);
	BOOST_TEST(json["command"].get<string>() == "hook-remove");
	BOOST_TEST(json["error"].get<int>() == hook_error::not_found);
	BOOST_TEST(json["errorCategory"].get<string>() == "hook");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
