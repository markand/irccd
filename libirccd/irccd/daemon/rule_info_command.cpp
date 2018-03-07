/*
 * rule_info_command.cpp -- implementation of rule-info transport command
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

#include "irccd.hpp"
#include "rule_info_command.hpp"
#include "transport_client.hpp"

#include <irccd/daemon/service/rule_service.hpp>

namespace irccd {

std::string rule_info_command::get_name() const noexcept
{
    return "rule-info";
}

void rule_info_command::exec(irccd& irccd, transport_client& client, const nlohmann::json& args)
{
    auto json = rule_service::to_json(irccd.rules().require(irccd.rules().get_index(args)));

    json.push_back({"command", "rule-info"});
    client.send(std::move(json));
}

} // !irccd
