/*
 * main.cpp -- test Irccd.ElapsedTimer API
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

#define BOOST_TEST_MODULE "ElapsedTimer Javascript API"
#include <boost/test/unit_test.hpp>

#include <thread>

#include <irccd/test/js_fixture.hpp>

using namespace std::chrono_literals;

using namespace irccd::js;
using namespace irccd::test;

namespace irccd {

namespace {

BOOST_FIXTURE_TEST_SUITE(elapsed_timer_js_api_suite, js_fixture)

BOOST_AUTO_TEST_CASE(standard)
{
    if (duk_peval_string(plugin_->get_context(), "timer = new Irccd.ElapsedTimer();") != 0)
        throw duk::get_stack(plugin_->get_context(), -1);

    std::this_thread::sleep_for(300ms);

    if (duk_peval_string(plugin_->get_context(), "result = timer.elapsed();") != 0)
        throw duk::get_stack(plugin_->get_context(), -1);

    BOOST_REQUIRE(duk_get_global_string(plugin_->get_context(), "result"));
    BOOST_REQUIRE_GE(duk_get_int(plugin_->get_context(), -1), 250);
    BOOST_REQUIRE_LE(duk_get_int(plugin_->get_context(), -1), 350);
}

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
