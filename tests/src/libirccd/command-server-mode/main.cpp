/*
 * main.cpp -- test server-mode remote command
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

#define BOOST_TEST_MODULE "server-mode"
#include <boost/test/unit_test.hpp>

#include <irccd/daemon/server_service.hpp>
#include <irccd/daemon/server_mode_command.hpp>

#include <irccd/test/command_test.hpp>
#include <irccd/test/journal_server.hpp>

namespace irccd {

namespace {

class server_mode_test : public command_test<server_mode_command> {
protected:
    std::shared_ptr<journal_server> server_{new journal_server(service_, "test")};

    server_mode_test()
    {
        daemon_->servers().add(server_);
    }
};

} // !namespace

BOOST_FIXTURE_TEST_SUITE(server_mode_test_suite, server_mode_test)

BOOST_AUTO_TEST_CASE(basic)
{
    ctl_->send({
        { "command",    "server-mode"   },
        { "server",     "test"          },
        { "channel",    "#irccd"        },
        { "mode",       "+t"            }
    });

    wait_for([this] () {
        return !server_->cqueue().empty();
    });

    auto cmd = server_->cqueue().back();

    BOOST_TEST(cmd["command"].get<std::string>() == "mode");
    BOOST_TEST(cmd["channel"].get<std::string>() == "#irccd");
    BOOST_TEST(cmd["mode"].get<std::string>() == "+t");
}

BOOST_AUTO_TEST_SUITE(errors)

BOOST_AUTO_TEST_CASE(invalid_identifier_1)
{
    boost::system::error_code result;
    nlohmann::json message;

    ctl_->send({
        { "command",    "server-mode"   },
        { "server",     123456          },
        { "channel",    "#music"        },
        { "mode",       "+i"            }
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
        { "command",    "server-mode"   },
        { "server",     ""              },
        { "channel",    "#music"        },
        { "mode",       "+i"            }
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
        { "command",    "server-mode"   },
        { "server",     "test"          },
        { "channel",    ""              },
        { "mode",       "+i"            }
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
        { "command",    "server-mode"   },
        { "server",     "test"          },
        { "channel",    123456          },
        { "mode",       "+i"            }
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

BOOST_AUTO_TEST_CASE(invalid_mode_1)
{
    boost::system::error_code result;
    nlohmann::json message;

    ctl_->send({
        { "command",    "server-mode"   },
        { "server",     "test"          },
        { "channel",    "#music"        },
        { "mode",       ""              }
    });
    ctl_->recv([&] (auto rresult, auto rmessage) {
        result = rresult;
        message = rmessage;
    });

    wait_for([&] {
        return result;
    });

    BOOST_TEST(result == server_error::invalid_mode);
    BOOST_TEST(message["error"].template get<int>() == server_error::invalid_mode);
    BOOST_TEST(message["errorCategory"].template get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(invalid_mode_2)
{
    boost::system::error_code result;
    nlohmann::json message;

    ctl_->send({
        { "command",    "server-mode"   },
        { "server",     "test"          },
        { "channel",    "#music"        },
        { "mode",       123456          }
    });
    ctl_->recv([&] (auto rresult, auto rmessage) {
        result = rresult;
        message = rmessage;
    });

    wait_for([&] {
        return result;
    });

    BOOST_TEST(result == server_error::invalid_mode);
    BOOST_TEST(message["error"].template get<int>() == server_error::invalid_mode);
    BOOST_TEST(message["errorCategory"].template get<std::string>() == "server");
}
BOOST_AUTO_TEST_CASE(not_found)
{
    boost::system::error_code result;
    nlohmann::json message;

    ctl_->send({
        { "command",    "server-mode"   },
        { "server",     "unknown"       },
        { "channel",    "#music"        },
        { "mode",       "+i"            }
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
