/*
 * main.cpp -- test server-list remote command
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

#define BOOST_TEST_MODULE "server-list"
#include <boost/test/unit_test.hpp>

#include <irccd/daemon/server_service.hpp>

#include <journal_server.hpp>
#include <command_test.hpp>

namespace irccd {

namespace {

class server_list_test : public command_test<server_list_command> {
protected:
    server_list_test()
    {
        daemon_->servers().add(std::make_unique<journal_server>(service_, "s1"));
        daemon_->servers().add(std::make_unique<journal_server>(service_, "s2"));
    }
};

} // !namespace

BOOST_FIXTURE_TEST_SUITE(server_list_test_suite, server_list_test)

BOOST_AUTO_TEST_CASE(basic)
{
    nlohmann::json result;

    ctl_->send({{"command", "server-list"}});
    ctl_->recv([&] (auto, auto msg) {
        result = msg;
    });

    wait_for([&] () {
        return result.is_object();
    });

    BOOST_TEST(result.is_object());
    BOOST_TEST(result["list"].is_array());
    BOOST_TEST(result["list"].size() == 2U);
    BOOST_TEST(result["list"][0].get<std::string>() == "s1");
    BOOST_TEST(result["list"][1].get<std::string>() == "s2");
}

BOOST_AUTO_TEST_SUITE_END()

} // !irccd
