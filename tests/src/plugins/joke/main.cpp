/*
 * main.cpp -- test joke plugin
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

#define BOOST_TEST_MODULE "Joke plugin"
#include <boost/test/unit_test.hpp>

#include <irccd/test/js_plugin_fixture.hpp>

using irccd::daemon::plugin;

using irccd::test::js_plugin_fixture;

namespace irccd {

namespace {

class joke_test : public js_plugin_fixture {
public:
	joke_test()
		: js_plugin_fixture(PLUGIN_PATH)
	{
		plugin_->set_formats({
			{ "error", "error=#{server}:#{channel}:#{origin}:#{nickname}" }
		});
	}

	void load(plugin::map config = {})
	{
		// Add file if not there.
		if (config.count("file") == 0)
			config.emplace("file", CMAKE_CURRENT_SOURCE_DIR "/jokes.json");

		plugin_->set_options(config);
		plugin_->handle_load(bot_);
	}
};

BOOST_FIXTURE_TEST_SUITE(joke_test_suite, joke_test)

BOOST_AUTO_TEST_CASE(simple)
{
	/*
	 * Jokes.json have two jokes.
	 *
	 * aaa
	 *
	 * And
	 *
	 * bbbb
	 * bbbb
	 */
	std::unordered_map<std::string, int> said{
		{ "aaa",        0 },
		{ "bbbb",       0 }
	};

	load();

	const auto call = [&] () {
		plugin_->handle_command(bot_, { server_, "jean!jean@localhost", "#joke", "" });

		const auto cmd = server_->find("message").back();
		const auto msg = std::any_cast<std::string>(cmd[1]);

		// "bbbb" is two lines.
		if (msg == "bbbb") {
			const auto first = server_->find("message").front();

			BOOST_TEST(std::any_cast<std::string>(first[0]) == "#joke");
			BOOST_TEST(std::any_cast<std::string>(first[1]) == "bbbb");
		} else
			BOOST_TEST(std::any_cast<std::string>(cmd[1]) == "aaa");

		said[msg] += 1;
		server_->clear();
	};

	call();
	call();

	BOOST_TEST(said.size() == 2U);
	BOOST_TEST(said["aaa"] == 1U);
	BOOST_TEST(said["bbbb"] == 1U);
}

BOOST_AUTO_TEST_CASE(toobig)
{
	// xxx and yyy are both 3-lines which we disallow. only a must be said.
	load({
		{ "file", CMAKE_CURRENT_SOURCE_DIR "/error-toobig.json" },
		{ "max-list-lines", "2" }
	});

	std::unordered_map<std::string, int> said{
		{ "a", 0 }
	};

	const auto call = [&] () {
		plugin_->handle_command(bot_, { server_, "jean!jean@localhost", "#joke", "" });

		const auto cmd = server_->find("message").back();

		BOOST_TEST(std::any_cast<std::string>(cmd[0]) == "#joke");
		BOOST_TEST(std::any_cast<std::string>(cmd[1]) == "a");

		said[std::any_cast<std::string>(cmd[1])] += 1;
		server_->clear();
	};

	call();
	call();
	call();

	BOOST_TEST(said.size() == 1U);
	BOOST_TEST(said["a"] == 3U);
}

BOOST_AUTO_TEST_CASE(invalid)
{
	// Only a is the valid joke in this file.
	load({{ "file", CMAKE_CURRENT_SOURCE_DIR "/error-invalid.json" }});

	std::unordered_map<std::string, int> said{
		{ "a", 0 }
	};

	const auto call = [&] () {
		plugin_->handle_command(bot_, { server_, "jean!jean@localhost", "#joke", "" });

		const auto cmd = server_->find("message").back();

		BOOST_TEST(std::any_cast<std::string>(cmd[0]) == "#joke");
		BOOST_TEST(std::any_cast<std::string>(cmd[1]) == "a");

		server_->clear();
		said[std::any_cast<std::string>(cmd[1])] += 1;
	};

	call();
	call();
	call();

	BOOST_TEST(said.size() == 1U);
	BOOST_TEST(said["a"] == 3U);
}

BOOST_AUTO_TEST_SUITE(errors)

BOOST_AUTO_TEST_CASE(not_found)
{
	load({{"file", "doesnotexist.json"}});

	plugin_->handle_command(bot_, { server_, "jean!jean@localhost", "#joke", "" });

	const auto cmd = server_->find("message").back();

	BOOST_TEST(std::any_cast<std::string>(cmd[0]) == "#joke");
	BOOST_TEST(std::any_cast<std::string>(cmd[1]) == "error=test:#joke:jean!jean@localhost:jean");
}

BOOST_AUTO_TEST_CASE(not_array)
{
	load({{"file", CMAKE_CURRENT_SOURCE_DIR "/error-not-array.json"}});

	plugin_->handle_command(bot_, { server_, "jean!jean@localhost", "#joke", "" });

	const auto cmd = server_->find("message").back();

	BOOST_TEST(std::any_cast<std::string>(cmd[0]) == "#joke");
	BOOST_TEST(std::any_cast<std::string>(cmd[1]) == "error=test:#joke:jean!jean@localhost:jean");
}

BOOST_AUTO_TEST_CASE(empty)
{
	load({{"file", CMAKE_CURRENT_SOURCE_DIR "/error-empty.json"}});

	plugin_->handle_command(bot_, {server_, "jean!jean@localhost", "#joke", ""});

	const auto cmd = server_->find("message").back();

	BOOST_TEST(std::any_cast<std::string>(cmd[0]) == "#joke");
	BOOST_TEST(std::any_cast<std::string>(cmd[1]) == "error=test:#joke:jean!jean@localhost:jean");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
