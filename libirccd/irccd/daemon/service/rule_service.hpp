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

#include <irccd/daemon/rule.hpp>

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
     */
    rule_service(irccd& instance);

    /**
     * Get the list of rules.
     *
     * \return the list of rules
     */
    inline const std::vector<rule>& list() const noexcept
    {
        return rules_;
    }

    /**
     * Get the number of rules.
     *
     * \return the number of rules
     */
    inline std::size_t length() const noexcept
    {
        return rules_.size();
    }

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
    const rule& require(unsigned position) const;

    /**
     * Overloaded function.
     *
     * \copydoc require
     */
    rule& require(unsigned position);

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
    bool solve(const std::string& server,
               const std::string& channel,
               const std::string& origin,
               const std::string& plugin,
               const std::string& event) noexcept;

    /**
     * Load rules from the configuration.
     *
     * \param cfg the config
     */
    void load(const config& cfg) noexcept;
};

} // !irccd

#endif // !IRCCD_DAEMON_RULE_SERVICE_HPP
