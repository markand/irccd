/*
 * main.cpp -- test server-disconnect remote command
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

#define BOOST_TEST_MODULE "server-disconnect"
#include <boost/test/unit_test.hpp>

#include <irccd/daemon/command/server_disconnect_command.hpp>
#include <irccd/daemon/service/server_service.hpp>

#include <irccd/test/mock_server.hpp>
#include <irccd/test/command_test.hpp>

namespace irccd {

namespace {

class server_disconnect_test : public command_test<server_disconnect_command> {
protected:
    std::shared_ptr<mock_server> s1_;
    std::shared_ptr<mock_server> s2_;

    server_disconnect_test()
        : s1_(new mock_server(service_, "s1", "localhost"))
        , s2_(new mock_server(service_, "s2", "localhost"))
    {
        daemon_->servers().add(s1_);
        daemon_->servers().add(s2_);
    }
};

BOOST_FIXTURE_TEST_SUITE(server_disconnect_test_suite, server_disconnect_test)

BOOST_AUTO_TEST_CASE(one)
{
    const auto result = request({
        { "command",    "server-disconnect" },
        { "server",     "s1"                }
    });

    BOOST_TEST(result.first["command"].get<std::string>() == "server-disconnect");
    BOOST_TEST(s1_->find("disconnect").size() == 1U);
    BOOST_TEST(!daemon_->servers().has("s1"));
    BOOST_TEST(daemon_->servers().has("s2"));
}

BOOST_AUTO_TEST_CASE(all)
{
    const auto result = request({
        { "command", "server-disconnect" }
    });

    BOOST_TEST(result.first["command"].get<std::string>() == "server-disconnect");
    BOOST_TEST(s1_->find("disconnect").size() == 1U);
    BOOST_TEST(s2_->find("disconnect").size() == 1U);
    BOOST_TEST(!daemon_->servers().has("s1"));
    BOOST_TEST(!daemon_->servers().has("s2"));
}

BOOST_AUTO_TEST_SUITE(errors)

BOOST_AUTO_TEST_CASE(invalid_identifier)
{
    const auto result = request({
        { "command",    "server-disconnect" },
        { "server",     123456              }
    });

    BOOST_TEST(result.second == server_error::invalid_identifier);
    BOOST_TEST(result.first["error"].template get<int>() == server_error::invalid_identifier);
    BOOST_TEST(result.first["errorCategory"].template get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(not_found)
{
    const auto result = request({
        { "command",    "server-disconnect" },
        { "server",     "unknown"           }
    });

    BOOST_TEST(result.second == server_error::not_found);
    BOOST_TEST(result.first["error"].template get<int>() == server_error::not_found);
    BOOST_TEST(result.first["errorCategory"].template get<std::string>() == "server");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
