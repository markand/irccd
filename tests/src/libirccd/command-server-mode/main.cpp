/*
 * main.cpp -- test server-mode remote command
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

#define BOOST_TEST_MODULE "server-mode"
#include <boost/test/unit_test.hpp>

#include <irccd/test/command_fixture.hpp>

using namespace irccd::test;

namespace irccd {

namespace {

BOOST_FIXTURE_TEST_SUITE(server_mode_fixture_suite, command_fixture)

BOOST_AUTO_TEST_CASE(basic)
{
	const auto [json, code] = request({
		{ "command",    "server-mode"   },
		{ "server",     "test"          },
		{ "channel",    "#irccd"        },
		{ "mode",       "+t"            }
	});

	const auto cmd = server_->find("mode").back();

	BOOST_TEST(!code);
	BOOST_TEST(std::any_cast<std::string>(cmd[0]) == "#irccd");
	BOOST_TEST(std::any_cast<std::string>(cmd[1]) == "+t");
}

BOOST_AUTO_TEST_SUITE(errors)

BOOST_AUTO_TEST_CASE(invalid_identifier_1)
{
	const auto [json, code] = request({
		{ "command",    "server-mode"   },
		{ "server",     123456          },
		{ "channel",    "#music"        },
		{ "mode",       "+i"            }
	});

	BOOST_TEST(code == server_error::invalid_identifier);
	BOOST_TEST(json["error"].get<int>() == server_error::invalid_identifier);
	BOOST_TEST(json["errorCategory"].get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(invalid_identifier_2)
{
	const auto [json, code] = request({
		{ "command",    "server-mode"   },
		{ "server",     ""              },
		{ "channel",    "#music"        },
		{ "mode",       "+i"            }
	});

	BOOST_TEST(code == server_error::invalid_identifier);
	BOOST_TEST(json["error"].get<int>() == server_error::invalid_identifier);
	BOOST_TEST(json["errorCategory"].get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(invalid_channel_1)
{
	const auto [json, code] = request({
		{ "command",    "server-mode"   },
		{ "server",     "test"          },
		{ "channel",    ""              },
		{ "mode",       "+i"            }
	});

	BOOST_TEST(code == server_error::invalid_channel);
	BOOST_TEST(json["error"].get<int>() == server_error::invalid_channel);
	BOOST_TEST(json["errorCategory"].get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(invalid_channel_2)
{
	const auto [json, code] = request({
		{ "command",    "server-mode"   },
		{ "server",     "test"          },
		{ "channel",    123456          },
		{ "mode",       "+i"            }
	});

	BOOST_TEST(code == server_error::invalid_channel);
	BOOST_TEST(json["error"].get<int>() == server_error::invalid_channel);
	BOOST_TEST(json["errorCategory"].get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(invalid_mode_1)
{
	const auto [json, code] = request({
		{ "command",    "server-mode"   },
		{ "server",     "test"          },
		{ "channel",    "#music"        },
		{ "mode",       ""              }
	});

	BOOST_TEST(code == server_error::invalid_mode);
	BOOST_TEST(json["error"].get<int>() == server_error::invalid_mode);
	BOOST_TEST(json["errorCategory"].get<std::string>() == "server");
}

BOOST_AUTO_TEST_CASE(invalid_mode_2)
{
	const auto [json, code] = request({
		{ "command",    "server-mode"   },
		{ "server",     "test"          },
		{ "channel",    "#music"        },
		{ "mode",       123456          }
	});

	BOOST_TEST(code == server_error::invalid_mode);
	BOOST_TEST(json["error"].get<int>() == server_error::invalid_mode);
	BOOST_TEST(json["errorCategory"].get<std::string>() == "server");
}
BOOST_AUTO_TEST_CASE(not_found)
{
	const auto [json, code] = request({
		{ "command",    "server-mode"   },
		{ "server",     "unknown"       },
		{ "channel",    "#music"        },
		{ "mode",       "+i"            }
	});

	BOOST_TEST(code == server_error::not_found);
	BOOST_TEST(json["error"].get<int>() == server_error::not_found);
	BOOST_TEST(json["errorCategory"].get<std::string>() == "server");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
