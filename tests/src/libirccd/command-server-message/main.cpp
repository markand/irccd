/*
 * main.cpp -- test server-message remote command
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

#define BOOST_TEST_MODULE "server-message"
#include <boost/test/unit_test.hpp>

#include <irccd/daemon/command/server_message_command.hpp>
#include <irccd/daemon/service/server_service.hpp>

#include <irccd/test/command_test.hpp>
#include <irccd/test/mock_server.hpp>

namespace irccd {

namespace {

class server_message_test : public command_test<server_message_command> {
protected:
    std::shared_ptr<mock_server> server_;

    server_message_test()
        : server_(new mock_server(service_, "test"))
    {
        daemon_->servers().add(server_);
        server_->clear();
    }
};

BOOST_FIXTURE_TEST_SUITE(server_message_test_suite, server_message_test)

BOOST_AUTO_TEST_CASE(basic)
{
    const auto result = request({
        { "command",    "server-message"    },
        { "server",     "test"              },
        { "target",     "#staff"            },
        { "message",    "plop!"             }
    });

    const auto cmd = server_->find("message").back();

    BOOST_TEST(std::any_cast<std::string>(cmd[1]) == "plop!");
    BOOST_TEST(std::any_cast<std::string>(cmd[0]) == "#staff");
}

BOOST_AUTO_TEST_SUITE(errors)

BOOST_AUTO_TEST_CASE(invalid_identifier_1)
{
    const auto result = request({
        { "command",    "server-message"    },
        { "server",     123456              },
        { "target",     "#music"            },
        { "message",    "plop!"             }
    });

    BOOST_TEST(result.second == server_error::invalid_identifier);
    BOOST_TEST(result.first["error"].template get<int>() == server_error::invalid_identifier);
    BOOST_TEST(result.first["errorCategory"].template get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(invalid_identifier_2)
{
    const auto result = request({
        { "command",    "server-message"    },
        { "server",     ""                  },
        { "target",     "#music"            },
        { "message",    "plop!"             }
    });

    BOOST_TEST(result.second == server_error::invalid_identifier);
    BOOST_TEST(result.first["error"].template get<int>() == server_error::invalid_identifier);
    BOOST_TEST(result.first["errorCategory"].template get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(invalid_channel_1)
{
    const auto result = request({
        { "command",    "server-message"    },
        { "server",     "test"              },
        { "target",     ""                  },
        { "message",    "plop!"             }
    });

    BOOST_TEST(result.second == server_error::invalid_channel);
    BOOST_TEST(result.first["error"].template get<int>() == server_error::invalid_channel);
    BOOST_TEST(result.first["errorCategory"].template get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(invalid_channel_2)
{
    const auto result = request({
        { "command",    "server-message"    },
        { "server",     "test"              },
        { "target",     123456              },
        { "message",    "plop!"             }
    });

    BOOST_TEST(result.second == server_error::invalid_channel);
    BOOST_TEST(result.first["error"].template get<int>() == server_error::invalid_channel);
    BOOST_TEST(result.first["errorCategory"].template get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(not_found)
{
    const auto result = request({
        { "command",    "server-message"    },
        { "server",     "unknown"           },
        { "target",     "#music"            },
        { "message",    "plop!"             }
    });

    BOOST_TEST(result.second == server_error::not_found);
    BOOST_TEST(result.first["error"].template get<int>() == server_error::not_found);
    BOOST_TEST(result.first["errorCategory"].template get<std::string>() == "server");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
