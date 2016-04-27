/*
 * main.cpp -- test ElapsedTimer
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

#include <thread>

#include <gtest/gtest.h>

#include <irccd/elapsed-timer.hpp>

using namespace irccd;
using namespace std::chrono_literals;

TEST(TestElapsedTimer, standard)
{
	ElapsedTimer timer;

	std::this_thread::sleep_for(300ms);

	ASSERT_GE(timer.elapsed(), 250U);
	ASSERT_LE(timer.elapsed(), 350U);
}

TEST(TestElapsedTimer, reset)
{
	ElapsedTimer timer;

	std::this_thread::sleep_for(300ms);

	timer.reset();

	ASSERT_LE(timer.elapsed(), 100U);
}

int main(int argc, char **argv)
{
	testing::InitGoogleTest(&argc, argv);

	return RUN_ALL_TESTS();
}

