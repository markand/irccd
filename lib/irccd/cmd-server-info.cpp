/*
 * cmd-server-info.cpp -- implementation of server-info transport command
 *
 * Copyright (c) 2013-2016 David Demelier <markand@malikania.fr>
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

#include <iostream>

#include "cmd-server-info.hpp"
#include "irccd.hpp"
#include "server.hpp"
#include "service-server.hpp"

namespace irccd {

namespace command {

ServerInfo::ServerInfo()
	: Command("server-info", "Server")
{
}

std::string ServerInfo::help() const
{
	return "";
}

std::vector<Command::Arg> ServerInfo::args() const
{
	return {{ "server", true }};
}

std::vector<Command::Property> ServerInfo::properties() const
{
	return {{ "server", { json::Type::String }}};
}

json::Value ServerInfo::request(Irccdctl &, const CommandRequest &args) const
{
	return json::object({
		{ "server",	args.args()[0] },
		{ "target",	args.args()[1] },
		{ "channel",	args.args()[2] }
	});
}

json::Value ServerInfo::exec(Irccd &irccd, const json::Value &request) const
{
	auto response = Command::exec(irccd, request);
	auto server = irccd.serverService().require(request.at("server").toString());

	// General stuff.
	response.insert("name", server->name());
	response.insert("host", server->info().host);
	response.insert("port", server->info().port);
	response.insert("nickname", server->identity().nickname);
	response.insert("username", server->identity().username);
	response.insert("realname", server->identity().realname);

	// Optional stuff.
	if (server->info().flags & irccd::ServerInfo::Ipv6)
		response.insert("ipv6", true);
	if (server->info().flags & irccd::ServerInfo::Ssl)
		response.insert("ssl", true);
	if (server->info().flags & irccd::ServerInfo::SslVerify)
		response.insert("sslVerify", true);

	// Channel list.
	auto channels = json::array({});

	for (const auto &c : server->settings().channels)
		channels.append(c.name);

	response.insert("channels", std::move(channels));

	return response;
}

void ServerInfo::result(Irccdctl &irccdctl, const json::Value &response) const
{
	Command::result(irccdctl, response);

	// Server information.
	std::cout << std::boolalpha;
	std::cout << "Name           : " << response.valueOr("name", "").toString(true) << std::endl;
	std::cout << "Host           : " << response.valueOr("host", "").toString(true) << std::endl;
	std::cout << "Port           : " << response.valueOr("port", "").toString(true) << std::endl;
	std::cout << "Ipv6           : " << response.valueOr("ipv6", "").toString(true) << std::endl;
	std::cout << "SSL            : " << response.valueOr("ssl", "").toString(true) << std::endl;
	std::cout << "SSL verified   : " << response.valueOr("sslVerify", "").toString(true) << std::endl;

	// Channels.
	std::cout << "Channels       : ";

	for (const json::Value &v : response.valueOr("channels", json::Type::Array, json::array({})))
		std::cout << v.toString() << " ";

	std::cout << std::endl;

	// Identity.
	std::cout << "Nickname       : " << response.valueOr("nickname", "").toString(true) << std::endl;
	std::cout << "User name      : " << response.valueOr("username", "").toString(true) << std::endl;
	std::cout << "Real name      : " << response.valueOr("realname", "").toString(true) << std::endl;
}

} // !command

} // !irccd
