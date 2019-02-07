/*
 * server_util.cpp -- server utilities
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

#include <irccd/sysconfig.hpp>

#include <algorithm>

#include <irccd/config.hpp>
#include <irccd/ini_util.hpp>
#include <irccd/json_util.hpp>
#include <irccd/string_util.hpp>

#include "server.hpp"
#include "server_util.hpp"

using irccd::json_util::deserializer;

namespace irccd::daemon::server_util {

namespace {

void toggle(server& s, server::options opt, bool value) noexcept
{
	if (value)
		s.set_options(s.get_options() | opt);
	else
		s.set_options(s.get_options() & ~(opt));
}

void from_config_load_identity(server& sv, const ini::section& sc)
{
	const auto username = ini_util::optional_string(sc, "username", sv.get_username());
	const auto realname = ini_util::optional_string(sc, "realname", sv.get_realname());
	const auto nickname = ini_util::optional_string(sc, "nickname", sv.get_nickname());
	const auto ctcp_version = ini_util::optional_string(sc, "ctcp-version", sv.get_ctcp_version());

	if (username.empty())
		throw server_error(server_error::invalid_username);
	if (realname.empty())
		throw server_error(server_error::invalid_realname);
	if (nickname.empty())
		throw server_error(server_error::invalid_nickname);
	if (ctcp_version.empty())
		throw server_error(server_error::invalid_ctcp_version);

	sv.set_username(username);
	sv.set_realname(realname);
	sv.set_nickname(nickname);
	sv.set_ctcp_version(ctcp_version);
}

void from_config_load_channels(server& sv, const ini::section& sc)
{
	for (const auto& s : sc.get("channels")) {
		channel channel;

		if (auto pos = s.find(":") != std::string::npos) {
			channel.name = s.substr(0, pos);
			channel.password = s.substr(pos + 1);
		} else
			channel.name = s;

		sv.join(channel.name, channel.password);
	}
}

void from_config_load_flags(server& sv, const ini::section& sc)
{
	const auto ssl = sc.find("ssl");
	const auto auto_rejoin = sc.find("auto-rejoin");
	const auto auto_reconnect = sc.find("auto-reconnect");
	const auto join_invite = sc.find("join-invite");
	const auto ipv4 = sc.find("ipv4");
	const auto ipv6 = sc.find("ipv6");

	if (ssl != sc.end())
		toggle(sv, server::options::ssl, string_util::is_boolean(ssl->get_value()));
	if (auto_rejoin != sc.end())
		toggle(sv, server::options::auto_rejoin, string_util::is_boolean(auto_rejoin->get_value()));
	if (auto_reconnect != sc.end())
		toggle(sv, server::options::auto_reconnect, string_util::is_boolean(auto_reconnect->get_value()));
	if (join_invite != sc.end())
		toggle(sv, server::options::join_invite, string_util::is_boolean(join_invite->get_value()));
	if (ipv4 != sc.end())
		toggle(sv, server::options::ipv4, string_util::is_boolean(ipv4->get_value()));
	if (ipv6 != sc.end())
		toggle(sv, server::options::ipv6, string_util::is_boolean(ipv6->get_value()));

	if ((sv.get_options() & server::options::ipv4) != server::options::ipv4 &&
	    (sv.get_options() & server::options::ipv6) != server::options::ipv6)
		throw server_error(server_error::invalid_family);
}

void from_config_load_numeric_parameters(server& sv, const ini::section& sc)
{
	const auto port = ini_util::optional_uint<std::uint16_t>(sc, "port", sv.get_port());
	const auto ping_timeout = ini_util::optional_uint<uint16_t>(sc, "ping-timeout", sv.get_ping_timeout());
	const auto reco_timeout = ini_util::optional_uint<uint16_t>(sc, "auto-reconnect-delay", sv.get_reconnect_delay());

	if (!port)
		throw server_error(server_error::invalid_port);
	if (!ping_timeout)
		throw server_error(server_error::invalid_ping_timeout);
	if (!reco_timeout)
		throw server_error(server_error::invalid_reconnect_delay);

	sv.set_port(*port);
	sv.set_ping_timeout(*ping_timeout);
	sv.set_reconnect_delay(*reco_timeout);
}

void from_config_load_options(server& sv, const ini::section& sc)
{
	const auto password = ini_util::optional_string(sc, "password", "");
	const auto command_char = ini_util::optional_string(sc, "command-char", sv.get_command_char());

	sv.set_password(password);
	sv.set_command_char(command_char);
}

void from_json_load_general(server& sv, const deserializer& parser)
{
	const auto port = parser.optional<std::uint16_t>("port", sv.get_port());
	const auto nickname = parser.optional<std::string>("nickname", sv.get_nickname());
	const auto realname = parser.optional<std::string>("realname", sv.get_realname());
	const auto username = parser.optional<std::string>("username", sv.get_username());
	const auto ctcp_version = parser.optional<std::string>("ctcpVersion", sv.get_ctcp_version());
	const auto command = parser.optional<std::string>("commandChar", sv.get_command_char());
	const auto password = parser.optional<std::string>("password", sv.get_password());

	if (!port || *port > std::numeric_limits<std::uint16_t>::max())
		throw server_error(server_error::invalid_port);
	if (!nickname)
		throw server_error(server_error::invalid_nickname);
	if (!realname)
		throw server_error(server_error::invalid_realname);
	if (!username)
		throw server_error(server_error::invalid_username);
	if (!ctcp_version)
		throw server_error(server_error::invalid_ctcp_version);
	if (!command)
		throw server_error(server_error::invalid_command_char);
	if (!password)
		throw server_error(server_error::invalid_password);

	sv.set_port(*port);
	sv.set_nickname(*nickname);
	sv.set_realname(*realname);
	sv.set_username(*username);
	sv.set_ctcp_version(*ctcp_version);
	sv.set_command_char(*command);
	sv.set_password(*password);
}

void from_json_load_options(server& sv, const deserializer& parser)
{
	const auto auto_rejoin = parser.get<bool>("autoRejoin");
	const auto join_invite = parser.get<bool>("joinInvite");
	const auto ssl = parser.get<bool>("ssl");
	const auto ipv4 = parser.optional<bool>("ipv4", true);
	const auto ipv6 = parser.optional<bool>("ipv6", true);

	if (!ipv4 || !ipv6)
		throw server_error(server_error::invalid_family);

	if (ipv4)
		toggle(sv, server::options::ipv4, *ipv4);
	if (ipv6)
		toggle(sv, server::options::ipv6, *ipv6);
	if (auto_rejoin)
		toggle(sv, server::options::auto_rejoin, *auto_rejoin);
	if (join_invite)
		toggle(sv, server::options::join_invite, *join_invite);
	if (ssl)
		toggle(sv, server::options::ssl, *ssl);

#if !defined(IRCCD_HAVE_SSL)
	if ((sv.get_options() & server::options::ssl) == server::options::ssl)
		throw server_error(server_error::ssl_disabled);
#endif

	// Verify that at least IPv4 or IPv6 is set.
	if ((sv.get_options() & server::options::ipv4) != server::options::ipv4 &&
	    (sv.get_options() & server::options::ipv6) != server::options::ipv6)
		throw server_error(server_error::invalid_family);
}

} // !namespace

auto message_type::parse(std::string_view message,
                         std::string_view cchar,
                         std::string_view plugin) -> message_type
{
	auto result = std::string(message);
	auto cc = std::string(cchar);
	auto name = std::string(plugin);
	auto type = is_message;

	// handle special commands "!<plugin> command"
	if (cc.length() > 0) {
		auto pos = result.find_first_of(" \t");
		auto fullcommand = cc + name;

		/*
		 * If the message that comes is "!foo" without spaces we
		 * compare the command char + the plugin name. If there
		 * is a space, we check until we find a space, if not
		 * typing "!foo123123" will trigger foo plugin.
		 */
		if (pos == std::string::npos)
			type = result == fullcommand ? is_command : is_message;
		else if (result.length() >= fullcommand.length() && result.compare(0, pos, fullcommand) == 0)
			type = is_command;

		if (type == is_command) {
			/*
			 * If no space is found we just set the message to "" otherwise
			 * the plugin name will be passed through onCommand
			 */
			if (pos == std::string::npos)
				result = "";
			else
				result = message.substr(pos + 1);
		}
	}

	return {type, result};
}

