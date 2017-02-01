/*
 * main.cpp -- test irccd timer
 *
 * Copyright (c) 2013-2017 David Demelier <markand@malikania.fr>
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
#include <irccd/timer.hpp>

using namespace irccd;
using namespace std::chrono_literals;

/* --------------------------------------------------------
 * Timer object itself
 * -------------------------------------------------------- */

TEST(Basic, single)
{
    Timer timer(TimerType::Single, 1000);
    ElapsedTimer elapsed;
    int count = 0;

    timer.onSignal.connect([&] () {
        count = elapsed.elapsed();
    });

    elapsed.reset();
    timer.start();

    std::this_thread::sleep_for(3s);

    ASSERT_GE(count, 900);
    ASSERT_LE(count, 1100);

}

TEST(Basic, repeat)
{
    Timer timer(TimerType::Repeat, 500);
    int max = 0;

    timer.onSignal.connect([&] () {
        max ++;
    });

    timer.start();

    // Should be at least 5
    std::this_thread::sleep_for(3s);

    ASSERT_GE(max, 5);

    timer.stop();
}

TEST(Basic, restart)
{
    Timer timer(TimerType::Repeat, 500);
    int max = 0;

    timer.onSignal.connect([&] () {
        max ++;
    });

    timer.start();
    std::this_thread::sleep_for(3s);
    timer.stop();
    std::this_thread::sleep_for(3s);
    timer.start();
    std::this_thread::sleep_for(3s);

    ASSERT_GE(max, 10);
    ASSERT_LT(max, 15);

    timer.stop();
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
