/*
 * main.cpp -- test plugin-list remote command
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

#define BOOST_TEST_MODULE "plugin-list"
#include <boost/test/unit_test.hpp>

#include <irccd/daemon/plugin_list_command.hpp>
#include <irccd/daemon/plugin_service.hpp>

#include <irccd/test/command_test.hpp>

namespace irccd {

class plugin_list_test : public command_test<plugin_list_command> {
public:
    plugin_list_test()
    {
        daemon_->plugins().add(std::make_unique<plugin>("t1", ""));
        daemon_->plugins().add(std::make_unique<plugin>("t2", ""));
    }
};

BOOST_FIXTURE_TEST_SUITE(plugin_list_test_suite, plugin_list_test)

BOOST_AUTO_TEST_CASE(basic)
{
    auto response = nlohmann::json();

    ctl_->send({{"command", "plugin-list"}});
    ctl_->recv([&] (auto, auto message) {
        response = message;
    });

    wait_for([&] () {
        return response.is_object();
    });

    BOOST_TEST(response.is_object());
    BOOST_TEST("t1", response["list"][0]);
    BOOST_TEST("t2", response["list"][1]);
}

BOOST_AUTO_TEST_SUITE_END()

} // !irccd
