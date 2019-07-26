/*
 * main.cpp -- test plugin_service object
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

#define BOOST_TEST_MODULE "plugin_service"
#include <boost/test/unit_test.hpp>

#include <irccd/daemon/dynlib_plugin.hpp>
#include <irccd/daemon/plugin_service.hpp>

#include <irccd/test/irccd_fixture.hpp>
#include <irccd/test/mock_plugin.hpp>
#include <irccd/test/test_plugin_loader.hpp>

namespace test = irccd::test;

namespace irccd::daemon {

namespace {

BOOST_FIXTURE_TEST_SUITE(plugin_test_suite, test::irccd_fixture)

BOOST_AUTO_TEST_CASE(add)
{
	bot_.get_plugins().add(std::make_shared<test::mock_plugin>("p1"));
	bot_.get_plugins().add(std::make_shared<test::mock_plugin>("p2"));

	BOOST_TEST(bot_.get_plugins().list().size() == 2U);
	BOOST_TEST(bot_.get_plugins().list()[0]->get_id() == "p1");
	BOOST_TEST(bot_.get_plugins().list()[1]->get_id() == "p2");
}

BOOST_AUTO_TEST_CASE(get)
{
	bot_.get_plugins().add(std::make_shared<test::mock_plugin>("p1"));
	bot_.get_plugins().add(std::make_shared<test::mock_plugin>("p2"));

	BOOST_TEST(bot_.get_plugins().get("p1"));
	BOOST_TEST(bot_.get_plugins().get("p2"));
	BOOST_TEST(!bot_.get_plugins().get("none"));
}

BOOST_AUTO_TEST_CASE(require)
{
	bot_.get_plugins().add(std::make_shared<test::mock_plugin>("p1"));
	bot_.get_plugins().add(std::make_shared<test::mock_plugin>("p2"));

	BOOST_TEST(bot_.get_plugins().require("p1"));
	BOOST_TEST(bot_.get_plugins().require("p2"));
	BOOST_REQUIRE_THROW(bot_.get_plugins().require("none"), plugin_error);
}

BOOST_AUTO_TEST_CASE(get_options)
{
	bot_.set_config(config{CMAKE_CURRENT_SOURCE_DIR "/options.conf"});

	BOOST_TEST(bot_.get_plugins().get_options("p1").size() == 1U);
	BOOST_TEST(bot_.get_plugins().get_options("p1")["difficulty"] == "extreme");
}

BOOST_AUTO_TEST_CASE(get_templates)
{
	bot_.set_config(config{CMAKE_CURRENT_SOURCE_DIR "/templates.conf"});

	BOOST_TEST(bot_.get_plugins().get_templates("p1").size() == 1U);
	BOOST_TEST(bot_.get_plugins().get_templates("p1")["win"] == "congratulations, you've won");
}

BOOST_AUTO_TEST_CASE(get_paths)
{
	bot_.set_config(config{CMAKE_CURRENT_SOURCE_DIR "/paths.conf"});

	BOOST_TEST(bot_.get_plugins().get_paths("p1").size() == 3U);
	BOOST_TEST(bot_.get_plugins().get_paths("p1")["cache"] == "/var/super-cache");
	BOOST_TEST(bot_.get_plugins().get_paths("p1")["config"] == "/etc/plugin/p1");
	BOOST_TEST(bot_.get_plugins().get_paths("p1")["data"] == "/share/plugin/p1");

	BOOST_TEST(bot_.get_plugins().get_paths("p2").size() == 3U);
	BOOST_TEST(bot_.get_plugins().get_paths("p2")["cache"] == "/var/cache/plugin/p2");
	BOOST_TEST(bot_.get_plugins().get_paths("p2")["config"] == "/super-etc");
	BOOST_TEST(bot_.get_plugins().get_paths("p2")["data"] == "/share/plugin/p2");

	BOOST_TEST(bot_.get_plugins().get_paths("p3").size() == 3U);
	BOOST_TEST(bot_.get_plugins().get_paths("p3")["cache"] == "/var/cache/plugin/p3");
	BOOST_TEST(bot_.get_plugins().get_paths("p3")["config"] == "/etc/plugin/p3");
	BOOST_TEST(bot_.get_plugins().get_paths("p3")["data"] == "/super-share");

	BOOST_TEST(bot_.get_plugins().get_paths("all").size() == 3U);
	BOOST_TEST(bot_.get_plugins().get_paths("all")["cache"] == "/var/super-cache");
	BOOST_TEST(bot_.get_plugins().get_paths("all")["config"] == "/super-etc");
	BOOST_TEST(bot_.get_plugins().get_paths("all")["data"] == "/super-share");
}

BOOST_AUTO_TEST_CASE(open)
{
	bot_.get_plugins().add_loader(std::make_unique<dynlib_plugin_loader>());
	bot_.get_plugins().add_loader(std::make_unique<test::test_plugin_loader>());

	BOOST_TEST(bot_.get_plugins().open("mock", ""));
}

BOOST_AUTO_TEST_CASE(load)
{
	bot_.get_plugins().add_loader(std::make_unique<dynlib_plugin_loader>());
	bot_.get_plugins().add_loader(std::make_unique<test::test_plugin_loader>());
	bot_.get_plugins().load("mock", "");

	BOOST_TEST(bot_.get_plugins().list().size() == 1U);

	const auto mock = std::dynamic_pointer_cast<test::mock_plugin>(bot_.get_plugins().get("mock"));

	BOOST_TEST(mock);
	BOOST_TEST(mock->find("handle_load").size() == 1U);
}

BOOST_AUTO_TEST_CASE(unload)
{
	bot_.get_plugins().add_loader(std::make_unique<dynlib_plugin_loader>());
	bot_.get_plugins().add_loader(std::make_unique<test::test_plugin_loader>());
	bot_.get_plugins().load("mock", "");

	const auto mock = std::dynamic_pointer_cast<test::mock_plugin>(bot_.get_plugins().get("mock"));

	bot_.get_plugins().unload("mock");

	BOOST_TEST(bot_.get_plugins().list().empty());
	BOOST_TEST(mock);
	BOOST_TEST(mock->find("handle_unload").size() == 1U);
}

BOOST_AUTO_TEST_CASE(reload)
{
	bot_.get_plugins().add_loader(std::make_unique<dynlib_plugin_loader>());
	bot_.get_plugins().add_loader(std::make_unique<test::test_plugin_loader>());
	bot_.get_plugins().load("mock", "");
	bot_.get_plugins().reload("mock");

	BOOST_TEST(bot_.get_plugins().list().size() == 1U);

	const auto mock = std::dynamic_pointer_cast<test::mock_plugin>(bot_.get_plugins().get("mock"));

	BOOST_TEST(mock);
	BOOST_TEST(mock->find("handle_reload").size() == 1U);
}

BOOST_AUTO_TEST_CASE(clear)
{
	auto m1 = std::make_shared<test::mock_plugin>("m1");
	auto m2 = std::make_shared<test::mock_plugin>("m2");

	bot_.get_plugins().add(m1);
	bot_.get_plugins().add(m2);
	bot_.get_plugins().clear();

	BOOST_TEST(bot_.get_plugins().list().empty());
	BOOST_TEST(m1->find("handle_unload").size() == 1U);
	BOOST_TEST(m2->find("handle_unload").size() == 1U);
}

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd::daemon
