/*
 * main.cpp -- test server-connect remote command
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

#define BOOST_TEST_MODULE "server-connect"
#include <boost/test/unit_test.hpp>

#include <irccd/server_service.hpp>

#include <command_test.hpp>

namespace irccd {

BOOST_FIXTURE_TEST_SUITE(server_connect_test_suite, command_test<server_connect_command>)

BOOST_AUTO_TEST_CASE(minimal)
{
    nlohmann::json result;

    ctl_->send({
        { "command",    "server-connect"    },
        { "name",       "local"             },
        { "host",       "irc.example.org"   }
    });
    ctl_->recv([&] (auto, auto msg) {
        result = msg;
    });

    wait_for([&] () {
        return result.is_object();
    });

    auto s = daemon_->servers().get("local");

    BOOST_TEST(s);
    BOOST_TEST(s->name() == "local");
    BOOST_TEST(s->host() == "irc.example.org");
    BOOST_TEST(s->port() == 6667U);
}

BOOST_AUTO_TEST_CASE(full)
{
    nlohmann::json result;

    ctl_->send({
        { "command",    "server-connect"    },
        { "name",       "local2"            },
        { "host",       "irc.example2.org"  },
        { "password",   "nonono"            },
        { "nickname",   "francis"           },
        { "realname",   "the_francis"       },
        { "username",   "frc"               },
        { "ctcpVersion", "ultra bot"        },
        { "commandChar", "::"               },
        { "port",       18000               },
        { "ssl",        true                },
        { "sslVerify",  true                },
        { "autoRejoin", true                },
        { "joinInvite", true                }
    });
    ctl_->recv([&] (auto, auto msg) {
        result = msg;
    });

    wait_for([&] () {
        return result.is_object();
    });

    auto s = daemon_->servers().get("local2");

    BOOST_TEST(s);
    BOOST_TEST(s->name() == "local2");
    BOOST_TEST(s->host() == "irc.example2.org");
    BOOST_TEST(s->port() == 18000U);
    BOOST_TEST(s->password() == "nonono");
    BOOST_TEST(s->nickname() == "francis");
    BOOST_TEST(s->realname() == "the_francis");
    BOOST_TEST(s->username() == "frc");
    BOOST_TEST(s->command_char() == "::");
    BOOST_TEST(s->ctcp_version() == "ultra bot");
    BOOST_TEST(s->flags() & server::ssl);
    BOOST_TEST(s->flags() & server::ssl_verify);
    BOOST_TEST(s->flags() & server::auto_rejoin);
    BOOST_TEST(s->flags() & server::join_invite);
}

BOOST_AUTO_TEST_SUITE_END()

} // !irccd
