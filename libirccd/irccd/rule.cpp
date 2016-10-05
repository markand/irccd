/*
 * rule.cpp -- rule for server and channels
 *
 * Copyright (c) 2013-2016 David Demelier <markand@malikania.fr>
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

#include "logger.hpp"
#include "rule.hpp"
#include "util.hpp"

using namespace std;

namespace {

const std::unordered_set<std::string> validEvents{
    "onChannelMode"
    "onChannelNotice",
    "onCommand",
    "onConnect",
    "onInvite",
    "onJoin",
    "onKick",
    "onMessage",
    "onMode",
    "onNames",
    "onNick",
    "onNotice",
    "onPart",
    "onQuery",
    "onQueryCommand",
    "onTopic",
    "onWhois"
};

} // !namespace

namespace irccd {

bool Rule::matchMap(const RuleSet &map, const std::string &value) const noexcept
{
    return value.empty() || map.empty() || map.count(value) == 1;
}

Rule::Rule(RuleSet servers, RuleSet channels, RuleSet origins, RuleSet plugins, RuleSet events, RuleAction action)
    : m_servers(std::move(servers))
    , m_channels(std::move(channels))
    , m_origins(std::move(origins))
    , m_plugins(std::move(plugins))
    , m_events(std::move(events))
    , m_action(action)
{
    for (const std::string &n : m_events)
        if (validEvents.count(n) == 0)
            throw std::invalid_argument(n + " is not a valid event name");
}

bool Rule::match(const std::string &server,
                 const std::string &channel,
                 const std::string &nick,
                 const std::string &plugin,
                 const std::string &event) const noexcept
{
    return matchMap(m_servers, server) &&
           matchMap(m_channels, channel) &&
           matchMap(m_origins, nick) &&
           matchMap(m_plugins, plugin) &&
           matchMap(m_events, event);
}

RuleAction Rule::action() const noexcept
{
    return m_action;
}

const RuleSet &Rule::servers() const noexcept
{
    return m_servers;
}

const RuleSet &Rule::channels() const noexcept
{
    return m_channels;
}

const RuleSet &Rule::origins() const noexcept
{
    return m_origins;
}

const RuleSet &Rule::plugins() const noexcept
{
    return m_plugins;
}

const RuleSet &Rule::events() const noexcept
{
    return m_events;
}

} // !irccd
