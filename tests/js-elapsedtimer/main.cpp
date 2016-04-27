/*
 * main.cpp -- test Irccd.ElapsedTimer API
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

#include <thread>

#include <irccd/js-irccd.hpp>
#include <irccd/js-elapsed-timer.hpp>

using namespace irccd;
using namespace std::chrono_literals;

class TestElapsedTimer : public testing::Test {
protected:
	duk::Context m_context;

	TestElapsedTimer()
	{
		loadJsIrccd(m_context);
		loadJsElapsedTimer(m_context);
	}
};

TEST_F(TestElapsedTimer, standard)
{
	try {
		if (duk::pevalString(m_context, "timer = new Irccd.ElapsedTimer();") != 0)
			throw duk::error(m_context, -1);

		std::this_thread::sleep_for(300ms);

		if (duk::pevalString(m_context, "result = timer.elapsed();") != 0)
			throw duk::error(m_context, -1);

		ASSERT_GE(duk::getGlobal<int>(m_context, "result"), 250);
		ASSERT_LE(duk::getGlobal<int>(m_context, "result"), 350);
	} catch (const std::exception &ex) {
		FAIL() << ex.what();
	}
}

TEST_F(TestElapsedTimer, reset)
{
	try {
		if (duk::pevalString(m_context, "timer = new Irccd.ElapsedTimer();") != 0)
			throw duk::error(m_context, -1);

		std::this_thread::sleep_for(300ms);

		if (duk::pevalString(m_context, "timer.reset(); result = timer.elapsed();") != 0)
			throw duk::error(m_context, -1);

		ASSERT_LE(duk::getGlobal<int>(m_context, "result"), 100);
	} catch (const std::exception &ex) {
		FAIL() << ex.what();
	}
}

int main(int argc, char **argv)
{
	testing::InitGoogleTest(&argc, argv);

	return RUN_ALL_TESTS();
}
