/*
 * main.cpp -- test tictactoe plugin
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

#define BOOST_TEST_MODULE "Plugin tictactoe"
#include <boost/test/unit_test.hpp>

#include <irccd/string_util.hpp>

#include <irccd/daemon/bot.hpp>
#include <irccd/daemon/plugin_service.hpp>
#include <irccd/daemon/server.hpp>

#include <irccd/test/js_plugin_fixture.hpp>

using irccd::test::js_plugin_fixture;

namespace irccd {

namespace {

class test_fixture : public js_plugin_fixture {
public:
	test_fixture()
		: js_plugin_fixture(PLUGIN_PATH)
	{
		plugin_->set_templates({
			{ "draw", "draw=#{channel}:#{command}:#{nickname}:#{plugin}:#{server}" },
			{ "invalid", "invalid=#{channel}:#{command}:#{nickname}:#{origin}:#{plugin}:#{server}" },
			{ "running", "running=#{channel}:#{command}:#{nickname}:#{origin}:#{plugin}:#{server}" },
			{ "turn", "turn=#{channel}:#{command}:#{nickname}:#{plugin}:#{server}" },
			{ "used", "used=#{channel}:#{command}:#{nickname}:#{origin}:#{plugin}:#{server}" },
			{ "win", "win=#{channel}:#{command}:#{nickname}:#{plugin}:#{server}" }
		});
	}

	auto next_players() const -> std::pair<std::string, std::string>
	{
		const auto functions = server_->find("message");

		if (functions.size() == 0U)
			throw std::runtime_error("no message");

		const auto cmd = functions.back();
		const auto list = string_util::split(std::any_cast<std::string>(cmd[1]), ":");

		BOOST_TEST(list.size() == 5U);
		BOOST_TEST(list[0] == "turn=#tictactoe");
		BOOST_TEST(list[1] == "!tictactoe");
		BOOST_TEST(list[3] == "tictactoe");
		BOOST_TEST(list[4] == "test");

		return list[2] == "a" ? std::make_pair("a", "b") : std::make_pair("b", "a");
	}

	auto start()
	{
		plugin_->handle_command(bot_, { server_, "a!a@localhost", "#tictactoe", "b" });
		plugin_->handle_names(bot_, { server_, "#tictactoe", { "a", "b" }});

		return next_players();
	}

	/**
	 * Helper to place several tokens on the board and automatically toggling
	 * players.
	 *
	 * This will start the game from "a" with target opponent "b".
	 *
	 */
	void run(const std::initializer_list<std::string>& points)
	{
		auto players = start();

		for (const auto& p : points) {
			server_->clear();
			plugin_->handle_message(bot_, { server_, players.first, "#tictactoe", p });
			players = next_players();
		}
	}
};

BOOST_FIXTURE_TEST_SUITE(test_fixture_suite, test_fixture)

BOOST_AUTO_TEST_CASE(win)
{
	run({ "a 1", "b1", "a 2", "b2" });

	const auto players = next_players();

	plugin_->handle_message(bot_, { server_, players.first, "#tictactoe", "a 3" });

	const auto cmd = server_->find("message").back();

	BOOST_TEST(std::any_cast<std::string>(cmd[0]) == "#tictactoe");

	const auto parts = string_util::split(std::any_cast<std::string>(cmd[1]), ":");

	BOOST_TEST(parts.size() == 5U);
	BOOST_TEST(parts[0] == "win=#tictactoe");
	BOOST_TEST(parts[1] == "!tictactoe");
	BOOST_TEST(parts[2] == players.first);
	BOOST_TEST(parts[3] == "tictactoe");
	BOOST_TEST(parts[4] == "test");
}

BOOST_AUTO_TEST_CASE(draw)
{
	/*
	 *   a b c
	 * 1 o x o
	 * 2 o x x
	 * 3 x o x
	 */
	run({ "b 2", "c 1", "c 3", "b 3", "c 2", "a 2", "a 3", "a 1" });

	const auto players = next_players();

	plugin_->handle_message(bot_, { server_, players.first, "#tictactoe", "b 1" });

	const auto cmd = server_->find("message").back();

	BOOST_TEST(std::any_cast<std::string>(cmd[0]) == "#tictactoe");

	const auto parts = string_util::split(std::any_cast<std::string>(cmd[1]), ":");

	BOOST_TEST(parts.size() == 5U);
	BOOST_TEST(parts[0] == "draw=#tictactoe");
	BOOST_TEST(parts[1] == "!tictactoe");
	BOOST_TEST(parts[2] == players.first);
	BOOST_TEST(parts[3] == "tictactoe");
	BOOST_TEST(parts[4] == "test");
}

