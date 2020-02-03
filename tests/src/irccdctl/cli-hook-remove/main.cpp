/*
 * main.cpp -- test irccdctl hook-remove
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

#define BOOST_TEST_MODULE "irccdctl hook-remove"
#include <boost/test/unit_test.hpp>

#include <irccd/test/cli_fixture.hpp>

using namespace irccd::test;

using irccd::daemon::hook;

namespace irccd {

namespace {

class hook_remove_fixture : public cli_fixture {
public:
	hook_remove_fixture()
		: cli_fixture(IRCCDCTL_EXECUTABLE)
	{
		bot_.get_hooks().add(hook("true", "/bin/true"));
		bot_.get_hooks().add(hook("false", "/bin/false"));
	}
};

BOOST_FIXTURE_TEST_SUITE(hook_remove_suite, hook_remove_fixture)

BOOST_AUTO_TEST_CASE(basic)
{
	start();

	const auto [code, out, err] = exec({ "hook-remove", "false" });

	BOOST_TEST(!code);
	BOOST_TEST(out.size() == 0U);
	BOOST_TEST(err.size() == 0U);
	BOOST_TEST(bot_.get_hooks().list().size() == 1U);
	BOOST_TEST(bot_.get_hooks().list()[0].get_id() == "true");
	BOOST_TEST(bot_.get_hooks().list()[0].get_path() == "/bin/true");
}

BOOST_AUTO_TEST_SUITE(errors)

BOOST_AUTO_TEST_CASE(invalid_identifier)
{
	start();

	const auto [code, out, err] = exec({ "hook-remove", "#@#@" });

	BOOST_TEST(code);
	BOOST_TEST(out.size() == 0U);
	BOOST_TEST(err.size() == 1U);
	BOOST_TEST(err[0] == "abort: invalid hook identifier");
}

BOOST_AUTO_TEST_CASE(not_found)
{
	start();

	const auto [code, out, err] = exec({ "hook-remove", "nonexistent" });

	BOOST_TEST(code);
	BOOST_TEST(out.size() == 0U);
	BOOST_TEST(err.size() == 1U);
	BOOST_TEST(err[0] == "abort: hook not found");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
