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

#include <irccd/js/plugin_jsapi.hpp>
#include <irccd/js/timer_jsapi.hpp>

#include <irccd/test/js_test.hpp>

#include <irccd/js/logger_jsapi.hpp>

namespace irccd {

namespace {

class js_timer_test : public js_test<plugin_jsapi, timer_jsapi> {
public:
    js_timer_test()
        : js_test(CMAKE_CURRENT_SOURCE_DIR "/timer.js")
    {
    }

    void set_type(const std::string& name)
    {
        duk_get_global_string(plugin_->context(), "Irccd");
        duk_get_prop_string(plugin_->context(), -1, "Timer");
        duk_get_prop_string(plugin_->context(), -1, name.c_str());
        duk_put_global_string(plugin_->context(), "type");
        duk_pop_n(plugin_->context(), 2);

        plugin_->open();
        plugin_->on_load(irccd_);
    }
};

} // !namespace

BOOST_FIXTURE_TEST_SUITE(js_timer_test_suite, js_timer_test)

BOOST_AUTO_TEST_CASE(single)
{
    boost::timer::cpu_timer timer;

    set_type("Single");

    while (timer.elapsed().wall / 1000000LL < 3000) {
        service_.reset();
        service_.poll();
    }

    BOOST_TEST(duk_get_global_string(plugin_->context(), "count"));
    BOOST_TEST(duk_get_int(plugin_->context(), -1) == 1);
}

BOOST_AUTO_TEST_CASE(repeat)
{
    boost::timer::cpu_timer timer;

    set_type("Repeat");

    while (timer.elapsed().wall / 1000000LL < 3000) {
        service_.reset();
        service_.poll();
    }

    BOOST_TEST(duk_get_global_string(plugin_->context(), "count"));
    BOOST_TEST(duk_get_int(plugin_->context(), -1) >= 5);
}

BOOST_AUTO_TEST_SUITE_END()

} // !irccd
