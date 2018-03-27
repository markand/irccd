/*
 * rule_service.cpp -- rule service
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

#include <stdexcept>

#include <irccd/config.hpp>
#include <irccd/string_util.hpp>

#include <irccd/daemon/irccd.hpp>
#include <irccd/daemon/logger.hpp>
#include <irccd/daemon/rule_util.hpp>

#include <irccd/daemon/service/rule_service.hpp>

namespace irccd {

rule_service::rule_service(irccd &irccd)
    : irccd_(irccd)
{
}

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
        throw rule_error(rule_error::invalid_index);

    return rules_[position];
}

rule &rule_service::require(unsigned position)
{
    if (position >= rules_.size())
        throw rule_error(rule_error::invalid_index);

    return rules_[position];
}

bool rule_service::solve(const std::string& server,
                         const std::string& channel,
                         const std::string& origin,
                         const std::string& plugin,
                         const std::string& event) noexcept
{
    bool result = true;

    irccd_.log().debug(string_util::sprintf("rule: solving for server=%s, channel=%s, origin=%s, plugin=%s, event=%s",
        server, channel, origin, plugin, event));

    int i = 0;
    for (const auto& rule : rules_) {
        auto action = rule.get_action() == rule::action::accept ? "accept" : "drop";

        irccd_.log().debug() << "  candidate "   << i++ << ":\n"
            << "    servers: "  << string_util::join(rule.get_servers()) << "\n"
            << "    channels: " << string_util::join(rule.get_channels()) << "\n"
            << "    origins: "  << string_util::join(rule.get_origins()) << "\n"
            << "    plugins: "  << string_util::join(rule.get_plugins()) << "\n"
            << "    events: "   << string_util::join(rule.get_events()) << "\n"
            << "    action: "   << action << std::endl;

        if (rule.match(server, channel, origin, plugin, event))
            result = rule.get_action() == rule::action::accept;
    }

    return result;
}

void rule_service::load(const config& cfg) noexcept
{
    rules_.clear();

    for (const auto& section : cfg.doc()) {
        if (section.key() != "rule")
            continue;

        try {
            rules_.push_back(rule_util::from_config(section));
        } catch (const std::exception& ex) {
            irccd_.log().warning() << "rule: " << ex.what() << std::endl;
        }
    }
}

} // !irccd
