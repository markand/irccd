/*
 * service-rule.cpp -- store and solve rules
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

#include <cassert>

#include <format.h>

#include "logger.hpp"
#include "service-rule.hpp"
#include "util.hpp"

using namespace fmt::literals;

namespace irccd {

void RuleService::add(Rule rule)
{
	m_rules.push_back(std::move(rule));
}

void RuleService::insert(Rule rule, unsigned position)
{
	assert(position <= m_rules.size());

	m_rules.insert(m_rules.begin() + position, std::move(rule));
}

void RuleService::remove(unsigned position)
{
	assert(position < m_rules.size());

	m_rules.erase(m_rules.begin() + position);
}

bool RuleService::solve(const std::string &server,
			const std::string &channel,
			const std::string &origin,
			const std::string &plugin,
			const std::string &event) noexcept
{
	bool result = true;

	log::debug("rule: solving for server={}, channel={}, origin={}, plugin={}, event={}"_format(server, channel,
		   origin, plugin, event));

	int i = 0;
	for (const Rule &rule : m_rules) {
		log::debug() << "  candidate " << i++ << ":\n"
			     << "    servers: " << util::join(rule.servers().begin(), rule.servers().end()) << "\n"
			     << "    channels: " << util::join(rule.channels().begin(), rule.channels().end()) << "\n"
			     << "    origins: " << util::join(rule.origins().begin(), rule.origins().end()) << "\n"
			     << "    plugins: " << util::join(rule.plugins().begin(), rule.plugins().end()) << "\n"
			     << "    events: " << util::join(rule.events().begin(), rule.events().end()) << "\n"
			     << "    action: " << ((rule.action() == RuleAction::Accept) ? "accept" : "drop") << std::endl;

		if (rule.match(server, channel, origin, plugin, event))
			result = rule.action() == RuleAction::Accept;
	}

	return result;
}

} // !irccd
