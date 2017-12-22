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

#include <irccd/config.hpp>
#include <irccd/string_util.hpp>

#include "irccd.hpp"
#include "logger.hpp"
#include "rule_service.hpp"

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
    rule::action action = rule::action::accept;

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
        action = rule::action::drop;
    else if (actionstr == "accept")
        action = rule::action::accept;
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

rule rule_service::from_json(const nlohmann::json& json)
{
    auto toset = [] (auto object, auto name) {
        rule::set result;

        for (const auto& s : object[name])
            if (s.is_string())
                result.insert(s.template get<std::string>());

        return result;
    };
    auto toaction = [] (auto object, auto name) {
        auto v = object[name];

        if (!v.is_string())
            throw rule_error(rule_error::invalid_action);

        auto s = v.template get<std::string>();
        if (s == "accept")
            return rule::action::accept;
        if (s == "drop")
            return rule::action::drop;

        throw rule_error(rule_error::invalid_action);
    };

    return {
        toset(json, "servers"),
        toset(json, "channels"),
        toset(json, "origins"),
        toset(json, "plugins"),
        toset(json, "events"),
        toaction(json, "action")
    };
}

unsigned rule_service::get_index(const nlohmann::json& json, const std::string& key)
{
    auto index = json.find(key);

    if (index == json.end() || !index->is_number_integer() || index->get<int>() < 0)
        throw rule_error(rule_error::invalid_index);

    return index->get<int>();
}

nlohmann::json rule_service::to_json(const rule& rule)
{
    auto join = [] (const auto& set) {
        auto array = nlohmann::json::array();

        for (const auto& entry : set)
            array.push_back(entry);

        return array;
    };
    auto str = [] (auto action) {
        switch (action) {
        case rule::action::accept:
            return "accept";
        default:
            return "drop";
        }
    };

    return {
        { "servers",    join(rule.get_servers())    },
        { "channels",   join(rule.get_channels())   },
        { "plugins",    join(rule.get_plugins())    },
        { "events",     join(rule.get_events())     },
        { "action",     str(rule.get_action())      }
    };
}

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
            rules_.push_back(load_rule(section));
        } catch (const std::exception& ex) {
            irccd_.log().warning() << "rule: " << ex.what() << std::endl;
        }
    }
}

} // !irccd
