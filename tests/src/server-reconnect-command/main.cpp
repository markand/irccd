/*
 * main.cpp -- test server-reconnect remote command
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

#define BOOST_TEST_MODULE "server-reconnect"
#include <boost/test/unit_test.hpp>

#include <irccd/daemon/server_service.hpp>

#include <irccd/test/command_test.hpp>
#include <irccd/test/journal_server.hpp>

namespace irccd {

namespace {

class server_reconnect_test : public command_test<server_reconnect_command> {
protected:
    std::shared_ptr<journal_server> server1_{new journal_server(service_, "s1")};
    std::shared_ptr<journal_server> server2_{new journal_server(service_, "s2")};

    server_reconnect_test()
    {
        daemon_->servers().add(server1_);
        daemon_->servers().add(server2_);
    }
};

} // !namespace

BOOST_FIXTURE_TEST_SUITE(server_reconnect_test_suite, server_reconnect_test)

BOOST_AUTO_TEST_CASE(basic)
{
    ctl_->send({
        { "command",    "server-reconnect"  },
        { "server",     "s1"                }
    });

    wait_for([this] () {
        return !server1_->cqueue().empty();
    });

    auto cmd1 = server1_->cqueue().back();

    BOOST_TEST(cmd1["command"].get<std::string>() == "reconnect");
    BOOST_TEST(server2_->cqueue().empty());
}

BOOST_AUTO_TEST_CASE(all)
{
    ctl_->send({{"command", "server-reconnect"}});

    wait_for([this] () {
        return !server1_->cqueue().empty() && !server2_->cqueue().empty();
    });

    auto cmd1 = server1_->cqueue().back();
    auto cmd2 = server2_->cqueue().back();

    BOOST_TEST(cmd1["command"].get<std::string>() == "reconnect");
    BOOST_TEST(cmd2["command"].get<std::string>() == "reconnect");
}

BOOST_AUTO_TEST_SUITE(errors)

BOOST_AUTO_TEST_CASE(invalid_identifier_1)
{
    boost::system::error_code result;
    nlohmann::json message;

    ctl_->send({
        { "command",    "server-reconnect"  },
        { "server",     123456              }
    });
    ctl_->recv([&] (auto rresult, auto rmessage) {
        result = rresult;
        message = rmessage;
    });

    wait_for([&] {
        return result;
    });

    BOOST_ASSERT(result == server_error::invalid_identifier);
    BOOST_ASSERT(message["error"].template get<int>() == server_error::invalid_identifier);
    BOOST_ASSERT(message["errorCategory"].template get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(invalid_identifier_2)
{
    boost::system::error_code result;
    nlohmann::json message;

    ctl_->send({
        { "command",    "server-reconnect"  },
        { "server",     ""                  }
    });
    ctl_->recv([&] (auto rresult, auto rmessage) {
        result = rresult;
        message = rmessage;
    });

    wait_for([&] {
        return result;
    });

    BOOST_ASSERT(result == server_error::invalid_identifier);
    BOOST_ASSERT(message["error"].template get<int>() == server_error::invalid_identifier);
    BOOST_ASSERT(message["errorCategory"].template get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(not_found)
{
    boost::system::error_code result;
    nlohmann::json message;

    ctl_->send({
        { "command",    "server-reconnect"  },
        { "server",     "unknown"           }
    });
    ctl_->recv([&] (auto rresult, auto rmessage) {
        result = rresult;
        message = rmessage;
    });

    wait_for([&] {
        return result;
    });

    BOOST_ASSERT(result == server_error::not_found);
    BOOST_ASSERT(message["error"].template get<int>() == server_error::not_found);
    BOOST_ASSERT(message["errorCategory"].template get<std::string>() == "server");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

} // !irccd
