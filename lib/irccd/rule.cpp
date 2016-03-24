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

#include "logger.h"
#include "rule.h"
#include "util.h"

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

bool Rule::solve(const std::vector<Rule> &rules,
		 const std::string &server,
		 const std::string &channel,
		 const std::string &origin,
		 const std::string &plugin,
		 const std::string &event) noexcept
{
	bool result = true;

	log::debug() << "rule: solving for:\n"
		     << "  server: " << server << "\n"
		     << "  channel: " << channel << "\n"
		     << "  origin: " << origin << "\n"
		     << "  plugin: " << plugin << "\n"
		     << "  event: " << event << std::endl;

	int i = 0;
	for (const Rule &rule : rules) {
		log::debug() << "  candidate " << i++ << ":\n"
			     << "    servers: " << util::join(rule.m_servers.begin(), rule.m_servers.end()) << "\n"
			     << "    channels: " << util::join(rule.m_channels.begin(), rule.m_channels.end()) << "\n"
			     << "    origins: " << util::join(rule.m_origins.begin(), rule.m_origins.end()) << "\n"
			     << "    plugins: " << util::join(rule.m_plugins.begin(), rule.m_plugins.end()) << "\n"
			     << "    events: " << util::join(rule.m_events.begin(), rule.m_events.end()) << "\n"
			     << "    action: " << ((rule.m_action == RuleAction::Accept) ? "accept" : "drop") << std::endl;

		if (rule.match(server, channel, origin, plugin, event))
			result = rule.action() == RuleAction::Accept;
	}

	return result;
}

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
