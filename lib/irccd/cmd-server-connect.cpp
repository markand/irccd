/*
 * cmd-server-connect.cpp -- implementation of server-connect transport command
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

#include <limits>

#include "cmd-server-connect.hpp"
#include "irccd.hpp"
#include "server.hpp"
#include "util.hpp"

namespace irccd {

namespace command {

namespace {

std::string readInfoName(const json::Value &object)
{
	auto it = object.find("name");

	if (it == object.end())
		throw std::invalid_argument("missing 'name' property");
	if (!it->isString() || !util::isIdentifierValid(it->toString()))
		throw std::invalid_argument("invalid server name");

	return it->toString();
}

std::string readInfoHost(const json::Value &object)
{
	auto it = object.find("host");

	if (it == object.end())
		throw std::invalid_argument("missing 'host' property");
	if (!it->isString())
		throw std::invalid_argument("invalid host");

	return it->toString();
}

std::uint16_t readInfoPort(const json::Value &object)
{
	auto it = object.find("port");
	uint16_t port = 6667;

	if (it != object.end())
		if (it->isInt() && it->toInt() >= 0 && it->toInt() <= std::numeric_limits<std::uint16_t>::max())
			port = static_cast<std::uint16_t>(it->toInt());

	return port;
}

ServerInfo readInfo(const json::Value &object)
{
	ServerInfo info;

	/* Mandatory */
	info.name = readInfoName(object);
	info.host = readInfoHost(object);

	/* Optional */
	info.port = readInfoPort(object);

	if (object.valueOr("ssl", json::Type::Boolean, false).toBool())
#if defined(WITH_SSL)
		info.flags |= ServerInfo::Ssl;
#else
		throw std::invalid_argument("ssl is disabled");
#endif

	if (object.valueOr("sslVerify", json::Type::Boolean, false).toBool())
		info.flags |= ServerInfo::SslVerify;

	return info;
}

ServerIdentity readIdentity(const json::Value &object)
{
	ServerIdentity identity;

	identity.nickname = object.valueOr("nickname", json::Type::String, identity.nickname).toString();
	identity.realname = object.valueOr("realname", json::Type::String, identity.realname).toString();
	identity.username = object.valueOr("username", json::Type::String, identity.username).toString();
	identity.ctcpversion = object.valueOr("ctcpVersion", json::Type::String, identity.ctcpversion).toString();

	return identity;
}

ServerSettings readSettings(const json::Value &object)
{
	ServerSettings settings;

	settings.command = object.valueOr("commandChar", json::Type::String, settings.command).toString();
	settings.recotries = object.valueOr("reconnectTries", json::Type::Int, settings.recotries).toInt();
	settings.recotimeout = object.valueOr("reconnectTimeout", json::Type::Int, settings.recotimeout).toInt();

	return settings;
}

} // !namespace

ServerConnect::ServerConnect()
	: RemoteCommand("server-connect", "Server")
{
}

std::string ServerConnect::help() const
{
	return "Connect to a server.";
}

std::vector<RemoteCommand::Option> ServerConnect::options() const
{
	return {
		{ "command", "c", "command", "char", "command character to use" },
		{ "nickname", "n", "nickname", "nickname", "nickname to use" },
		{ "realname", "r", "realname", "realname", "realname to use" },
		{ "sslverify", "S", "ssl-verify", "", "verify SSL" },
		{ "ssl", "s", "ssl", "", "connect with SSL" },
		{ "username", "u", "username", "", "username to use" },
	};
}

std::vector<RemoteCommand::Arg> ServerConnect::args() const
{
	return {
		{ "id", true },
		{ "host", true },
		{ "port", false }
	};
}

json::Value ServerConnect::exec(Irccd &irccd, const json::Value &request) const
{
	auto server = std::make_shared<Server>(readInfo(request), readIdentity(request), readSettings(request));

	if (irccd.hasServer(server->info().name))
		throw std::invalid_argument("server '" + server->info().name + "' already exists");

	irccd.addServer(std::move(server));

	return RemoteCommand::exec(irccd, request);
}

} // !command

} // !irccd