auto from_json(boost::asio::io_service& service, const nlohmann::json& object) -> std::shared_ptr<server>
{
	// Mandatory parameters.
	const deserializer parser(object);
	const auto id = parser.get<std::string>("name");
	const auto hostname = parser.get<std::string>("hostname");

	if (!id || !string_util::is_identifier(*id))
		throw server_error(server_error::invalid_identifier);
	if (!hostname || hostname->empty())
		throw server_error(server_error::invalid_hostname);

	const auto sv = std::make_shared<server>(service, *id, *hostname);

	from_json_load_general(*sv, parser);
	from_json_load_options(*sv, parser);

	return sv;
}

auto from_config(boost::asio::io_service& service,
                 const ini::section& sc) -> std::shared_ptr<server>
{
	// Mandatory parameters.
	const auto id = sc.get("name");
	const auto hostname = sc.get("hostname");

	if (!string_util::is_identifier(id.get_value()))
		throw server_error(server_error::invalid_identifier);
	if (hostname.get_value().empty())
		throw server_error(server_error::invalid_hostname);

	const auto sv = std::make_shared<server>(service, id.get_value(), hostname.get_value());

	from_config_load_channels(*sv, sc);
	from_config_load_flags(*sv, sc);
	from_config_load_numeric_parameters(*sv, sc);
	from_config_load_options(*sv, sc);
	from_config_load_identity(*sv, sc);

	return sv;
}

} // !irccd::daemon::server_util
