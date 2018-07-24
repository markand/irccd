/*
 * main.cpp -- test server-list remote command
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

#define BOOST_TEST_MODULE "server-list"
#include <boost/test/unit_test.hpp>

#include <irccd/daemon/command/server_list_command.hpp>
#include <irccd/daemon/service/server_service.hpp>

#include <irccd/test/command_test.hpp>
#include <irccd/test/mock_server.hpp>

namespace irccd {

namespace {

class server_list_test : public command_test<server_list_command> {
protected:
    server_list_test()
    {
        daemon_->servers().add(std::make_unique<mock_server>(service_, "s1", "localhost"));
        daemon_->servers().add(std::make_unique<mock_server>(service_, "s2", "localhost"));
    }
};

BOOST_FIXTURE_TEST_SUITE(server_list_test_suite, server_list_test)

BOOST_AUTO_TEST_CASE(basic)
{
    const auto result = request({
        { "command", "server-list" }
    });

    BOOST_TEST(result.first.is_object());
    BOOST_TEST(result.first["list"].is_array());
    BOOST_TEST(result.first["list"].size() == 2U);
    BOOST_TEST(result.first["list"][0].get<std::string>() == "s1");
    BOOST_TEST(result.first["list"][1].get<std::string>() == "s2");
}

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
