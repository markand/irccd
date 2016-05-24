/*
 * service-rule.hpp -- store and solve rules
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

#ifndef IRCCD_SERVICE_RULE_HPP
#define IRCCD_SERVICE_RULE_HPP

/**
 * \file service-rule.hpp
 * \brief Store and solve rules.
 */

#include <vector>

#include "rule.hpp"

namespace irccd {

/**
 * \brief Store and solve rules.
 */
class RuleService {
private:
	std::vector<Rule> m_rules;

public:
	/**
	 * Get the list of rules.
	 *
	 * \return the list of rules
	 */
	inline const std::vector<Rule> &rules() const noexcept
	{
		return m_rules;
	}

	/**
	 * Get the number of rules.
	 *
	 * \return the number of rules
	 */
	inline unsigned length() const noexcept
	{
		return m_rules.size();
	}

	/**
	 * Append a rule.
	 *
	 * \param rule the rule to append
	 */
	IRCCD_EXPORT void add(Rule rule);

	/**
	 * Insert a new rule at the specified position.
	 *
	 * \param rule the rule
	 * \param position the position
	 */
	IRCCD_EXPORT void insert(Rule rule, unsigned position);

	/**
	 * Remove a new rule from the specified position.
	 *
	 * \pre position must be valid
	 * \param position the position
	 */
	IRCCD_EXPORT void remove(unsigned position);

	/**
	 * Resolve the action to execute with the specified list of rules.
	 *
	 * \param server the server name
	 * \param channel the channel name
	 * \param origin the origin
	 * \param plugin the plugin name
	 * \param event the event name (e.g onKick)
	 * \return true if the plugin must be called
	 */
	IRCCD_EXPORT bool solve(const std::string &server,
				const std::string &channel,
				const std::string &origin,
				const std::string &plugin,
				const std::string &event) noexcept;
};

} // !irccd

#endif // !IRCCD_SERVICE_RULE_HPP
