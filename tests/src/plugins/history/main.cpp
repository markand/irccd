/*
 * main.cpp -- test history plugin
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

#include <regex>

#define BOOST_TEST_MODULE "History plugin"
#include <boost/test/unit_test.hpp>

#include <irccd/daemon/bot.hpp>
#include <irccd/daemon/server.hpp>

#include <irccd/test/js_plugin_fixture.hpp>

using irccd::daemon::plugin;

using irccd::test::js_plugin_fixture;

namespace irccd {

namespace {

class history_test : public js_plugin_fixture {
public:
	history_test()
		: js_plugin_fixture(PLUGIN_PATH)
	{
		plugin_->set_formats({
			{ "error", "error=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}" },
			{ "seen", "seen=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}:#{target}:%H:%M" },
			{ "said", "said=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}:#{target}:#{message}:%H:%M" },
			{ "unknown", "unknown=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}:#{target}" },
		});
	}

	void load(plugin::map config = {})
	{
		// Add file if not there.
		if (config.count("file") == 0)
			config.emplace("file", CMAKE_CURRENT_SOURCE_DIR "/words.conf");

		plugin_->set_options(config);
		plugin_->handle_load(bot_);
	}
};

BOOST_FIXTURE_TEST_SUITE(history_test_suite, history_test)

BOOST_AUTO_TEST_CASE(format_error)
{
	load({{"file", CMAKE_CURRENT_SOURCE_DIR "/error.json"}});

	plugin_->handle_command(bot_, { server_, "jean!jean@localhost", "#history", "seen francis" });

	const auto cmd = server_->find("message").front();

	BOOST_TEST(std::any_cast<std::string>(cmd[0]) == "#history");
	BOOST_TEST(std::any_cast<std::string>(cmd[1]) == "error=history:!history:test:#history:jean!jean@localhost:jean");
}

BOOST_AUTO_TEST_CASE(format_seen)
{
	static const std::regex rule("seen=history:!history:test:#history:destructor!dst@localhost:destructor:jean:\\d{2}:\\d{2}");

	remove(CMAKE_CURRENT_BINARY_DIR "/seen.json");
	load({{ "file", CMAKE_CURRENT_BINARY_DIR "/seen.json" }});

	plugin_->handle_message(bot_, { server_, "jean!jean@localhost", "#history", "hello" });
	plugin_->handle_command(bot_, { server_, "destructor!dst@localhost", "#history", "seen jean" });

	auto cmd = server_->find("message").front();

	BOOST_TEST(std::any_cast<std::string>(cmd[0]) == "#history");
	BOOST_TEST(std::regex_match(std::any_cast<std::string>(cmd[1]), rule));
}

BOOST_AUTO_TEST_CASE(format_said)
{
	std::regex rule("said=history:!history:test:#history:destructor!dst@localhost:destructor:jean:hello:\\d{2}:\\d{2}");

	remove(CMAKE_CURRENT_BINARY_DIR "/said.json");
	load({{ "file", CMAKE_CURRENT_BINARY_DIR "/said.json" }});

	plugin_->handle_message(bot_, { server_, "jean!jean@localhost", "#history", "hello" });
	plugin_->handle_command(bot_, { server_, "destructor!dst@localhost", "#history", "said jean" });

	const auto cmd = server_->find("message").front();

	BOOST_TEST(std::any_cast<std::string>(cmd[0]) == "#history");
	BOOST_TEST(std::regex_match(std::any_cast<std::string>(cmd[1]), rule));
}

BOOST_AUTO_TEST_CASE(format_unknown)
{
	remove(CMAKE_CURRENT_BINARY_DIR "/unknown.json");
	load({{ "file", CMAKE_CURRENT_BINARY_DIR "/unknown.json" }});

	plugin_->handle_message(bot_, { server_, "jean!jean@localhost", "#history", "hello" });
	plugin_->handle_command(bot_, { server_, "destructor!dst@localhost", "#history", "seen nobody" });

	const auto cmd = server_->find("message").front();

	BOOST_TEST(std::any_cast<std::string>(cmd[0]) == "#history");
	BOOST_TEST(std::any_cast<std::string>(cmd[1]) == "unknown=history:!history:test:#history:destructor!dst@localhost:destructor:nobody");
}

BOOST_AUTO_TEST_CASE(issue_642)
{
	static const std::regex rule("said=history:!history:test:#history:destructor!dst@localhost:destructor:jean:hello:\\d{2}:\\d{2}");

	remove(CMAKE_CURRENT_BINARY_DIR "/issue-642.json");
	load({{"file", CMAKE_CURRENT_BINARY_DIR "/issue-642.json"}});

	plugin_->handle_message(bot_, { server_, "JeaN!JeaN@localhost", "#history", "hello" });

	// Full caps.
	plugin_->handle_command(bot_, { server_, "destructor!dst@localhost", "#HISTORY", "said JEAN" });

	auto cmd = server_->find("message").front();

	BOOST_TEST(std::any_cast<std::string>(cmd[0]) == "#history");
	BOOST_TEST(std::regex_match(std::any_cast<std::string>(cmd[1]), rule));

	// Random caps.
	plugin_->handle_command(bot_, { server_, "destructor!dst@localhost", "#HiSToRy", "said JeaN" });
	cmd = server_->find("message").back();

	BOOST_TEST(std::any_cast<std::string>(cmd[0]) == "#history");
	BOOST_TEST(std::regex_match(std::any_cast<std::string>(cmd[1]), rule));
}

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
