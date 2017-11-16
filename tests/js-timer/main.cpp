/*
 * main.cpp -- test Irccd.Timer API
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

#define BOOST_TEST_MODULE "Timer Javascript API"
#include <boost/test/unit_test.hpp>
#include <boost/timer/timer.hpp>

#include <irccd/net_util.hpp>

#include <irccd/js/plugin_jsapi.hpp>
#include <irccd/js/timer_jsapi.hpp>

#include <js_test.hpp>

namespace irccd {

class fixture : public js_test<plugin_jsapi, timer_jsapi> {
public:
    using js_test::js_test;
};

BOOST_FIXTURE_TEST_SUITE(timer_jsapi_suite, fixture)

BOOST_AUTO_TEST_CASE(single)
{
    fixture f(CMAKE_CURRENT_SOURCE_DIR "/timer-single.js");

    boost::timer::cpu_timer timer;

    while (timer.elapsed().wall / 1000000LL < 3000)
        net_util::poll(512, f.irccd_);

    BOOST_TEST(duk_get_global_string(f.plugin_->context(), "count"));
    BOOST_TEST(duk_get_int(f.plugin_->context(), -1) == 1);
}

BOOST_AUTO_TEST_CASE(repeat)
{
    fixture f(CMAKE_CURRENT_SOURCE_DIR "/timer-repeat.js");

    boost::timer::cpu_timer timer;

    while (timer.elapsed().wall / 1000000LL < 3000)
        net_util::poll(512, f.irccd_);

    BOOST_TEST(duk_get_global_string(f.plugin_->context(), "count"));
    BOOST_TEST(duk_get_int(f.plugin_->context(), -1) >= 5);
}

BOOST_AUTO_TEST_SUITE_END()

} // !irccd
