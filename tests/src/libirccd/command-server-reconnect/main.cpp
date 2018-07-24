/*
 * main.cpp -- test server-reconnect remote command
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

#define BOOST_TEST_MODULE "server-reconnect"
#include <boost/test/unit_test.hpp>

#include <irccd/daemon/command/server_reconnect_command.hpp>
#include <irccd/daemon/service/server_service.hpp>

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

BOOST_FIXTURE_TEST_SUITE(server_reconnect_test_suite, server_reconnect_test)

BOOST_AUTO_TEST_CASE(basic)
{
    ctl_->write({
        { "command",    "server-reconnect"  },
        { "server",     "s1"                }
    });

    wait_for([this] () {
        return !server1_->cqueue().empty();
    });

    auto cmd1 = server1_->cqueue().back();

#if 0
    BOOST_TEST(cmd1["command"].get<std::string>() == "reconnect");
    BOOST_TEST(server2_->cqueue().empty());
#endif
}

BOOST_AUTO_TEST_CASE(all)
{
    ctl_->write({{"command", "server-reconnect"}});

    wait_for([this] () {
        return !server1_->cqueue().empty() && !server2_->cqueue().empty();
    });

    auto cmd1 = server1_->cqueue().back();
    auto cmd2 = server2_->cqueue().back();

#if 0
    BOOST_TEST(cmd1["command"].get<std::string>() == "reconnect");
    BOOST_TEST(cmd2["command"].get<std::string>() == "reconnect");
#endif
}

BOOST_AUTO_TEST_SUITE(errors)

BOOST_AUTO_TEST_CASE(invalid_identifier_1)
{
    const auto result = request({
        { "command",    "server-reconnect"  },
        { "server",     123456              }
    });

    BOOST_TEST(result.second == server_error::invalid_identifier);
    BOOST_TEST(result.first["error"].template get<int>() == server_error::invalid_identifier);
    BOOST_TEST(result.first["errorCategory"].template get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(invalid_identifier_2)
{
    const auto result = request({
        { "command",    "server-reconnect"  },
        { "server",     ""                  }
    });

    BOOST_TEST(result.second == server_error::invalid_identifier);
    BOOST_TEST(result.first["error"].template get<int>() == server_error::invalid_identifier);
    BOOST_TEST(result.first["errorCategory"].template get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(not_found)
{
    const auto result = request({
        { "command",    "server-reconnect"  },
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
