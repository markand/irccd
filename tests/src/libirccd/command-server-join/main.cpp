/*
 * main.cpp -- test server-join remote command
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

#define BOOST_TEST_MODULE "server-join"
#include <boost/test/unit_test.hpp>

#include <irccd/daemon/server_service.hpp>

#include <irccd/test/command_test.hpp>
#include <irccd/test/mock_server.hpp>

namespace irccd {

namespace {

class server_join_test : public command_test<server_join_command> {
protected:
    std::shared_ptr<mock_server> server_;

    server_join_test()
        : server_(new mock_server(service_, "test", "localhost"))
    {
        daemon_->servers().add(server_);
        server_->clear();
    }
};

BOOST_FIXTURE_TEST_SUITE(server_join_test_suite, server_join_test)

BOOST_AUTO_TEST_CASE(basic)
{
    const auto [json, code] = request({
        { "command",    "server-join"       },
        { "server",     "test"              },
        { "channel",    "#music"            },
        { "password",   "plop"              }
    });

    const auto cmd = server_->find("join").back();

    BOOST_TEST(!code);
    BOOST_TEST(std::any_cast<std::string>(cmd[0]) == "#music");
    BOOST_TEST(std::any_cast<std::string>(cmd[1]) == "plop");
}

BOOST_AUTO_TEST_CASE(nopassword)
{
    const auto [json, code] = request({
        { "command",    "server-join"       },
        { "server",     "test"              },
        { "channel",    "#music"            }
    });

    const auto cmd = server_->find("join").back();

    BOOST_TEST(!code);
    BOOST_TEST(std::any_cast<std::string>(cmd[0]) == "#music");
    BOOST_TEST(std::any_cast<std::string>(cmd[1]) == "");
}

BOOST_AUTO_TEST_SUITE(errors)

BOOST_AUTO_TEST_CASE(invalid_identifier_1)
{
    const auto [json, code] = request({
        { "command",    "server-join"   },
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
        { "command",    "server-join"   },
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
        { "command",    "server-join"   },
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
        { "command",    "server-join"   },
        { "server",     "test"          },
        { "channel",    123456          }
    });

    BOOST_TEST(code == server_error::invalid_channel);
    BOOST_TEST(json["error"].get<int>() == server_error::invalid_channel);
    BOOST_TEST(json["errorCategory"].get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(invalid_password)
{
    const auto [json, code] = request({
        { "command",    "server-join"   },
        { "server",     "test"          },
        { "channel",    "#staff"        },
        { "password",   123456          }
    });

    BOOST_TEST(code == server_error::invalid_password);
    BOOST_TEST(json["error"].get<int>() == server_error::invalid_password);
    BOOST_TEST(json["errorCategory"].get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(not_found)
{
    const auto [json, code] = request({
        { "command",    "server-join"   },
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
