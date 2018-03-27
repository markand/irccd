/*
 * main.cpp -- test server-kick remote command
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

#define BOOST_TEST_MODULE "server-kick"
#include <boost/test/unit_test.hpp>

#include <irccd/daemon/command/server_kick_command.hpp>
#include <irccd/daemon/service/server_service.hpp>

#include <irccd/test/command_test.hpp>
#include <irccd/test/journal_server.hpp>

namespace irccd {

namespace {

class server_kick_test : public command_test<server_kick_command> {
protected:
    std::shared_ptr<journal_server> server_{new journal_server(service_, "test")};

    server_kick_test()
    {
        daemon_->servers().add(server_);
    }
};

} // !namespace

BOOST_FIXTURE_TEST_SUITE(server_kick_test_suite, server_kick_test)

BOOST_AUTO_TEST_CASE(basic)
{
    ctl_->send({
        { "command",    "server-kick"       },
        { "server",     "test"              },
        { "target",     "francis"           },
        { "channel",    "#staff"            },
        { "reason",     "too noisy"         }
    });

    wait_for([this] () {
        return !server_->cqueue().empty();
    });

    auto cmd = server_->cqueue().back();

    BOOST_TEST(cmd["command"].get<std::string>() == "kick");
    BOOST_TEST(cmd["channel"].get<std::string>() == "#staff");
    BOOST_TEST(cmd["target"].get<std::string>() == "francis");
    BOOST_TEST(cmd["reason"].get<std::string>() == "too noisy");
}

BOOST_AUTO_TEST_CASE(noreason)
{
    ctl_->send({
        { "command",    "server-kick"       },
        { "server",     "test"              },
        { "target",     "francis"           },
        { "channel",    "#staff"            }
    });

    wait_for([this] () {
        return !server_->cqueue().empty();
    });

    auto cmd = server_->cqueue().back();

    BOOST_TEST(cmd["command"].get<std::string>() == "kick");
    BOOST_TEST(cmd["channel"].get<std::string>() == "#staff");
    BOOST_TEST(cmd["target"].get<std::string>() == "francis");
    BOOST_TEST(cmd["reason"].get<std::string>() == "");
}

BOOST_AUTO_TEST_SUITE(errors)

BOOST_AUTO_TEST_CASE(invalid_identifier_1)
{
    const auto result = request({
        { "command",    "server-kick"   },
        { "server",     123456          },
        { "target",     "francis"       },
        { "channel",    "#music"        }
    });

    BOOST_TEST(result.second == server_error::invalid_identifier);
    BOOST_TEST(result.first["error"].template get<int>() == server_error::invalid_identifier);
    BOOST_TEST(result.first["errorCategory"].template get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(invalid_identifier_2)
{
    const auto result = request({
        { "command",    "server-kick"   },
        { "server",     ""              },
        { "target",     "francis"       },
        { "channel",    "#music"        }
    });

    BOOST_TEST(result.second == server_error::invalid_identifier);
    BOOST_TEST(result.first["error"].template get<int>() == server_error::invalid_identifier);
    BOOST_TEST(result.first["errorCategory"].template get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(invalid_nickname_1)
{
    const auto result = request({
        { "command",    "server-kick"   },
        { "server",     "test"          },
        { "target",     ""              },
        { "channel",    "#music"        }
    });

    BOOST_TEST(result.second == server_error::invalid_nickname);
    BOOST_TEST(result.first["error"].template get<int>() == server_error::invalid_nickname);
    BOOST_TEST(result.first["errorCategory"].template get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(invalid_nickname_2)
{
    const auto result = request({
        { "command",    "server-kick"   },
        { "server",     "test"          },
        { "target",     123456          },
        { "channel",    "#music"        }
    });

    BOOST_TEST(result.second == server_error::invalid_nickname);
    BOOST_TEST(result.first["error"].template get<int>() == server_error::invalid_nickname);
    BOOST_TEST(result.first["errorCategory"].template get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(invalid_channel_1)
{
    const auto result = request({
        { "command",    "server-kick"   },
        { "server",     "test"          },
        { "target",     "jean"          },
        { "channel",    ""              }
    });

    BOOST_TEST(result.second == server_error::invalid_channel);
    BOOST_TEST(result.first["error"].template get<int>() == server_error::invalid_channel);
    BOOST_TEST(result.first["errorCategory"].template get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(invalid_channel_2)
{
    const auto result = request({
        { "command",    "server-kick"   },
        { "server",     "test"          },
        { "target",     "jean"          },
        { "channel",    123456          }
    });

    BOOST_TEST(result.second == server_error::invalid_channel);
    BOOST_TEST(result.first["error"].template get<int>() == server_error::invalid_channel);
    BOOST_TEST(result.first["errorCategory"].template get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(invalid_message)
{
    const auto result = request({
        { "command",    "server-kick"   },
        { "server",     "test"          },
        { "target",     "jean"          },
        { "channel",    "#staff"        },
        { "reason",     123456          }
    });

    BOOST_TEST(result.second == server_error::invalid_message);
    BOOST_TEST(result.first["error"].template get<int>() == server_error::invalid_message);
    BOOST_TEST(result.first["errorCategory"].template get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(not_found)
{
    const auto result = request({
        { "command",    "server-kick"   },
        { "server",     "unknown"       },
        { "target",     "francis"       },
        { "channel",    "#music"        }
    });

    BOOST_TEST(result.second == server_error::not_found);
    BOOST_TEST(result.first["error"].template get<int>() == server_error::not_found);
    BOOST_TEST(result.first["errorCategory"].template get<std::string>() == "server");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

} // !irccd
