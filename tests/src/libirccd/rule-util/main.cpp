/*
 * main.cpp -- test rule_util functions
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

#define BOOST_TEST_MODULE "rule_util"
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>

#include <irccd/ini.hpp>

#include <irccd/daemon/rule.hpp>
#include <irccd/daemon/rule_util.hpp>

namespace irccd {

namespace {

auto open(const std::string& config) -> ini::document
{
	boost::filesystem::path path;

	path /= CMAKE_CURRENT_SOURCE_DIR;
	path /= config;

	return ini::read_file(path.string());
}

BOOST_AUTO_TEST_SUITE(from_config)

BOOST_AUTO_TEST_SUITE(valid)

BOOST_AUTO_TEST_CASE(servers)
{
	const auto rule = rule_util::from_config(open("simple.conf")[0]);

	BOOST_TEST(rule.servers.size() == 1U);
	BOOST_TEST(rule.servers.count("s1"));
	BOOST_TEST(rule.channels.empty());
	BOOST_TEST(rule.plugins.empty());
	BOOST_TEST(rule.events.empty());
}

BOOST_AUTO_TEST_CASE(channels)
{
	const auto rule = rule_util::from_config(open("simple.conf")[1]);

	BOOST_TEST(rule.servers.empty());
	BOOST_TEST(rule.channels.size() == 1U);
	BOOST_TEST(rule.channels.count("#c1"));
	BOOST_TEST(rule.plugins.empty());
	BOOST_TEST(rule.events.empty());
}

BOOST_AUTO_TEST_CASE(plugins)
{
	const auto rule = rule_util::from_config(open("simple.conf")[2]);

	BOOST_TEST(rule.servers.empty());
	BOOST_TEST(rule.channels.empty());
	BOOST_TEST(rule.plugins.size() == 1U);
	BOOST_TEST(rule.plugins.count("hangman"));
	BOOST_TEST(rule.events.empty());
}

BOOST_AUTO_TEST_CASE(events)
{
	const auto rule = rule_util::from_config(open("simple.conf")[3]);

	BOOST_TEST(rule.servers.empty());
	BOOST_TEST(rule.channels.empty());
	BOOST_TEST(rule.plugins.empty());
	BOOST_TEST(rule.events.size() == 1U);
	BOOST_TEST(rule.events.count("onCommand"));
}

BOOST_AUTO_TEST_SUITE(errors)

BOOST_AUTO_TEST_CASE(invalid_action)
{
	BOOST_REQUIRE_THROW(rule_util::from_config(open("error-invalid-action.conf")[0]), rule_error);

	try {
		rule_util::from_config(open("error-invalid-action.conf")[0]);
	} catch (const rule_error& ex) {
		BOOST_TEST(ex.code() == rule_error::invalid_action);
	}
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
