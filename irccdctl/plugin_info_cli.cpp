/*
 * plugin_info_cli.cpp -- implementation of irccdctl plugin-info
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

#include <irccd/json_util.hpp>

#include "plugin_info_cli.hpp"

namespace irccd {

namespace ctl {

std::string plugin_info_cli::name() const
{
    return "plugin-info";
}

void plugin_info_cli::exec(ctl::controller& ctl, const std::vector<std::string>& args)
{
    if (args.size() < 1)
        throw std::invalid_argument("plugin-info requires 1 argument");

    request(ctl, {{ "plugin", args[0] }}, [] (auto result) {
        const json_util::parser parser(result);

        std::cout << std::boolalpha;
        std::cout << "Author         : " <<
            parser.get<std::string>("author").value_or("(unknown)") << std::endl;
        std::cout << "License        : " <<
            parser.get<std::string>("license").value_or("(unknown)") << std::endl;
        std::cout << "Summary        : " <<
            parser.get<std::string>("summary").value_or("(unknown)") << std::endl;
        std::cout << "Version        : " <<
            parser.get<std::string>("version").value_or("(unknown)") << std::endl;
    });
}

} // !ctl

} // !irccd
