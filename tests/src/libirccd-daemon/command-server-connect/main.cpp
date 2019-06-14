/*
 * main.cpp -- test server-connect remote command
 *
 * Copyright (c) 2013-2019 David Demelier <markand@malikania.fr>
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

#include <irccd/test/command_fixture.hpp>

namespace irccd {

namespace {

BOOST_FIXTURE_TEST_SUITE(server_connect_fixture_suite, test::command_fixture)

BOOST_AUTO_TEST_CASE(minimal)
{
	const auto json = request({
		{ "command",    "server-connect"    },
		{ "name",       "local"             },
		{ "hostname",   "irc.example.org"   }
	});

	const auto s = bot_.get_servers().get("local");

	BOOST_TEST(json.size() == 1U);
	BOOST_TEST(json["command"].get<std::string>() == "server-connect");
	BOOST_TEST(s);
	BOOST_TEST(s->get_id() == "local");
	BOOST_TEST(s->get_hostname() == "irc.example.org");
	BOOST_TEST(s->get_port() == 6667U);
}

#if defined(IRCCD_HAVE_SSL)

BOOST_AUTO_TEST_CASE(full)
{
	const auto json = request({
		{ "command",            "server-connect"        },
		{ "name",               "local2"                },
		{ "hostname",           "irc.example2.org"      },
		{ "password",           "nonono"                },
		{ "nickname",           "francis"               },
		{ "realname",           "the_francis"           },
		{ "username",           "frc"                   },
		{ "ipv4",               false                   },
		{ "ipv6",               true                    },
		{ "ctcpVersion",        "ultra bot"             },
		{ "commandChar",        "::"                    },
		{ "port",               18000U                  },
		{ "ssl",                true                    },
		{ "sslVerify",          true                    },
		{ "autoRejoin",         true                    },
		{ "joinInvite",         true                    }
	});

	const auto s = bot_.get_servers().get("local2");

	BOOST_TEST(json.size() == 1U);
	BOOST_TEST(json["command"].get<std::string>() == "server-connect");
	BOOST_TEST(s);
	BOOST_TEST(s->get_id() == "local2");
	BOOST_TEST(s->get_hostname() == "irc.example2.org");
	BOOST_TEST(s->get_port() == 18000U);
	BOOST_TEST(s->get_password() == "nonono");
	BOOST_TEST(s->get_nickname() == "francis");
	BOOST_TEST(s->get_realname() == "the_francis");
	BOOST_TEST(s->get_username() == "frc");
	BOOST_TEST(s->get_command_char() == "::");
	BOOST_TEST(s->get_ctcp_version() == "ultra bot");
	BOOST_TEST(!static_cast<bool>(s->get_options() & daemon::server::options::ipv4));
	BOOST_TEST(static_cast<bool>(s->get_options() & daemon::server::options::ipv6));
	BOOST_TEST(static_cast<bool>(s->get_options() & daemon::server::options::ssl));
	BOOST_TEST(static_cast<bool>(s->get_options() & daemon::server::options::auto_rejoin));
	BOOST_TEST(static_cast<bool>(s->get_options() & daemon::server::options::join_invite));
}

#endif // !IRCCD_HAVE_SSL

BOOST_AUTO_TEST_SUITE(errors)

BOOST_AUTO_TEST_CASE(already_exists)
{
	bot_.get_servers().add(std::make_unique<test::mock_server>(ctx_, "local"));

	const auto json = request({
		{ "command",    "server-connect"        },
		{ "name",       "local"                 },
		{ "hostname",   "127.0.0.1"             }
	});

	BOOST_TEST(json.size() == 4U);
	BOOST_TEST(json["command"].get<std::string>() == "server-connect");
	BOOST_TEST(json["error"].get<int>() == daemon::server_error::already_exists);
	BOOST_TEST(json["errorCategory"].get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(invalid_hostname_1)
{
	const auto json = request({
		{ "command",    "server-connect"        },
		{ "name",       "new"                   },
	});

	BOOST_TEST(json.size() == 4U);
	BOOST_TEST(json["command"].get<std::string>() == "server-connect");
	BOOST_TEST(json["error"].get<int>() == daemon::server_error::invalid_hostname);
	BOOST_TEST(json["errorCategory"].get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(invalid_hostname_2)
{
	const auto json = request({
		{ "command",    "server-connect"        },
		{ "name",       "new"                   },
		{ "hostname",   123456                  }
	});

	BOOST_TEST(json.size() == 4U);
	BOOST_TEST(json["command"].get<std::string>() == "server-connect");
	BOOST_TEST(json["error"].get<int>() == daemon::server_error::invalid_hostname);
	BOOST_TEST(json["errorCategory"].get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(invalid_identifier_1)
{
	const auto json = request({
		{ "command",    "server-connect"        },
		{ "name",       ""                      },
		{ "hostname",   "127.0.0.1"             }
	});

	BOOST_TEST(json.size() == 4U);
	BOOST_TEST(json["command"].get<std::string>() == "server-connect");
	BOOST_TEST(json["error"].get<int>() == daemon::server_error::invalid_identifier);
	BOOST_TEST(json["errorCategory"].get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(invalid_identifier_2)
{
	const auto json = request({
		{ "command",    "server-connect"        },
		{ "name",       123456                  },
		{ "hostname",   "127.0.0.1"             }
	});

	BOOST_TEST(json.size() == 4U);
	BOOST_TEST(json["command"].get<std::string>() == "server-connect");
	BOOST_TEST(json["error"].get<int>() == daemon::server_error::invalid_identifier);
	BOOST_TEST(json["errorCategory"].get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(invalid_port_1)
{
	const auto json = request({
		{ "command",    "server-connect"        },
		{ "name",       "new"                   },
		{ "hostname",   "127.0.0.1"             },
		{ "port",       "notaint"               }
	});

	BOOST_TEST(json.size() == 4U);
	BOOST_TEST(json["command"].get<std::string>() == "server-connect");
	BOOST_TEST(json["error"].get<int>() == daemon::server_error::invalid_port);
	BOOST_TEST(json["errorCategory"].get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(invalid_port_2)
{
	const auto json = request({
		{ "command",    "server-connect"        },
		{ "name",       "new"                   },
		{ "hostname",   "127.0.0.1"             },
		{ "port",       -123                    }
	});

	BOOST_TEST(json.size() == 4U);
	BOOST_TEST(json["command"].get<std::string>() == "server-connect");
	BOOST_TEST(json["error"].get<int>() == daemon::server_error::invalid_port);
	BOOST_TEST(json["errorCategory"].get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(invalid_port_3)
{
	const auto json = request({
		{ "command",    "server-connect"        },
		{ "name",       "new"                   },
		{ "hostname",   "127.0.0.1"             },
		{ "port",       1000000U                }
	});

	BOOST_TEST(json.size() == 4U);
	BOOST_TEST(json["command"].get<std::string>() == "server-connect");
	BOOST_TEST(json["error"].get<int>() == daemon::server_error::invalid_port);
	BOOST_TEST(json["errorCategory"].get<std::string>() == "server");
}

#if !defined(IRCCD_HAVE_SSL)

BOOST_AUTO_TEST_CASE(ssl_disabled)
{
	const auto json = request({
		{ "command",    "server-connect"        },
		{ "name",       "new"                   },
		{ "hostname",   "127.0.0.1"             },
		{ "ssl",        true                    }
	});

	BOOST_TEST(json.size() == 4U);
	BOOST_TEST(json["command"].get<std::string>() == "server-connect");
	BOOST_TEST(json["error"].get<int>() == daemon::server_error::ssl_disabled);
	BOOST_TEST(json["errorCategory"].get<std::string>() == "server");
}

#endif

BOOST_AUTO_TEST_CASE(invalid_family_1)
{
	const auto json = request({
		{ "command",    "server-connect"        },
		{ "name",       "new"                   },
		{ "hostname",   "127.0.0.1"             },
		{ "port",       6667U                   },
		{ "ipv4",       "invalid"               }
	});

	BOOST_TEST(json.size() == 4U);
	BOOST_TEST(json["command"].get<std::string>() == "server-connect");
	BOOST_TEST(json["error"].get<int>() == daemon::server_error::invalid_family);
	BOOST_TEST(json["errorCategory"].get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(invalid_family_2)
{
	const auto json = request({
		{ "command",    "server-connect"        },
		{ "name",       "new"                   },
		{ "hostname",   "127.0.0.1"             },
		{ "port",       6667U                   },
		{ "ipv6",       1234                    }
	});

	BOOST_TEST(json.size() == 4U);
	BOOST_TEST(json["command"].get<std::string>() == "server-connect");
	BOOST_TEST(json["error"].get<int>() == daemon::server_error::invalid_family);
	BOOST_TEST(json["errorCategory"].get<std::string>() == "server");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
