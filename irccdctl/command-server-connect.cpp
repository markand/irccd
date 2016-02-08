/*
 * command-server-connect.cpp -- implementation of irccdctl server-connect
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

#include <options.h>

#include "command-server-connect.h"

namespace irccd {

namespace command {

namespace {

parser::Result parse(std::vector<std::string> &args)
{
	parser::Options options{
		{ "-c",			true	},
		{ "--command",		true	},
		{ "-n",			true	},
		{ "--nickname",		true	},
		{ "-r",			true	},
		{ "--realname",		true	},
		{ "-S",			false	},
		{ "--ssl-verify",	false	},
		{ "-s",			false	},
		{ "--ssl",		false	},
		{ "-u",			true	},
		{ "--username",		true	}
	};

	return parser::read(args, options);
}

} // !namespace

void ServerConnect::usage(Irccdctl &) const
{
	log::warning() << "usage: " << sys::programName() << " server-connect [options] id host [port]\n\n";
	log::warning() << "Connect to a server.\n\n";
	log::warning() << "Available options:\n";
	log::warning() << "  -c, --command\t\tspecify the command char\n";
	log::warning() << "  -n, --nickname\tspecify a nickname\n";
	log::warning() << "  -r, --realname\tspecify a real name\n";
	log::warning() << "  -S, --ssl-verify\tverify SSL\n";
	log::warning() << "  -s, --ssl\t\tconnect using SSL\n";
	log::warning() << "  -u, --username\tspecify a user name\n\n";
	log::warning() << "Example:\n";
	log::warning() << "\t" << sys::programName() << " server-connect -n jean example irc.example.org\n";
	log::warning() << "\t" << sys::programName() << " server-connect --ssl example irc.example.org 6697" << std::endl;
}

void ServerConnect::exec(Irccdctl &irccdctl, const std::vector<std::string> &args) const
{
	std::vector<std::string> copy(args);

	parser::Result result = parse(copy);
	parser::Result::const_iterator it;

	if (copy.size() < 2)
		throw std::invalid_argument("server-connect requires at least 2 arguments");

	json::Value object = json::object({
		{ "command",	"server-connect"	},
		{ "name",	copy[0]			},
		{ "host",	copy[1]			}
	});

	/* Port */
	if (copy.size() == 3) {
		if (!util::isNumber(copy[2]))
			throw std::invalid_argument("invalid port number");

		object.insert("port", std::stoi(copy[2]));
	}

	/* SSL */
	if (result.count("-S") > 0 || result.count("--ssl-verify") > 0)
		object.insert("sslVerify", true);
	if (result.count("-s") > 0 || result.count("--ssl") > 0)
		object.insert("ssl", true);

	/* Identity */
	if ((it = result.find("-n")) != result.end() || (it = result.find("--nickname")) != result.end())
		object.insert("nickname", it->second);
	if ((it = result.find("-r")) != result.end() || (it = result.find("--realname")) != result.end())
		object.insert("realname", it->second);
	if ((it = result.find("-u")) != result.end() || (it = result.find("--username")) != result.end())
		object.insert("username", it->second);

	irccdctl.connection().send(object.toJson(0));
	irccdctl.connection().verify("server-connect");
}

} // !command

} // !irccd
