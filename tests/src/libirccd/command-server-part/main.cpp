/*
 * main.cpp -- test server-part remote command
 *
 * Copyright (c) 2013-2018 David Demelier <markand@malikania.fr>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright part and this permission part appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#define BOOST_TEST_MODULE "server-part"
#include <boost/test/unit_test.hpp>

#include <irccd/daemon/server_service.hpp>
#include <irccd/daemon/server_part_command.hpp>

#include <irccd/test/command_test.hpp>
#include <irccd/test/journal_server.hpp>

namespace irccd {

namespace {

class server_part_test : public command_test<server_part_command> {
protected:
    std::shared_ptr<journal_server> server_{new journal_server(service_, "test")};

    server_part_test()
    {
        daemon_->servers().add(server_);
    }
};

} // !namespace

BOOST_FIXTURE_TEST_SUITE(server_part_test_suite, server_part_test)

BOOST_AUTO_TEST_CASE(basic)
{
    ctl_->send({
        { "command",    "server-part"   },
        { "server",     "test"          },
        { "channel",    "#staff"        },
        { "reason",     "too noisy"     }
    });

    wait_for([this] () {
        return !server_->cqueue().empty();
    });

    auto cmd = server_->cqueue().back();

    BOOST_TEST(cmd["command"].get<std::string>() == "part");
    BOOST_TEST(cmd["channel"].get<std::string>() == "#staff");
    BOOST_TEST(cmd["reason"].get<std::string>() == "too noisy");
}

BOOST_AUTO_TEST_CASE(noreason)
{
    ctl_->send({
        { "command",    "server-part"   },
        { "server",     "test"          },
        { "channel",    "#staff"        }
    });

    wait_for([this] () {
        return !server_->cqueue().empty();
    });

    auto cmd = server_->cqueue().back();

    BOOST_TEST(cmd["command"].get<std::string>() == "part");
    BOOST_TEST(cmd["channel"].get<std::string>() == "#staff");
    BOOST_TEST(cmd["reason"].get<std::string>() == "");
}

BOOST_AUTO_TEST_SUITE(errors)

BOOST_AUTO_TEST_CASE(invalid_identifier_1)
{
    boost::system::error_code result;
    nlohmann::json message;

    ctl_->send({
        { "command",    "server-part"   },
        { "server",     123456          },
        { "channel",    "#music"        }
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

BOOST_AUTO_TEST_CASE(invalid_identifier_2)
{
    boost::system::error_code result;
    nlohmann::json message;

    ctl_->send({
        { "command",    "server-part"   },
        { "server",     ""              },
        { "channel",    "#music"        }
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

BOOST_AUTO_TEST_CASE(invalid_channel_1)
{
    boost::system::error_code result;
    nlohmann::json message;

    ctl_->send({
        { "command",    "server-part"   },
        { "server",     "test"          },
        { "channel",    ""              }
    });
    ctl_->recv([&] (auto rresult, auto rmessage) {
        result = rresult;
        message = rmessage;
    });

    wait_for([&] {
        return result;
    });

    BOOST_TEST(result == server_error::invalid_channel);
    BOOST_TEST(message["error"].template get<int>() == server_error::invalid_channel);
    BOOST_TEST(message["errorCategory"].template get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(invalid_channel_2)
{
    boost::system::error_code result;
    nlohmann::json message;

    ctl_->send({
        { "command",    "server-part"   },
        { "server",     "test"          },
        { "channel",    123456          }
    });
    ctl_->recv([&] (auto rresult, auto rmessage) {
        result = rresult;
        message = rmessage;
    });

    wait_for([&] {
        return result;
    });

    BOOST_TEST(result == server_error::invalid_channel);
    BOOST_TEST(message["error"].template get<int>() == server_error::invalid_channel);
    BOOST_TEST(message["errorCategory"].template get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(not_found)
{
    boost::system::error_code result;
    nlohmann::json message;

    ctl_->send({
        { "command",    "server-part"   },
        { "server",     "unknown"       },
        { "channel",    "#music"        }
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
