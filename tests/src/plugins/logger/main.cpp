/*
 * main.cpp -- test logger plugin
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

#include <fstream>
#include <iterator>

#define BOOST_TEST_MODULE "Logger plugin"
#include <boost/test/unit_test.hpp>

#include <irccd/daemon/bot.hpp>
#include <irccd/daemon/server.hpp>

#include <irccd/test/js_plugin_fixture.hpp>

using irccd::daemon::plugin;

using irccd::test::js_plugin_fixture;

namespace irccd {

namespace {

class logger_test : public js_plugin_fixture {
protected:
	std::string last() const
	{
		std::ifstream file(CMAKE_CURRENT_BINARY_DIR "/log.txt");

		return std::string(std::istreambuf_iterator<char>(file.rdbuf()), {});
	}

public:
	logger_test()
		: js_plugin_fixture(PLUGIN_PATH)
	{
		remove(CMAKE_CURRENT_BINARY_DIR "/log.txt");

		plugin_->set_templates({
			{ "join", "join=#{server}:#{channel}:#{origin}:#{nickname}" },
			{ "kick", "kick=#{server}:#{channel}:#{origin}:#{nickname}:#{target}:#{reason}" },
			{ "me", "me=#{server}:#{channel}:#{origin}:#{nickname}:#{message}" },
			{ "message", "message=#{server}:#{channel}:#{origin}:#{nickname}:#{message}" },
			{ "mode", "mode=#{server}:#{origin}:#{channel}:#{mode}:#{limit}:#{user}:#{mask}" },
			{ "notice", "notice=#{server}:#{origin}:#{channel}:#{message}" },
			{ "part", "part=#{server}:#{channel}:#{origin}:#{nickname}:#{reason}" },
			{ "query", "query=#{server}:#{origin}:#{nickname}:#{message}" },
			{ "topic", "topic=#{server}:#{channel}:#{origin}:#{nickname}:#{topic}" },
		});
	}

	void load(plugin::map config = {})
	{
		if (config.count("path") == 0)
			config.emplace("path", CMAKE_CURRENT_BINARY_DIR "/log.txt");

		plugin_->set_options(config);
		plugin_->handle_load(bot_);
	}
};

BOOST_FIXTURE_TEST_SUITE(logger_test_suite, logger_test)

BOOST_AUTO_TEST_CASE(template_join)
{
	load();

	plugin_->handle_join(bot_, {server_, "jean!jean@localhost", "#staff"});

	BOOST_REQUIRE_EQUAL("join=test:#staff:jean!jean@localhost:jean\n", last());
}

BOOST_AUTO_TEST_CASE(template_kick)
{
	load();

	plugin_->handle_kick(bot_, {server_, "jean!jean@localhost", "#staff", "badboy", "please do not flood"});

	BOOST_REQUIRE_EQUAL("kick=test:#staff:jean!jean@localhost:jean:badboy:please do not flood\n", last());
}

BOOST_AUTO_TEST_CASE(template_me)
{
	load();

	plugin_->handle_me(bot_, {server_, "jean!jean@localhost", "#staff", "is drinking water"});

	BOOST_REQUIRE_EQUAL("me=test:#staff:jean!jean@localhost:jean:is drinking water\n", last());
}

BOOST_AUTO_TEST_CASE(template_message)
{
	load();

	plugin_->handle_message(bot_, {server_, "jean!jean@localhost", "#staff", "hello guys"});

	BOOST_REQUIRE_EQUAL("message=test:#staff:jean!jean@localhost:jean:hello guys\n", last());
}

BOOST_AUTO_TEST_CASE(template_mode)
{
	load();

	plugin_->handle_mode(bot_, {server_, "jean!jean@localhost", "chris", "+i", "l", "u", "m"});

	BOOST_REQUIRE_EQUAL("mode=test:jean!jean@localhost:chris:+i:l:u:m\n", last());
}

BOOST_AUTO_TEST_CASE(template_notice)
{
	load();

	plugin_->handle_notice(bot_, {server_, "jean!jean@localhost", "chris", "tu veux voir mon chat ?"});

	BOOST_REQUIRE_EQUAL("notice=test:jean!jean@localhost:chris:tu veux voir mon chat ?\n", last());
}

BOOST_AUTO_TEST_CASE(template_part)
{
	load();

	plugin_->handle_part(bot_, {server_, "jean!jean@localhost", "#staff", "too noisy here"});

	BOOST_REQUIRE_EQUAL("part=test:#staff:jean!jean@localhost:jean:too noisy here\n", last());
}

BOOST_AUTO_TEST_CASE(template_topic)
{
	load();

	plugin_->handle_topic(bot_, {server_, "jean!jean@localhost", "#staff", "oh yeah yeaaaaaaaah"});

	BOOST_REQUIRE_EQUAL("topic=test:#staff:jean!jean@localhost:jean:oh yeah yeaaaaaaaah\n", last());
}

BOOST_AUTO_TEST_CASE(fix_642)
{
	load();

	plugin_->handle_message(bot_, {server_, "jean!jean@localhost", "#STAFF", "hello guys"});

	BOOST_REQUIRE_EQUAL("message=test:#staff:jean!jean@localhost:jean:hello guys\n", last());
}

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
