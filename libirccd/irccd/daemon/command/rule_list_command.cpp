/*
 * rule_list_command.cpp -- implementation of rule-list transport command
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

#include <irccd/daemon/irccd.hpp>
#include <irccd/daemon/transport_client.hpp>

#include <irccd/daemon/service/rule_service.hpp>

#include "rule_list_command.hpp"

namespace irccd {

std::string rule_list_command::get_name() const noexcept
{
    return "rule-list";
}

void rule_list_command::exec(irccd& irccd, transport_client& client, const nlohmann::json&)
{
    auto array = nlohmann::json::array();

    for (const auto& rule : irccd.rules().list())
        array.push_back(rule_service::to_json(rule));

    client.send({
        { "command",    "rule-list"         },
        { "list",       std::move(array)    }
    });
}

} // !irccd
