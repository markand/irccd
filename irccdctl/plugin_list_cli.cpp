/*
 * plugin_list_cli.cpp -- implementation of irccdctl plugin-list
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

#include <iostream>

#include "plugin_list_cli.hpp"

namespace irccd {

namespace ctl {

std::string plugin_list_cli::name() const
{
    return "plugin-list";
}

void plugin_list_cli::exec(ctl::controller& ctl, const std::vector<std::string>&)
{
    request(ctl, {{ "command", "plugin-list" }}, [] (auto result) {
        for (const auto& value : result["list"])
            if (value.is_string())
                std::cout << value.template get<std::string>() << std::endl;
    });
}

} // !ctl

} // !irccd
