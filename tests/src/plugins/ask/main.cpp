/*
 * main.cpp -- test ask plugin
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

#define BOOST_TEST_MODULE "Ask plugin"
#include <boost/test/unit_test.hpp>

#include <irccd/daemon/irccd.hpp>
#include <irccd/daemon/server.hpp>

#include <irccd/test/plugin_test.hpp>

namespace irccd {

class ask_test : public plugin_test {
public:
    inline ask_test()
        : plugin_test(PLUGIN_NAME, PLUGIN_PATH)
    {
        plugin_->set_config({
            { "file", CMAKE_CURRENT_SOURCE_DIR "/answers.conf" }
        });
        plugin_->handle_load(irccd_);
    }
};

BOOST_FIXTURE_TEST_SUITE(ask_test_suite, ask_test)

BOOST_AUTO_TEST_CASE(basic)
{
    bool no = false;
    bool yes = false;

    /*
     * Invoke the plugin 1000 times, it will be very unlucky to not have both
     * answers in that amount of tries.
     */
    for (int i = 0; i < 1000; ++i) {
        plugin_->handle_command(irccd_, {server_, "tester", "#dummy", ""});

        auto cmd = server_->cqueue().front();

        BOOST_REQUIRE_EQUAL(cmd["command"].get<std::string>(), "message");
        BOOST_REQUIRE_EQUAL(cmd["target"].get<std::string>(), "#dummy");

        auto msg = cmd["message"].get<std::string>();

        if (msg == "tester, YES")
            yes = true;
        if (msg == "tester, NO")
            no = true;

        server_->cqueue().clear();
    }

    BOOST_REQUIRE(no);
    BOOST_REQUIRE(yes);
}

BOOST_AUTO_TEST_SUITE_END()

} // !irccd
