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

#include <cassert>
#include <string>
#include <unordered_set>

#include "sysconfig.hpp"

namespace irccd {

/**
 * \brief Manage rule to activate or deactive events.
 */
class rule {
public:
    /**
     * List of criterias.
     */
    using set = std::unordered_set<std::string>;

    /**
     * \brief Rule action type.
     */
    enum class action_type {
        accept,         //!< The event is accepted (default)
        drop            //!< The event is dropped
    };

private:
    set servers_;
    set channels_;
    set origins_;
    set plugins_;
    set events_;
    action_type action_{action_type::accept};

    /*
     * Check if a set contains the value and return true if it is or return
     * true if value is empty (which means applicable).
     */
    bool match_set(const set&, const std::string&) const noexcept;

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
    rule(set servers = {},
         set channels = {},
         set nicknames = {},
         set plugins = {},
         set events = {},
         action_type action = action_type::accept);

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
    bool match(const std::string& server,
               const std::string& channel,
               const std::string& nick,
               const std::string& plugin,
               const std::string& event) const noexcept;

    /**
     * Get the action.
     *
     * \return the action
     */
    inline action_type action() const noexcept
    {
        return action_;
    }

    /**
     * Set the action.
     *
     * \pre action must be valid
     */
    inline void set_action(action_type action) noexcept
    {
        assert(action == action_type::accept || action == action_type::drop);

        action_ = action;
    }

    /**
     * Get the servers.
     *
     * \return the servers
     */
    inline const set& servers() const noexcept
    {
        return servers_;
    }

    /**
     * Overloaded function.
     *
     * \return the servers
     */
    inline set& servers() noexcept
    {
        return servers_;
    }

    /**
     * Get the channels.
     *
     * \return the channels
     */
    inline const set& channels() const noexcept
    {
        return channels_;
    }

    /**
     * Overloaded function.
     *
     * \return the channels
     */
    inline set& channels() noexcept
    {
        return channels_;
    }

    /**
     * Get the origins.
     *
     * \return the origins
     */
    inline const set& origins() const noexcept
    {
        return origins_;
    }

    /**
     * Get the plugins.
     *
     * \return the plugins
     */
    inline const set& plugins() const noexcept
    {
        return plugins_;
    }

    /**
     * Overloaded function.
     *
     * \return the plugins
     */
    inline set& plugins() noexcept
    {
        return plugins_;
    }

    /**
     * Get the events.
     *
     * \return the events
     */
    inline const set& events() const noexcept
    {
        return events_;
    }

    /**
     * Overloaded function.
     *
     * \return the events
     */
    inline set& events() noexcept
    {
        return events_;
    }
};

} // !irccd

#endif // !IRCCD_RULE_HPP
