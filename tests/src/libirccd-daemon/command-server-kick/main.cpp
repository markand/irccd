/*
 * main.cpp -- test server-kick remote command
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

#define BOOST_TEST_MODULE "server-kick"
#include <boost/test/unit_test.hpp>

#include <irccd/test/command_fixture.hpp>

using irccd::test::command_fixture;

using irccd::daemon::server;
using irccd::daemon::server_error;

namespace irccd {

namespace {

BOOST_FIXTURE_TEST_SUITE(server_kick_fixture_suite, command_fixture)

BOOST_AUTO_TEST_CASE(basic)
{
	const auto [json, code] = request({
		{ "command",    "server-kick"   },
		{ "server",     "test"          },
		{ "target",     "francis"       },
		{ "channel",    "#staff"        },
		{ "reason",     "too noisy"     }
	});

	const auto cmd = server_->find("kick").back();

	BOOST_TEST(!code);
	BOOST_TEST(std::any_cast<std::string>(cmd[1]) == "#staff");
	BOOST_TEST(std::any_cast<std::string>(cmd[0]) == "francis");
	BOOST_TEST(std::any_cast<std::string>(cmd[2]) == "too noisy");
}

BOOST_AUTO_TEST_CASE(noreason)
{
	const auto [json, code] = request({
		{ "command",    "server-kick"   },
		{ "server",     "test"          },
		{ "target",     "francis"       },
		{ "channel",    "#staff"        }
	});

	const auto cmd = server_->find("kick").back();

	BOOST_TEST(!code);
	BOOST_TEST(std::any_cast<std::string>(cmd[1]) == "#staff");
	BOOST_TEST(std::any_cast<std::string>(cmd[0]) == "francis");
	BOOST_TEST(std::any_cast<std::string>(cmd[2]) == "");
}

BOOST_AUTO_TEST_SUITE(errors)

BOOST_AUTO_TEST_CASE(invalid_identifier_1)
{
	const auto [json, code] = request({
		{ "command",    "server-kick"   },
		{ "server",     123456          },
		{ "target",     "francis"       },
		{ "channel",    "#music"        }
	});

	BOOST_TEST(code == server_error::invalid_identifier);
	BOOST_TEST(json["error"].get<int>() == server_error::invalid_identifier);
	BOOST_TEST(json["errorCategory"].get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(invalid_identifier_2)
{
	const auto [json, code] = request({
		{ "command",    "server-kick"   },
		{ "server",     ""              },
		{ "target",     "francis"       },
		{ "channel",    "#music"        }
	});

	BOOST_TEST(code == server_error::invalid_identifier);
	BOOST_TEST(json["error"].get<int>() == server_error::invalid_identifier);
	BOOST_TEST(json["errorCategory"].get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(invalid_nickname_1)
{
	const auto [json, code] = request({
		{ "command",    "server-kick"   },
		{ "server",     "test"          },
		{ "target",     ""              },
		{ "channel",    "#music"        }
	});

	BOOST_TEST(code == server_error::invalid_nickname);
	BOOST_TEST(json["error"].get<int>() == server_error::invalid_nickname);
	BOOST_TEST(json["errorCategory"].get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(invalid_nickname_2)
{
	const auto [json, code] = request({
		{ "command",    "server-kick"   },
		{ "server",     "test"          },
		{ "target",     123456          },
		{ "channel",    "#music"        }
	});

	BOOST_TEST(code == server_error::invalid_nickname);
	BOOST_TEST(json["error"].get<int>() == server_error::invalid_nickname);
	BOOST_TEST(json["errorCategory"].get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(invalid_channel_1)
{
	const auto [json, code] = request({
		{ "command",    "server-kick"   },
		{ "server",     "test"          },
		{ "target",     "jean"          },
		{ "channel",    ""              }
	});

	BOOST_TEST(code == server_error::invalid_channel);
	BOOST_TEST(json["error"].get<int>() == server_error::invalid_channel);
	BOOST_TEST(json["errorCategory"].get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(invalid_channel_2)
{
	const auto [json, code] = request({
		{ "command",    "server-kick"   },
		{ "server",     "test"          },
		{ "target",     "jean"          },
		{ "channel",    123456          }
	});

	BOOST_TEST(code == server_error::invalid_channel);
	BOOST_TEST(json["error"].get<int>() == server_error::invalid_channel);
	BOOST_TEST(json["errorCategory"].get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(invalid_message)
{
	const auto [json, code] = request({
		{ "command",    "server-kick"   },
		{ "server",     "test"          },
		{ "target",     "jean"          },
		{ "channel",    "#staff"        },
		{ "reason",     123456          }
	});

	BOOST_TEST(code == server_error::invalid_message);
	BOOST_TEST(json["error"].get<int>() == server_error::invalid_message);
	BOOST_TEST(json["errorCategory"].get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(not_found)
{
	const auto [json, code] = request({
		{ "command",    "server-kick"   },
		{ "server",     "unknown"       },
		{ "target",     "francis"       },
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
