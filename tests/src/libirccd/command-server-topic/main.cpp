/*
 * main.cpp -- test server-topic remote command
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

#define BOOST_TEST_MODULE "server-topic"
#include <boost/test/unit_test.hpp>

#include <irccd/daemon/command/server_topic_command.hpp>
#include <irccd/daemon/service/server_service.hpp>

#include <irccd/test/command_test.hpp>
#include <irccd/test/journal_server.hpp>

namespace irccd {

namespace {

class server_topic_test : public command_test<server_topic_command> {
protected:
    std::shared_ptr<journal_server> server_{new journal_server(service_, "test")};

    server_topic_test()
    {
        daemon_->servers().add(server_);
    }
};

} // !namespace

BOOST_FIXTURE_TEST_SUITE(server_topic_test_suite, server_topic_test)

BOOST_AUTO_TEST_CASE(basic)
{
    ctl_->send({
        { "command",    "server-topic"  },
        { "server",     "test"          },
        { "channel",    "#staff"        },
        { "topic",      "new version"   }
    });

    wait_for([this] () {
        return !server_->cqueue().empty();
    });

    auto cmd = server_->cqueue().back();

    BOOST_TEST(cmd["command"].get<std::string>() == "topic");
    BOOST_TEST(cmd["channel"].get<std::string>() == "#staff");
    BOOST_TEST(cmd["topic"].get<std::string>() == "new version");
}

BOOST_AUTO_TEST_SUITE(errors)

BOOST_AUTO_TEST_CASE(invalid_identifier_1)
{
    const auto result = request({
        { "command",    "server-topic"  },
        { "server",     123456          },
        { "channel",    "#music"        },
        { "topic",      "plop"          }
    });

    BOOST_TEST(result.second == server_error::invalid_identifier);
    BOOST_TEST(result.first["error"].template get<int>() == server_error::invalid_identifier);
    BOOST_TEST(result.first["errorCategory"].template get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(invalid_identifier_2)
{
    const auto result = request({
        { "command",    "server-topic"  },
        { "server",     ""              },
        { "channel",    "#music"        },
        { "topic",      "plop"          }
    });

    BOOST_TEST(result.second == server_error::invalid_identifier);
    BOOST_TEST(result.first["error"].template get<int>() == server_error::invalid_identifier);
    BOOST_TEST(result.first["errorCategory"].template get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(invalid_channel_1)
{
    const auto result = request({
        { "command",    "server-topic"  },
        { "server",     "test"          },
        { "channel",    ""              },
        { "topic",      "plop"          }
    });

    BOOST_TEST(result.second == server_error::invalid_channel);
    BOOST_TEST(result.first["error"].template get<int>() == server_error::invalid_channel);
    BOOST_TEST(result.first["errorCategory"].template get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(invalid_channel_2)
{
    const auto result = request({
        { "command",    "server-topic"  },
        { "server",     "test"          },
        { "channel",    123456          },
        { "topic",      "plop"          }
    });

    BOOST_TEST(result.second == server_error::invalid_channel);
    BOOST_TEST(result.first["error"].template get<int>() == server_error::invalid_channel);
    BOOST_TEST(result.first["errorCategory"].template get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(not_found)
{
    const auto result = request({
        { "command",    "server-topic"  },
        { "server",     "unknown"       },
        { "channel",    "#music"        },
        { "topic",      "plop"          }
    });

    BOOST_TEST(result.second == server_error::not_found);
    BOOST_TEST(result.first["error"].template get<int>() == server_error::not_found);
    BOOST_TEST(result.first["errorCategory"].template get<std::string>() == "server");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

} // !irccd