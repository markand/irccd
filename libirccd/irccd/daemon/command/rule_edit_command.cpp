/*
 * rule_edit_command.cpp -- implementation of rule-edit transport command
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

#include <irccd/daemon/irccd.hpp>
#include <irccd/daemon/rule_util.hpp>
#include <irccd/daemon/transport_client.hpp>

#include <irccd/daemon/service/rule_service.hpp>

#include "rule_edit_command.hpp"

using namespace std::string_literals;

namespace irccd {

std::string rule_edit_command::get_name() const noexcept
{
    return "rule-edit";
}

void rule_edit_command::exec(irccd& irccd, transport_client& client, const document& args)
{
    static const auto updateset = [] (auto& set, auto args, const auto& key) {
        for (const auto& v : args["remove-"s + key]) {
            if (v.is_string())
                set.erase(v.template get<std::string>());
        }
        for (const auto& v : args["add-"s + key]) {
            if (v.is_string())
                set.insert(v.template get<std::string>());
        }
    };

    const auto index = args.get<unsigned>("index");

    if (!index)
        throw rule_error(rule_error::invalid_index);

    // Create a copy to avoid incomplete edition in case of errors.
    auto rule = irccd.rules().require(*index);

    updateset(rule.get_channels(), args, "channels");
    updateset(rule.get_events(), args, "events");
    updateset(rule.get_plugins(), args, "plugins");
    updateset(rule.get_servers(), args, "servers");

    auto action = args.find("action");

    if (action != args.end()) {
        if (!action->is_string())
            throw rule_error(rule_error::error::invalid_action);

        if (action->get<std::string>() == "accept")
            rule.set_action(rule::action::accept);
        else if (action->get<std::string>() == "drop")
            rule.set_action(rule::action::drop);
        else
            throw rule_error(rule_error::invalid_action);
    }

    // All done, sync the rule.
    irccd.rules().require(*index) = rule;
    client.success("rule-edit");
}

} // !irccd
