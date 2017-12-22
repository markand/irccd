/*
 * rule_move_command.cpp -- implementation of rule-move transport command
 *
 * Copyright (c) 2013-2017 David Demelier <markand@malikania.fr>
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
#include "rule_move_command.hpp"
#include "rule_service.hpp"
#include "transport_client.hpp"

namespace irccd {

std::string rule_move_command::get_name() const noexcept
{
    return "rule-move";
}

void rule_move_command::exec(irccd& irccd, transport_client& client, const nlohmann::json& args)
{
    auto from = rule_service::get_index(args, "from");
    auto to = rule_service::get_index(args, "to");

    /*
     * Examples of moves
     * --------------------------------------------------------------
     *
     * Before: [0] [1] [2]
     *
     * from = 0
     * to   = 2
     *
     * After:  [1] [2] [0]
     *
     * --------------------------------------------------------------
     *
     * Before: [0] [1] [2]
     *
     * from = 2
     * to   = 0
     *
     * After:  [2] [0] [1]
     *
     * --------------------------------------------------------------
     *
     * Before: [0] [1] [2]
     *
     * from = 0
     * to   = 123
     *
     * After:  [1] [2] [0]
     */

    // Ignore dumb input.
    if (from == to) {
        client.success("rule-move");
        return;
    }

    if (from >= irccd.rules().length())
        throw rule_error(rule_error::error::invalid_index);

    auto save = irccd.rules().list()[from];

    irccd.rules().remove(from);
    irccd.rules().insert(save, to > irccd.rules().length() ? irccd.rules().length() : to);
    client.success("rule-move");
}

} // !irccd
