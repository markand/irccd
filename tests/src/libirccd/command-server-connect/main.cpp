/*
 * main.cpp -- test server-connect remote command
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

#define BOOST_TEST_MODULE "server-connect"
#include <boost/test/unit_test.hpp>

#include <irccd/daemon/command/server_connect_command.hpp>
#include <irccd/daemon/service/server_service.hpp>

#include <irccd/test/command_test.hpp>
#include <irccd/test/mock_server.hpp>

namespace irccd {

namespace {

BOOST_FIXTURE_TEST_SUITE(server_connect_test_suite, command_test<server_connect_command>)

BOOST_AUTO_TEST_CASE(minimal)
{
    const auto [json, code] = request({
        { "command",    "server-connect"    },
        { "name",       "local"             },
        { "host",       "irc.example.org"   }
    });

    const auto s = daemon_->servers().get("local");

    BOOST_TEST(!code);
    BOOST_TEST(s);
    BOOST_TEST(s->get_id() == "local");
    BOOST_TEST(s->get_host() == "irc.example.org");
    BOOST_TEST(s->get_port() == 6667U);
}

#if defined(IRCCD_HAVE_SSL)

BOOST_AUTO_TEST_CASE(full)
{
    const auto [json, code] = request({
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

    const auto s = daemon_->servers().get("local2");

    BOOST_TEST(!code);
    BOOST_TEST(s);
    BOOST_TEST(s->get_id() == "local2");
    BOOST_TEST(s->get_host() == "irc.example2.org");
    BOOST_TEST(s->get_port() == 18000U);
    BOOST_TEST(s->get_password() == "nonono");
    BOOST_TEST(s->get_nickname() == "francis");
    BOOST_TEST(s->get_realname() == "the_francis");
    BOOST_TEST(s->get_username() == "frc");
    BOOST_TEST(s->get_command_char() == "::");
    BOOST_TEST(s->get_ctcp_version() == "ultra bot");
    BOOST_TEST(static_cast<bool>(s->get_options() & server::options::ssl));
    BOOST_TEST(static_cast<bool>(s->get_options() & server::options::ssl_verify));
    BOOST_TEST(static_cast<bool>(s->get_options() & server::options::auto_rejoin));
    BOOST_TEST(static_cast<bool>(s->get_options() & server::options::join_invite));
}

#endif // !IRCCD_HAVE_SSL

BOOST_AUTO_TEST_SUITE(errors)

BOOST_AUTO_TEST_CASE(already_exists)
{
    daemon_->servers().add(std::make_unique<mock_server>(service_, "local"));

    const auto [json, code] = request({
        { "command",    "server-connect"    },
        { "name",       "local"             },
        { "host",       "127.0.0.1"         }
    });

    BOOST_TEST(code == server_error::already_exists);
    BOOST_TEST(json["error"].get<int>() == server_error::already_exists);
    BOOST_TEST(json["errorCategory"].get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(invalid_hostname_1)
{
    const auto [json, code] = request({
        { "command",    "server-connect"    },
        { "name",       "new"               },
    });

    BOOST_TEST(code == server_error::invalid_hostname);
    BOOST_TEST(json["error"].get<int>() == server_error::invalid_hostname);
    BOOST_TEST(json["errorCategory"].get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(invalid_hostname_2)
{
    const auto [json, code] = request({
        { "command",    "server-connect"    },
        { "name",       "new"               },
        { "host",       123456              }
    });

    BOOST_TEST(code == server_error::invalid_hostname);
    BOOST_TEST(json["error"].get<int>() == server_error::invalid_hostname);
    BOOST_TEST(json["errorCategory"].get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(invalid_identifier_1)
{
    const auto [json, code] = request({
        { "command",    "server-connect"    },
        { "name",       ""                  },
        { "host",       "127.0.0.1"         }
    });

    BOOST_TEST(code == server_error::invalid_identifier);
    BOOST_TEST(json["error"].get<int>() == server_error::invalid_identifier);
    BOOST_TEST(json["errorCategory"].get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(invalid_identifier_2)
{
    const auto [json, code] = request({
        { "command",    "server-connect"    },
        { "name",       123456              },
        { "host",       "127.0.0.1"         }
    });

    BOOST_TEST(code == server_error::invalid_identifier);
    BOOST_TEST(json["error"].get<int>() == server_error::invalid_identifier);
    BOOST_TEST(json["errorCategory"].get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(invalid_port_1)
{
    const auto [json, code] = request({
        { "command",    "server-connect"    },
        { "name",       "new"               },
        { "host",       "127.0.0.1"         },
        { "port",       "notaint"           }
    });

    BOOST_TEST(code == server_error::invalid_port);
    BOOST_TEST(json["error"].get<int>() == server_error::invalid_port);
    BOOST_TEST(json["errorCategory"].get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(invalid_port_2)
{
    const auto [json, code] = request({
        { "command",    "server-connect"    },
        { "name",       "new"               },
        { "host",       "127.0.0.1"         },
        { "port",       -123                }
    });

    BOOST_TEST(code == server_error::invalid_port);
    BOOST_TEST(json["error"].get<int>() == server_error::invalid_port);
    BOOST_TEST(json["errorCategory"].get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(invalid_port_3)
{
    const auto [json, code] = request({
        { "command",    "server-connect"    },
        { "name",       "new"               },
        { "host",       "127.0.0.1"         },
        { "port",       1000000             }
    });

    BOOST_TEST(code == server_error::invalid_port);
    BOOST_TEST(json["error"].get<int>() == server_error::invalid_port);
    BOOST_TEST(json["errorCategory"].get<std::string>() == "server");
}

#if !defined(IRCCD_HAVE_SSL)

BOOST_AUTO_TEST_CASE(ssl_disabled)
{
    const auto [json, code] = request({
        { "command",    "server-connect"    },
        { "name",       "new"               },
        { "host",       "127.0.0.1"         },
        { "ssl",        true                }
    });

    BOOST_TEST(code == server_error::ssl_disabled);
    BOOST_TEST(json["error"].get<int>() == server_error::ssl_disabled);
    BOOST_TEST(json["errorCategory"].get<std::string>() == "server");
}

#endif

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
