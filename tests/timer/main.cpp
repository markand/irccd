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

#define BOOST_TEST_MODULE "Timer"
#include <boost/test/unit_test.hpp>
#include <boost/timer/timer.hpp>

#include <irccd/js/timer.hpp>

using namespace std::chrono_literals;

namespace irccd {

/*
 * timer object itself
 * --------------------------------------------------------
 */

BOOST_AUTO_TEST_SUITE(timer_suite)

BOOST_AUTO_TEST_CASE(single)
{
    timer timer(timer::type::single, 1000);
    boost::timer::cpu_timer elapsed;
    int count = 0;

    timer.on_signal.connect([&] () {
        count = elapsed.elapsed().wall / 1000000LL;
    });

    elapsed.start();
    timer.start();

    std::this_thread::sleep_for(3s);

    BOOST_REQUIRE_GE(count, 900);
    BOOST_REQUIRE_LE(count, 1100);
}

BOOST_AUTO_TEST_CASE(repeat)
{
    timer timer(timer::type::repeat, 500);
    int max = 0;

    timer.on_signal.connect([&] () {
        max ++;
    });

    timer.start();

    // Should be at least 5
    std::this_thread::sleep_for(3s);

    BOOST_REQUIRE_GE(max, 5);

    timer.stop();
}

BOOST_AUTO_TEST_CASE(restart)
{
    timer timer(timer::type::repeat, 500);
    int max = 0;

    timer.on_signal.connect([&] () {
        max ++;
    });

    timer.start();
    std::this_thread::sleep_for(3s);
    timer.stop();
    std::this_thread::sleep_for(3s);
    timer.start();
    std::this_thread::sleep_for(3s);

    BOOST_REQUIRE_GE(max, 10);
    BOOST_REQUIRE_LT(max, 15);

    timer.stop();
}

BOOST_AUTO_TEST_SUITE_END()

} // !irccd
