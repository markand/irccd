/*
 * main.cpp -- test server_util functions
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

#define BOOST_TEST_MODULE "server_util"
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>

#include <irccd/ini.hpp>

#include <irccd/daemon/server.hpp>
#include <irccd/daemon/server_util.hpp>

using nlohmann::json;

using irccd::daemon::server;
using irccd::daemon::server_util::from_config;
using irccd::daemon::server_util::from_json;
using irccd::daemon::server_util::message_type;

namespace irccd {

namespace server_util {

auto operator<<(std::ostream& out, message_type::kind kind) -> std::ostream&
{
	if (kind == message_type::is_command)
		out << "command";
	else
		out << "message";

	return out;
}

} // !server_util

namespace {

class fixture {
protected:
	boost::asio::io_service ctx_;
};

auto open_config(const std::string& config) -> ini::document
{
	boost::filesystem::path path;

	path /= CMAKE_CURRENT_SOURCE_DIR;
	path /= config;

	return ini::read_file(path.string());
}

auto open_json(const std::string& file) -> json
{
	boost::filesystem::path path;

	path /= CMAKE_CURRENT_SOURCE_DIR;
	path /= file;

	std::ifstream input(path.string());

	if (!input)
		throw std::runtime_error(std::strerror(errno));

	return json::parse(std::string(std::istreambuf_iterator<char>(input.rdbuf()), {}));
}

BOOST_FIXTURE_TEST_SUITE(load_from_config, fixture)

BOOST_AUTO_TEST_SUITE(valid)

BOOST_AUTO_TEST_CASE(full)
{
	const auto sv = from_config(ctx_, open_config("full.conf")[0]);

	BOOST_TEST(sv->get_id() == "localhost");
	BOOST_TEST(sv->get_hostname() == "irc.localhost");
	BOOST_TEST(sv->get_port() == 3344U);
	BOOST_TEST(sv->get_password() == "secret");
	BOOST_TEST(sv->get_nickname() == "superbot");
	BOOST_TEST(sv->get_username() == "sp");
	BOOST_TEST(sv->get_realname() == "SuperBot 2000 NT");
	BOOST_TEST(static_cast<bool>(sv->get_options() & server::options::join_invite));
	BOOST_TEST(static_cast<bool>(sv->get_options() & server::options::auto_rejoin));
	BOOST_TEST(static_cast<bool>(sv->get_options() & server::options::auto_reconnect));
}

#if defined(IRCCD_HAVE_SSL)

BOOST_AUTO_TEST_CASE(ssl)
{
	const auto sv = from_config(ctx_, open_config("ssl.conf")[0]);

	BOOST_TEST(sv->get_id() == "localhost");
	BOOST_TEST(sv->get_hostname() == "irc.localhost");
	BOOST_TEST(sv->get_port() == 6697U);
	BOOST_TEST(sv->get_password() == "secret");
	BOOST_TEST(sv->get_nickname() == "secure");
	BOOST_TEST(sv->get_username() == "sc");
	BOOST_TEST(sv->get_realname() == "SuperBot 2000 NT SSL");
	BOOST_TEST(static_cast<bool>(sv->get_options() & server::options::ssl));
	BOOST_TEST(static_cast<bool>(sv->get_options() & server::options::join_invite));
	BOOST_TEST(static_cast<bool>(sv->get_options() & server::options::auto_rejoin));
	BOOST_TEST(static_cast<bool>(sv->get_options() & server::options::auto_reconnect));
}

#endif // !IRCCD_HAVE_SSL

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE(load_from_json, fixture)

BOOST_AUTO_TEST_SUITE(valid)

BOOST_AUTO_TEST_CASE(full)
{
	const auto sv = from_json(ctx_, open_json("full.json"));

	BOOST_TEST(sv->get_id() == "localhost");
	BOOST_TEST(sv->get_hostname() == "irc.localhost");
	BOOST_TEST(sv->get_port() == 3344U);
	BOOST_TEST(sv->get_password() == "secret");
	BOOST_TEST(sv->get_nickname() == "superbot");
	BOOST_TEST(sv->get_username() == "sp");
	BOOST_TEST(sv->get_realname() == "SuperBot 2000 NT");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(message)

BOOST_AUTO_TEST_CASE(valid_short)
{
	const auto m = message_type::parse("!hello", "!", "hello");

	BOOST_TEST(m.type == message_type::is_command);
	BOOST_TEST(m.message == "");
}

BOOST_AUTO_TEST_CASE(valid_arguments)
{
	const auto m = message_type::parse("!hello world", "!", "hello");

	BOOST_TEST(m.type == message_type::is_command);
	BOOST_TEST(m.message == "world");
}

BOOST_AUTO_TEST_CASE(cchar_with_message_short)
{
	const auto m = message_type::parse("!hello", "!", "hangman");

	BOOST_TEST(m.type == message_type::is_message);
	BOOST_TEST(m.message == "!hello");
}

BOOST_AUTO_TEST_CASE(cchar_with_message_arguments)
{
	const auto m = message_type::parse("!hello world", "!", "hangman");

	BOOST_TEST(m.type == message_type::is_message);
	BOOST_TEST(m.message == "!hello world");
}

BOOST_AUTO_TEST_CASE(command_with_different_cchar_short)
{
	const auto m = message_type::parse("!hello", ">", "hello");

	BOOST_TEST(m.type == message_type::is_message);
	BOOST_TEST(m.message == "!hello");
}

BOOST_AUTO_TEST_CASE(command_with_different_cchar_arguments)
{
	const auto m = message_type::parse("!hello", ">", "hello");

	BOOST_TEST(m.type == message_type::is_message);
	BOOST_TEST(m.message == "!hello");
}

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
