/*
 * rule_service.cpp -- rule service
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

#include "logger.hpp"
#include "rule_service.hpp"
#include "string_util.hpp"

namespace irccd {

void rule_service::add(rule rule)
{
    rules_.push_back(std::move(rule));
}

void rule_service::insert(rule rule, unsigned position)
{
    assert(position <= rules_.size());

    rules_.insert(rules_.begin() + position, std::move(rule));
}

void rule_service::remove(unsigned position)
{
    assert(position < rules_.size());

    rules_.erase(rules_.begin() + position);
}

const rule &rule_service::require(unsigned position) const
{
    if (position >= rules_.size())
        throw std::out_of_range("rule " + std::to_string(position) + " does not exist");

    return rules_[position];
}

rule &rule_service::require(unsigned position)
{
    if (position >= rules_.size())
        throw std::out_of_range("rule " + std::to_string(position) + " does not exist");

    return rules_[position];
}

bool rule_service::solve(const std::string& server,
                         const std::string& channel,
                         const std::string& origin,
                         const std::string& plugin,
                         const std::string& event) noexcept
{
    bool result = true;

    log::debug(string_util::sprintf("rule: solving for server=%s, channel=%s, origin=%s, plugin=%s, event=%s",
        server, channel, origin, plugin, event));

    int i = 0;
    for (const auto& rule : rules_) {
        log::debug() << "  candidate "   << i++ << ":\n"
                     << "    servers: "  << string_util::join(rule.servers().begin(), rule.servers().end()) << "\n"
                     << "    channels: " << string_util::join(rule.channels().begin(), rule.channels().end()) << "\n"
                     << "    origins: "  << string_util::join(rule.origins().begin(), rule.origins().end()) << "\n"
                     << "    plugins: "  << string_util::join(rule.plugins().begin(), rule.plugins().end()) << "\n"
                     << "    events: "   << string_util::join(rule.events().begin(), rule.events().end()) << "\n"
                     << "    action: "   << ((rule.action() == rule::action_type::accept) ? "accept" : "drop") << std::endl;

        if (rule.match(server, channel, origin, plugin, event))
            result = rule.action() == rule::action_type::accept;
    }

    return result;
}

} // !irccd
