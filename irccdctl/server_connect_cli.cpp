/*
 * server_connect_cli.cpp -- implementation of irccdctl server-connect
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

#include <irccd/options.hpp>
#include <irccd/string_util.hpp>

#include "server_connect_cli.hpp"

namespace irccd {

namespace ctl {

namespace {

option::result parse(std::vector<std::string> &args)
{
    option::options options{
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

std::string server_connect_cli::name() const
{
    return "server-connect";
}

void server_connect_cli::exec(ctl::controller& ctl, const std::vector<std::string>& args)
{
    std::vector<std::string> copy(args);

    option::result result = parse(copy);
    option::result::const_iterator it;

    if (copy.size() < 2)
        throw std::invalid_argument("server-connect requires at least 2 arguments");

    auto object = nlohmann::json::object({
        { "name", copy[0] },
        { "host", copy[1] }
    });

    if (copy.size() == 3) {
        const auto port = string_util::to_int(copy[2]);

        if (!port)
            throw std::invalid_argument("invalid port given");

        object["port"] = *port;
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

    request(ctl, object);
}

} // !ctl

} // !irccd
