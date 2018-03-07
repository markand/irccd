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

#include <irccd/daemon/server_disconnect_command.hpp>
#include <irccd/daemon/service/server_service.hpp>

#include <irccd/test/journal_server.hpp>
#include <irccd/test/command_test.hpp>

namespace irccd {

namespace {

class server_disconnect_test : public command_test<server_disconnect_command> {
protected:
    server_disconnect_test()
    {
        daemon_->servers().add(std::make_unique<journal_server>(service_, "s1"));
        daemon_->servers().add(std::make_unique<journal_server>(service_, "s2"));
    }
};

} // !namespace

BOOST_FIXTURE_TEST_SUITE(server_disconnect_test_suite, server_disconnect_test)

BOOST_AUTO_TEST_CASE(one)
{
    nlohmann::json result;

    ctl_->send({
        { "command",    "server-disconnect" },
        { "server",     "s1"                }
    });
    ctl_->recv([&] (auto, auto msg) {
        result = msg;
    });

    wait_for([&] () {
        return result.is_object();
    });

    BOOST_TEST(result["command"].get<std::string>() == "server-disconnect");
    BOOST_TEST(!daemon_->servers().has("s1"));
    BOOST_TEST(daemon_->servers().has("s2"));
}

BOOST_AUTO_TEST_CASE(all)
{
    nlohmann::json result;

    ctl_->send({{"command", "server-disconnect"}});
    ctl_->recv([&] (auto, auto msg) {
        result = msg;
    });

    wait_for([&] () {
        return result.is_object();
    });

    BOOST_TEST(result["command"].get<std::string>() == "server-disconnect");
    BOOST_TEST(!daemon_->servers().has("s1"));
    BOOST_TEST(!daemon_->servers().has("s2"));
}

BOOST_AUTO_TEST_SUITE(errors)

BOOST_AUTO_TEST_CASE(invalid_identifier)
{
    boost::system::error_code result;
    nlohmann::json message;

    ctl_->send({
        { "command",    "server-disconnect" },
        { "server",     123456              }
    });
    ctl_->recv([&] (auto rresult, auto rmessage) {
        result = rresult;
        message = rmessage;
    });

    wait_for([&] {
        return result;
    });

    BOOST_TEST(result == server_error::invalid_identifier);
    BOOST_TEST(message["error"].template get<int>() == server_error::invalid_identifier);
    BOOST_TEST(message["errorCategory"].template get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(not_found)
{
    boost::system::error_code result;
    nlohmann::json message;

    ctl_->send({
        { "command",    "server-disconnect" },
        { "server",     "unknown"           }
    });
    ctl_->recv([&] (auto rresult, auto rmessage) {
        result = rresult;
        message = rmessage;
    });

    wait_for([&] {
        return result;
    });

    BOOST_TEST(result == server_error::not_found);
    BOOST_TEST(message["error"].template get<int>() == server_error::not_found);
    BOOST_TEST(message["errorCategory"].template get<std::string>() == "server");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

} // !irccd
