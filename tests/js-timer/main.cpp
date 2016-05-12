/*
 * main.cpp -- test Irccd.Timer API
 *
 * Copyright (c) 2013-2016 David Demelier <markand@malikania.fr>
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

#include <gtest/gtest.h>

#include <irccd/elapsed-timer.hpp>
#include <irccd/irccd.hpp>
#include <irccd/logger.hpp>
#include <irccd/plugin-js.hpp>
#include <irccd/service-plugin.hpp>
#include <irccd/system.hpp>

using namespace irccd;

TEST(Basic, single)
{
	Irccd irccd;
	ElapsedTimer timer;

	auto plugin = std::make_shared<JsPlugin>("timer", IRCCD_TESTS_DIRECTORY "/timer-single.js");

	irccd.pluginService().add(plugin);

	while (timer.elapsed() < 3000) {
		irccd.poll();
		irccd.dispatch();
	}

	ASSERT_EQ(1, duk::getGlobal<int>(plugin->context(), "count"));
}

TEST(Basic, repeat)
{
	Irccd irccd;
	ElapsedTimer timer;

	auto plugin = std::make_shared<JsPlugin>("timer", IRCCD_TESTS_DIRECTORY "/timer-repeat.js");

	irccd.pluginService().add(plugin);

	while (timer.elapsed() < 3000) {
		irccd.poll();
		irccd.dispatch();
	}

	ASSERT_GE(duk::getGlobal<int>(plugin->context(), "count"), 5);
}

#if 0

/*
 * XXX: currently disabled because it will break single-shot timers.
 */

TEST(Basic, pending)
{
	/*
	 * This test ensure that if pending actions on a stopped timer are never executed.
	 */
	Irccd irccd;
	ElapsedTimer timer;

	auto plugin = std::make_shared<Plugin>("timer", IRCCD_TESTS_DIRECTORY "/timer-pending.js");

	irccd.addPlugin(plugin);
	irccd.poll();
	irccd.dispatch();

	ASSERT_EQ(0, plugin->context().getGlobal<int>("count"));
}

#endif

int main(int argc, char **argv)
{
	/* Needed for some components */
	sys::setProgramName("irccd");
	path::setApplicationPath(argv[0]);
	log::setInterface(std::make_unique<log::Console>());
	log::setVerbose(true);
	testing::InitGoogleTest(&argc, argv);

	return RUN_ALL_TESTS();
}
