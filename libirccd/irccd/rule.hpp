/*
 * rule.hpp -- rule for server and channels
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

#ifndef IRCCD_RULE_HPP
#define IRCCD_RULE_HPP

/**
 * \file rule.hpp
 * \brief Rule description
 */

#include <sstream>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "sysconfig.hpp"

namespace irccd {

/**
 * List of criterias.
 */
using RuleSet = std::unordered_set<std::string>;

/**
 * \enum RuleAction
 * \brief Rule action
 */
enum class RuleAction {
    Accept,         //!< The event is accepted (default)
    Drop            //!< The event is dropped
};

/**
 * \brief Manage rule to activate or deactive events.
 */
class Rule {
private:
    RuleSet m_servers;
    RuleSet m_channels;
    RuleSet m_origins;
    RuleSet m_plugins;
    RuleSet m_events;
    RuleAction m_action{RuleAction::Accept};

    /*
     * Check if a map contains the value and return true if it is
     * or return true if value is empty (which means applicable).
     */
    bool matchMap(const RuleSet &map, const std::string &value) const noexcept;

public:
    /**
     * Rule constructor.
     *
     * \param servers the server list
     * \param channels the channels
     * \param nicknames the nicknames
     * \param plugins the plugins
     * \param events the events
     * \param action the rule action
     * \throw std::invalid_argument if events are invalid
     */
    IRCCD_EXPORT Rule(RuleSet servers = RuleSet{},
                      RuleSet channels = RuleSet{},
                      RuleSet nicknames = RuleSet{},
                      RuleSet plugins = RuleSet{},
                      RuleSet events = RuleSet{},
                      RuleAction action = RuleAction::Accept);

    /**
     * Check if that rule apply for the given criterias.
     *
     * \param server the server
     * \param channel the channel
     * \param nick the origin
     * \param plugin the plugin
     * \param event the event
     * \return true if match
     */
    IRCCD_EXPORT bool match(const std::string &server,
                            const std::string &channel,
                            const std::string &nick,
                            const std::string &plugin,
                            const std::string &event) const noexcept;

    /**
     * Get the action.
     *
     * \return the action
     */
    IRCCD_EXPORT RuleAction action() const noexcept;

    /**
     * Set the action.
     *
     * \pre action must be valid
     */
    IRCCD_EXPORT void setAction(RuleAction action) noexcept;

    /**
     * Get the servers.
     *
     * \return the servers
     */
    IRCCD_EXPORT const RuleSet &servers() const noexcept;

    /**
     * Overloaded function.
     *
     * \return the servers
     */
    inline RuleSet &servers() noexcept
    {
        return m_servers;
    }

    /**
     * Get the channels.
     *
     * \return the channels
     */
    IRCCD_EXPORT const RuleSet &channels() const noexcept;


    /**
     * Overloaded function.
     *
     * \return the channels
     */
    inline RuleSet &channels() noexcept
    {
        return m_channels;
    }

    /**
     * Get the origins.
     *
     * \return the origins
     */
    IRCCD_EXPORT const RuleSet &origins() const noexcept;

    /**
     * Get the plugins.
     *
     * \return the plugins
     */
    IRCCD_EXPORT const RuleSet &plugins() const noexcept;


    /**
     * Overloaded function.
     *
     * \return the plugins
     */
    inline RuleSet &plugins() noexcept
    {
        return m_plugins;
    }

    /**
     * Get the events.
     *
     * \return the events
     */
    IRCCD_EXPORT const RuleSet &events() const noexcept;


    /**
     * Overloaded function.
     *
     * \return the events
     */
    inline RuleSet& events() noexcept
    {
        return m_events;
    }
};

} // !irccd

#endif // !IRCCD_RULE_HPP
