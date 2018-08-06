/*
 * main.cpp -- test server-nick remote command
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

#define BOOST_TEST_MODULE "server-nick"
#include <boost/test/unit_test.hpp>

#include <irccd/test/command_fixture.hpp>

namespace irccd::test {

namespace {

BOOST_FIXTURE_TEST_SUITE(server_nick_fixture_suite, command_fixture)

BOOST_AUTO_TEST_CASE(basic)
{
    const auto [json, code] = request({
        { "command",    "server-nick"   },
        { "server",     "test"          },
        { "nickname",   "chris"         }
    });

    BOOST_TEST(!code);
    BOOST_TEST(json.is_object());
    BOOST_TEST(server_->get_nickname() == "chris");
}

BOOST_AUTO_TEST_SUITE(errors)

BOOST_AUTO_TEST_CASE(invalid_identifier_1)
{
    const auto [json, code] = request({
        { "command",    "server-nick"   },
        { "server",     123456          },
        { "nickname",   "chris"         }
    });

    BOOST_TEST(code == server_error::invalid_identifier);
    BOOST_TEST(json["error"].get<int>() == server_error::invalid_identifier);
    BOOST_TEST(json["errorCategory"].get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(invalid_identifier_2)
{
    const auto [json, code] = request({
        { "command",    "server-nick"   },
        { "server",     ""              },
        { "nickname",   "chris"         }
    });

    BOOST_TEST(code == server_error::invalid_identifier);
    BOOST_TEST(json["error"].get<int>() == server_error::invalid_identifier);
    BOOST_TEST(json["errorCategory"].get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(invalid_nickname_1)
{
    const auto [json, code] = request({
        { "command",    "server-nick"   },
        { "server",     "test"          },
        { "nickname",   ""              }
    });

    BOOST_TEST(code == server_error::invalid_nickname);
    BOOST_TEST(json["error"].get<int>() == server_error::invalid_nickname);
    BOOST_TEST(json["errorCategory"].get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(invalid_nickname_2)
{
    const auto [json, code] = request({
        { "command",    "server-nick"   },
        { "server",     "test"          },
        { "nickname",   123456          }
    });

    BOOST_TEST(code == server_error::invalid_nickname);
    BOOST_TEST(json["error"].get<int>() == server_error::invalid_nickname);
    BOOST_TEST(json["errorCategory"].get<std::string>() == "server");
}
BOOST_AUTO_TEST_CASE(not_found)
{
    const auto [json, code] = request({
        { "command",    "server-nick"   },
        { "server",     "unknown"       },
        { "nickname",   "chris"         }
    });

    BOOST_TEST(code == server_error::not_found);
    BOOST_TEST(json["error"].get<int>() == server_error::not_found);
    BOOST_TEST(json["errorCategory"].get<std::string>() == "server");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd::test
