/*
 * rule_remove_cli.cpp -- implementation of irccdctl rule-remove
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

#include <irccd/string_util.hpp>

#include <irccd/daemon/service/rule_service.hpp>

#include "rule_remove_cli.hpp"

namespace irccd {

namespace ctl {

std::string rule_remove_cli::name() const
{
    return "rule-remove";
}

void rule_remove_cli::exec(ctl::controller& ctl, const std::vector<std::string>& args)
{
    if (args.size() < 1)
        throw std::invalid_argument("rule-remove requires 1 argument");

    const auto index = string_util::to_int(args[0]);

    if (!index)
        throw rule_error(rule_error::invalid_index);

    request(ctl, {
        { "command",    "rule-remove"   },
        { "index",      *index          }
    });
}

} // !ctl

} // !irccd
