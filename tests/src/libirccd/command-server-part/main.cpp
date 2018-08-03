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

#include <irccd/daemon/service/server_service.hpp>

#include <irccd/test/command_test.hpp>
#include <irccd/test/mock_server.hpp>

namespace irccd {

namespace {

class server_part_test : public command_test<server_part_command> {
protected:
    std::shared_ptr<mock_server> server_;

    server_part_test()
        : server_(new mock_server(service_, "test"))
    {
        daemon_->servers().add(server_);
        server_->clear();
    }
};

BOOST_FIXTURE_TEST_SUITE(server_part_test_suite, server_part_test)

BOOST_AUTO_TEST_CASE(basic)
{
    const auto [json, code] = request({
        { "command",    "server-part"   },
        { "server",     "test"          },
        { "channel",    "#staff"        },
        { "reason",     "too noisy"     }
    });

    const auto cmd = server_->find("part").back();

    BOOST_TEST(!code);
    BOOST_TEST(std::any_cast<std::string>(cmd[0]) == "#staff");
    BOOST_TEST(std::any_cast<std::string>(cmd[1]) == "too noisy");
}

BOOST_AUTO_TEST_CASE(noreason)
{
    const auto [json, code] = request({
        { "command",    "server-part"   },
        { "server",     "test"          },
        { "channel",    "#staff"        }
    });

    const auto cmd = server_->find("part").back();

    BOOST_TEST(!code);
    BOOST_TEST(std::any_cast<std::string>(cmd[0]) == "#staff");
    BOOST_TEST(std::any_cast<std::string>(cmd[1]) == "");
}

BOOST_AUTO_TEST_SUITE(errors)

BOOST_AUTO_TEST_CASE(invalid_identifier_1)
{
    const auto [json, code] = request({
        { "command",    "server-part"   },
        { "server",     123456          },
        { "channel",    "#music"        }
    });

    BOOST_TEST(code == server_error::invalid_identifier);
    BOOST_TEST(json["error"].get<int>() == server_error::invalid_identifier);
    BOOST_TEST(json["errorCategory"].get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(invalid_identifier_2)
{
    const auto [json, code] = request({
        { "command",    "server-part"   },
        { "server",     ""              },
        { "channel",    "#music"        }
    });

    BOOST_TEST(code == server_error::invalid_identifier);
    BOOST_TEST(json["error"].get<int>() == server_error::invalid_identifier);
    BOOST_TEST(json["errorCategory"].get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(invalid_channel_1)
{
    const auto [json, code] = request({
        { "command",    "server-part"   },
        { "server",     "test"          },
        { "channel",    ""              }
    });

    BOOST_TEST(code == server_error::invalid_channel);
    BOOST_TEST(json["error"].get<int>() == server_error::invalid_channel);
    BOOST_TEST(json["errorCategory"].get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(invalid_channel_2)
{
    const auto [json, code] = request({
        { "command",    "server-part"   },
        { "server",     "test"          },
        { "channel",    123456          }
    });

    BOOST_TEST(code == server_error::invalid_channel);
    BOOST_TEST(json["error"].get<int>() == server_error::invalid_channel);
    BOOST_TEST(json["errorCategory"].get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(not_found)
{
    const auto [json, code] = request({
        { "command",    "server-part"   },
        { "server",     "unknown"       },
        { "channel",    "#music"        }
    });

    BOOST_TEST(code == server_error::not_found);
    BOOST_TEST(json["error"].get<int>() == server_error::not_found);
    BOOST_TEST(json["errorCategory"].get<std::string>() == "server");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