BOOST_AUTO_TEST_CASE(used)
{
	auto players = start();

	plugin_->handle_message(bot_, { server_, players.first, "#tictactoe", "a 1" });
	plugin_->handle_message(bot_, { server_, players.second, "#tictactoe", "a 1" });

	const auto cmd = server_->find("message").back();

	BOOST_TEST(std::any_cast<std::string>(cmd[0]) == "#tictactoe");

	const auto parts = string_util::split(std::any_cast<std::string>(cmd[1]), ":");

	BOOST_TEST(parts[0] == "used=#tictactoe");
	BOOST_TEST(parts[1] == "!tictactoe");
	BOOST_TEST(parts[2] == players.second);
	BOOST_TEST(parts[3] == players.second);
	BOOST_TEST(parts[4] == "tictactoe");
	BOOST_TEST(parts[5] == "test");
}

BOOST_AUTO_TEST_CASE(invalid)
{
	// empty name (no names)
	plugin_->handle_command(bot_, { server_, "jean", "#tictactoe", "" });

	auto cmd = server_->find("message").back();

	BOOST_TEST(std::any_cast<std::string>(cmd[0]) == "#tictactoe");
	BOOST_TEST(std::any_cast<std::string>(cmd[1]) == "invalid=#tictactoe:!tictactoe:jean:jean:tictactoe:test");

	// bot name (no names)
	plugin_->handle_command(bot_, { server_, "jean", "#tictactoe", "irccd" });
	cmd = server_->find("message").back();

	BOOST_TEST(std::any_cast<std::string>(cmd[0]) == "#tictactoe");
	BOOST_TEST(std::any_cast<std::string>(cmd[1]) == "invalid=#tictactoe:!tictactoe:jean:jean:tictactoe:test");

	// target is origin (no names)
	plugin_->handle_command(bot_, { server_, server_->get_nickname(), "#tictactoe", server_->get_nickname() });
	cmd = server_->find("message").back();

	BOOST_TEST(std::any_cast<std::string>(cmd[0]) == "#tictactoe");
	BOOST_TEST(std::any_cast<std::string>(cmd[1]) == "invalid=#tictactoe:!tictactoe:irccd:irccd:tictactoe:test");

	// not existing (names)
	plugin_->handle_command(bot_, { server_, server_->get_nickname(), "#tictactoe", server_->get_nickname() });
	plugin_->handle_names(bot_, { server_, "#tictactoe", { "a", "b", "c" }});
	cmd = server_->find("message").back();

	BOOST_TEST(std::any_cast<std::string>(cmd[0]) == "#tictactoe");
	BOOST_TEST(std::any_cast<std::string>(cmd[1]) == "invalid=#tictactoe:!tictactoe:irccd:irccd:tictactoe:test");
}

BOOST_AUTO_TEST_CASE(random)
{
	/*
	 * Ensure that the first player is not always the originator, start the game
	 * for at most 1'000'000 times to avoid forever loop.
	 */
	unsigned count = 0;
	bool a = false;
	bool b = false;

	// Last player turn is the winner.
	while (!a && !b && count++ < 1000000U) {
		run({ "a 1", "b 1", "a 2", "b 2" });

		const auto players = next_players();

		if (players.first == "a")
			a = true;
		else
			b = true;

		plugin_->handle_message(bot_, { server_, players.first, "#tictactoe", "a 3" });
	}
}

BOOST_AUTO_TEST_CASE(disconnect)
{
	const auto players = start();

	plugin_->handle_disconnect(bot_, { server_ });
	server_->clear();
	plugin_->handle_message(bot_, { server_, players.first, "#tictactoe", "a 1" });

	BOOST_TEST(server_->empty());
}

BOOST_AUTO_TEST_CASE(kick)
{
	const auto players = start();

	server_->clear();
	plugin_->handle_kick(bot_, { server_, "kefka", "#tictactoe", players.first, "" });
	plugin_->handle_message(bot_, { server_, players.first, "#tictactoe", "a 1" });

	BOOST_TEST(server_->empty());
}

BOOST_AUTO_TEST_CASE(part)
{
	const auto players = start();

	server_->clear();
	plugin_->handle_part(bot_, { server_, players.first, "#tictactoe", "" });
	plugin_->handle_message(bot_, { server_, players.first, "#tictactoe", "a 1" });

	BOOST_TEST(server_->empty());
}

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
