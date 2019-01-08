/*
 * rule.hpp -- rule for server and channels
 *
 * Copyright (c) 2013-2019 David Demelier <markand@malikania.fr>
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

#ifndef IRCCD_DAEMON_RULE_HPP
#define IRCCD_DAEMON_RULE_HPP

/**
 * \file rule.hpp
 * \brief Rule description
 */

#include <irccd/sysconfig.hpp>

#include <cassert>
#include <set>
#include <string>
#include <system_error>

namespace irccd::daemon {

/**
 * \brief Manage rule to activate or deactive events.
 */
struct rule {
	/**
	 * List of criterias.
	 */
	using set = std::set<std::string>;

	/**
	 * \brief Rule action type.
	 */
	enum class action_type {
		accept,         //!< The event is accepted (default)
		drop            //!< The event is dropped
	};

	set servers;            //!< The list of servers
	set channels;           //!< The list of channels
	set origins;            //!< The list of originators
	set plugins;            //!< The list of plugins
	set events;             //!< The list of events

	/**
	 * The action.
	 */
	action_type action{action_type::accept};

	/**
	 * Check if a set contains the value and return true if it is or return
	 * true if value is empty (which means applicable).
	 *
	 * \param set the set to test
	 * \param value the value
	 * \return true if match
	 */
	auto match_set(const set& set, const std::string& value) const noexcept -> bool;

	/**
	 * Check if that rule apply for the given criterias.
	 *
	 * \param server the server
	 * \param channel the channel
	 * \param origin the origin
	 * \param plugin the plugin
	 * \param event the event
	 * \return true if match
	 */
	auto match(std::string_view server,
	           std::string_view channel,
	           std::string_view origin,
	           std::string_view plugin,
	           std::string_view event) const noexcept -> bool;
};

/**
 * \brief Rule error.
 */
class rule_error : public std::system_error {
public:
	/**
	 * \brief Rule related errors.
	 */
	enum error {
		//!< No error.
		no_error = 0,

		//!< Invalid action given.
		invalid_action,

		//!< Invalid rule index.
		invalid_index,
	};

	/**
	 * Inherited constructors.
	 */
	using system_error::system_error;
};

/**
 * Get the rule error category singleton.
 *
 * \return the singleton
 */
auto rule_category() -> const std::error_category&;

/**
 * Create a std::error_code from rule_error::error enum.
 *
 * \param e the error code
 * \return the error code
 */
auto make_error_code(rule_error::error e) -> std::error_code;

} // !irccd::daemon

/**
 * \cond IRCCD_HIDDEN_SYMBOLS
 */

namespace std {

template <>
struct is_error_code_enum<irccd::daemon::rule_error::error> : public std::true_type {
};

} // !std

/**
 * \endcond
 */

#endif // !IRCCD_DAEMON_RULE_HPP
