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

#include <options.hpp>
#include <util.hpp>

#include "cli-server-connect.hpp"

namespace irccd {

namespace cli {

namespace {

option::Result parse(std::vector<std::string> &args)
{
    option::Options options{
        { "-c",             true    },
        { "--command",      true    },
        { "-n",             true    },
        { "--nickname",     true    },
        { "-r",             true    },
        { "--realname",     true    },
        { "-S",             false   },
        { "--ssl-verify",   false   },
        { "-s",             false   },
        { "--ssl",          false   },
        { "-u",             true    },
        { "--username",     true    }
    };

    return option::read(args, options);
}

} // !namespace

#if 0
void ServerConnect::usage(Irccdctl &) const
{

}
#endif

ServerConnectCli::ServerConnectCli()
    : Cli("server-connect",
          "add a server",
          "server-connect [options] id host [port]",
          "Connect to a server.\n\n"
          "Available options:\n"
          "  -c, --command\t\tspecify the command char\n"
          "  -n, --nickname\tspecify a nickname\n"
          "  -r, --realname\tspecify a real name\n"
          "  -S, --ssl-verify\tverify SSL\n"
          "  -s, --ssl\t\tconnect using SSL\n"
          "  -u, --username\tspecify a user name\n\n"
          "Example:\n"
          "\tirccdctl server-connect -n jean example irc.example.org\n"
          "\tirccdctl server-connect --ssl example irc.example.org 6697")
{
}

void ServerConnectCli::exec(Irccdctl &irccdctl, const std::vector<std::string> &args)
{
    std::vector<std::string> copy(args);

    option::Result result = parse(copy);
    option::Result::const_iterator it;

    if (copy.size() < 2)
        throw std::invalid_argument("server-connect requires at least 2 arguments");

    auto object = nlohmann::json::object({
        { "name", copy[0] },
        { "host", copy[1] }
    });

    if (copy.size() == 3) {
        if (!util::isNumber(copy[2]))
            throw std::invalid_argument("invalid port number");

        object["port"] = std::stoi(copy[2]);
    }

    if (result.count("-S") > 0 || result.count("--ssl-verify") > 0)
        object["sslVerify"] = true;
    if (result.count("-s") > 0 || result.count("--ssl") > 0)
        object["ssl"] = true;
    if ((it = result.find("-n")) != result.end() || (it = result.find("--nickname")) != result.end())
        object["nickname"] = it->second;
    if ((it = result.find("-r")) != result.end() || (it = result.find("--realname")) != result.end())
        object["realname"] = it->second;
    if ((it = result.find("-u")) != result.end() || (it = result.find("--username")) != result.end())
        object["username"] = it->second;

    check(request(irccdctl, object));
}

} // !cli

} // !irccd
