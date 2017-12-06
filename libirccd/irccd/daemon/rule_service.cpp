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

#include <stdexcept>

#include <irccd/logger.hpp>
#include <irccd/string_util.hpp>

#include "config.hpp"
#include "rule_service.hpp"
#include "string_util.hpp"

namespace irccd {

namespace {

rule load_rule(const ini::section& sc)
{
    assert(sc.key() == "rule");

    // Simple converter from std::vector to std::unordered_set.
    auto toset = [] (const auto& v) {
        return std::unordered_set<std::string>(v.begin(), v.end());
    };

    rule::set servers, channels, origins, plugins, events;
    rule::action_type action = rule::action_type::accept;

    // Get the sets.
    ini::section::const_iterator it;

    if ((it = sc.find("servers")) != sc.end())
        servers = toset(*it);
    if ((it = sc.find("channels")) != sc.end())
        channels = toset(*it);
    if ((it = sc.find("origins")) != sc.end())
        origins = toset(*it);
    if ((it = sc.find("plugins")) != sc.end())
        plugins = toset(*it);
    if ((it = sc.find("channels")) != sc.end())
        channels = toset(*it);

    // Get the action.
    auto actionstr = sc.get("action").value();

    if (actionstr == "drop")
        action = rule::action_type::drop;
    else if (actionstr == "accept")
        action = rule::action_type::accept;
    else
        throw rule_error(rule_error::invalid_action);

    return {
        std::move(servers),
        std::move(channels),
        std::move(origins),
        std::move(plugins),
        std::move(events),
        action
    };
}

} // !namespace

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

    log::debug(string_util::sprintf("rule: solving for server=%s, channel=%s, origin=%s, plugin=%s, event=%s",
        server, channel, origin, plugin, event));

    int i = 0;
    for (const auto& rule : rules_) {
        auto action = rule.action() == rule::action_type::accept ? "accept" : "drop";

        log::debug() << "  candidate "   << i++ << ":\n"
                     << "    servers: "  << string_util::join(rule.servers()) << "\n"
                     << "    channels: " << string_util::join(rule.channels()) << "\n"
                     << "    origins: "  << string_util::join(rule.origins()) << "\n"
                     << "    plugins: "  << string_util::join(rule.plugins()) << "\n"
                     << "    events: "   << string_util::join(rule.events()) << "\n"
                     << "    action: "   << action << std::endl;

        if (rule.match(server, channel, origin, plugin, event))
            result = rule.action() == rule::action_type::accept;
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
            rules_.push_back(load_rule(section));
        } catch (const std::exception& ex) {
            log::warning() << "rule: " << ex.what() << std::endl;
        }
    }
}

} // !irccd
