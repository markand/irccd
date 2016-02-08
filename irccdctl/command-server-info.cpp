/*
 * command-server-info.cpp -- implementation of irccdctl server-info
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

#include "command-server-info.h"

namespace irccd {

namespace command {

void ServerInfo::usage(Irccdctl &) const
{
	log::warning() << "usage: " << sys::programName() << " server-info server" << std::endl;
}

void ServerInfo::exec(Irccdctl &ctl, const std::vector<std::string> &args) const
{
	if (args.size() < 1)
		throw std::invalid_argument("server-info requires 1 argument");

	ctl.connection().send(json::object({
		{ "command",	"server-info"	},
		{ "server",	args[0]		}
	}).toJson(0));

	/* Show everything */
	json::Value obj = ctl.connection().next("server-info");

	if (obj.contains("error"))
		throw std::runtime_error(obj.at("error").toString());

	/* Server information */
	std::cout << std::boolalpha;
	std::cout << "Name           : " << obj.valueOr("name", "").toString(true) << std::endl;
	std::cout << "Host           : " << obj.valueOr("host", "").toString(true) << std::endl;
	std::cout << "Port           : " << obj.valueOr("port", "").toString(true) << std::endl;
	std::cout << "Ipv6           : " << obj.valueOr("ipv6", "").toString(true) << std::endl;
	std::cout << "SSL            : " << obj.valueOr("ssl", "").toString(true) << std::endl;
	std::cout << "SSL verified   : " << obj.valueOr("sslVerify", "").toString(true) << std::endl;

	/* Channels */
	std::cout << "Channels       : ";

	for (const json::Value &v : obj.valueOr("channels", json::Type::Array, json::array({})))
		std::cout << v.toString() << " ";

	std::cout << std::endl;

	/* Identity */
	std::cout << "Nickname       : " << obj.valueOr("nickname", "").toString(true) << std::endl;
	std::cout << "User name      : " << obj.valueOr("username", "").toString(true) << std::endl;
	std::cout << "Real name      : " << obj.valueOr("realname", "").toString(true) << std::endl;
}

} // !command

} // !irccd
