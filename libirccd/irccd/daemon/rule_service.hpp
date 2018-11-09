/*
 * rule_service.hpp -- rule service
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

#ifndef IRCCD_DAEMON_RULE_SERVICE_HPP
#define IRCCD_DAEMON_RULE_SERVICE_HPP

/**
 * \file rule_service.hpp
 * \brief Rule service.
 */

#include <vector>

#include <json.hpp>

#include "rule.hpp"

namespace irccd {

class config;
class irccd;

/**
 * \brief Store and solve rules.
 * \ingroup services
 */
class rule_service {
private:
	irccd& irccd_;
	std::vector<rule> rules_;

public:
	/**
	 * Create the rule service.
	 *
	 * \param instance the irccd instance
	 */
	rule_service(irccd& instance);

	/**
	 * Get the list of rules.
	 *
	 * \return the list of rules
	 */
	auto list() const noexcept -> const std::vector<rule>&;

	/**
	 * Append a rule.
	 *
	 * \param rule the rule to append
	 */
	void add(rule rule);

	/**
	 * Insert a new rule at the specified position.
	 *
	 * \param rule the rule
	 * \param position the position
	 */
	void insert(rule rule, unsigned position);

	/**
	 * Remove a new rule from the specified position.
	 *
	 * \pre position must be valid
	 * \param position the position
	 */
	void remove(unsigned position);

	/**
	 * Get a rule at the specified index or throw an exception if not found.
	 *
	 * \param position the position
	 * \return the rule
	 * \throw std::out_of_range if position is invalid
	 */
	auto require(unsigned position) const -> const rule&;

	/**
	 * Overloaded function.
	 *
	 * \copydoc require
	 */
	auto require(unsigned position) -> rule&;

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
	auto solve(std::string_view server,
	           std::string_view channel,
	           std::string_view origin,
	           std::string_view plugin,
	           std::string_view event) noexcept -> bool;

	/**
	 * Load rules from the configuration.
	 *
	 * \param cfg the config
	 */
	void load(const config& cfg) noexcept;
};

namespace logger {

template <typename T>
struct loggable_traits;

/**
 * \brief Specialization for rule.
 */
template <>
struct loggable_traits<rule> {
	/**
	 * Get 'rule' category.
	 *
	 * \param rule the rule
	 * \return rule
	 */
	static auto get_category(const rule& rule) -> std::string_view;

	/**
	 * Returns nothing for the moment.
	 *
	 * \param rule the rule
	 * \return nothing
	 */
	static auto get_component(const rule& rule) -> std::string_view;
};

} // !logger

} // !irccd

#endif // !IRCCD_DAEMON_RULE_SERVICE_HPP
