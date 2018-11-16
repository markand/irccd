/*
 * main.cpp -- test dynlib_plugin
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

#define BOOST_TEST_MODULE "dynlib_plugin"
#include <boost/test/unit_test.hpp>

/*
 * For this test, we update internal plugin configuration each time a function
 * is called and check if it has been called correctly using get_options.
 */

#include <irccd/daemon/bot.hpp>
#include <irccd/daemon/dynlib_plugin.hpp>
#include <irccd/daemon/server.hpp>

using irccd::daemon::bot;
using irccd::daemon::dynlib_plugin_loader;
using irccd::daemon::plugin;

namespace irccd {

namespace {

class fixture {
protected:
	boost::asio::io_service service_;
	std::shared_ptr<plugin> plugin_;
	bot bot_{service_};

	fixture()
	{
		plugin_ = dynlib_plugin_loader({CMAKE_CURRENT_BINARY_DIR}).find("test-plugin");

		if (!plugin_)
			throw std::runtime_error("test plugin not found");
	}
};

BOOST_FIXTURE_TEST_SUITE(dynlib_plugin_suite, fixture)

BOOST_AUTO_TEST_CASE(handle_command)
{
	plugin_->handle_command(bot_, {});

	BOOST_TEST(plugin_->get_options().size() == 1U);
	BOOST_TEST(plugin_->get_options()["command"] == "true");
}

BOOST_AUTO_TEST_CASE(handle_connect)
{
	plugin_->handle_connect(bot_, {});

	BOOST_TEST(plugin_->get_options().size() == 1U);
	BOOST_TEST(plugin_->get_options()["connect"] == "true");
}

BOOST_AUTO_TEST_CASE(handle_invite)
{
	plugin_->handle_invite(bot_, {});

	BOOST_TEST(plugin_->get_options().size() == 1U);
	BOOST_TEST(plugin_->get_options()["invite"] == "true");
}

BOOST_AUTO_TEST_CASE(handle_join)
{
	plugin_->handle_join(bot_, {});

	BOOST_TEST(plugin_->get_options().size() == 1U);
	BOOST_TEST(plugin_->get_options()["join"] == "true");
}

BOOST_AUTO_TEST_CASE(handle_kick)
{
	plugin_->handle_kick(bot_, {});

	BOOST_TEST(plugin_->get_options().size() == 1U);
	BOOST_TEST(plugin_->get_options()["kick"] == "true");
}

BOOST_AUTO_TEST_CASE(handle_load)
{
	plugin_->handle_load(bot_);

	BOOST_TEST(plugin_->get_options().size() == 1U);
	BOOST_TEST(plugin_->get_options()["load"] == "true");
}

BOOST_AUTO_TEST_CASE(handle_message)
{
	plugin_->handle_message(bot_, {});

	BOOST_TEST(plugin_->get_options().size() == 1U);
	BOOST_TEST(plugin_->get_options()["message"] == "true");
}

BOOST_AUTO_TEST_CASE(handle_me)
{
	plugin_->handle_me(bot_, {});

	BOOST_TEST(plugin_->get_options().size() == 1U);
	BOOST_TEST(plugin_->get_options()["me"] == "true");
}

BOOST_AUTO_TEST_CASE(handle_mode)
{
	plugin_->handle_mode(bot_, {});

	BOOST_TEST(plugin_->get_options().size() == 1U);
	BOOST_TEST(plugin_->get_options()["mode"] == "true");
}

BOOST_AUTO_TEST_CASE(handle_names)
{
	plugin_->handle_names(bot_, {});

	BOOST_TEST(plugin_->get_options().size() == 1U);
	BOOST_TEST(plugin_->get_options()["names"] == "true");
}

BOOST_AUTO_TEST_CASE(handle_nick)
{
	plugin_->handle_nick(bot_, {});

	BOOST_TEST(plugin_->get_options().size() == 1U);
	BOOST_TEST(plugin_->get_options()["nick"] == "true");
}

BOOST_AUTO_TEST_CASE(handle_notice)
{
	plugin_->handle_notice(bot_, {});

	BOOST_TEST(plugin_->get_options().size() == 1U);
	BOOST_TEST(plugin_->get_options()["notice"] == "true");
}

BOOST_AUTO_TEST_CASE(handle_part)
{
	plugin_->handle_part(bot_, {});

	BOOST_TEST(plugin_->get_options().size() == 1U);
	BOOST_TEST(plugin_->get_options()["part"] == "true");
}

BOOST_AUTO_TEST_CASE(handle_reload)
{
	plugin_->handle_reload(bot_);

	BOOST_TEST(plugin_->get_options().size() == 1U);
	BOOST_TEST(plugin_->get_options()["reload"] == "true");
}

BOOST_AUTO_TEST_CASE(handle_topic)
{
	plugin_->handle_topic(bot_, {});

	BOOST_TEST(plugin_->get_options().size() == 1U);
	BOOST_TEST(plugin_->get_options()["topic"] == "true");
}

BOOST_AUTO_TEST_CASE(handle_unload)
{
	plugin_->handle_unload(bot_);

	BOOST_TEST(plugin_->get_options().size() == 1U);
	BOOST_TEST(plugin_->get_options()["unload"] == "true");
}

BOOST_AUTO_TEST_CASE(handle_whois)
{
	plugin_->handle_whois(bot_, {});

	BOOST_TEST(plugin_->get_options().size() == 1U);
	BOOST_TEST(plugin_->get_options()["whois"] == "true");
}

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
