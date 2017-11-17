/*
 * service.hpp -- irccd services
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

#ifndef IRCCD_SERVICE_HPP
#define IRCCD_SERVICE_HPP

/**
 * \file service.hpp
 * \brief irccd services.
 */

#include <memory>
#include <unordered_map>
#include <vector>

#include <json.hpp>

#include "command.hpp"
#include "net.hpp"
#include "plugin.hpp"
#include "rule.hpp"
#include "server.hpp"
#include "sysconfig.hpp"

namespace irccd {

/*
 * command_service.
 * ------------------------------------------------------------------
 */

/**
 * \brief Store remote commands.
 * \ingroup services
 */
class command_service {
private:
    std::vector<std::shared_ptr<command>> commands_;

public:
    /**
     * Get all commands.
     *
     * \return the list of commands.
     */
    inline const std::vector<std::shared_ptr<command>>& commands() const noexcept
    {
        return commands_;
    }

    /**
     * Tells if a command exists.
     *
     * \param name the command name
     * \return true if the command exists
     */
    bool contains(const std::string& name) const noexcept;

    /**
     * Find a command by name.
     *
     * \param name the command name
     * \return the command or empty one if not found
     */
    std::shared_ptr<command> find(const std::string& name) const noexcept;

    /**
     * Add a command or replace existing one.
     *
     * \pre command != nullptr
     * \param command the command name
     */
    void add(std::shared_ptr<command> command);
};

/*
 * interrupt_service.
 * ------------------------------------------------------------------
 */

/**
 * \brief Interrupt irccd event loop.
 * \ingroup services
 */
class interrupt_service {
private:
    net::TcpSocket in_;
    net::TcpSocket out_;

public:
    /**
     * Prepare the socket pair.
     *
     * \throw std::runtime_error on errors
     */
    interrupt_service();

    /**
     * \copydoc Service::prepare
     */
    void prepare(fd_set& in, fd_set& out, net::Handle& max);

    /**
     * \copydoc Service::sync
     */
    void sync(fd_set& in, fd_set& out);

    /**
     * Request interruption.
     */
    void interrupt() noexcept;
};

/*
 * plugin_service.
 * ------------------------------------------------------------------
 */

/**
 * \brief Manage plugins.
 * \ingroup services
 */
class plugin_service {
private:
    irccd& irccd_;
    std::vector<std::shared_ptr<plugin>> plugins_;
    std::vector<std::unique_ptr<plugin_loader>> loaders_;

public:
    /**
     * Create the plugin service.
     *
     * \param irccd the irccd instance
     */
    plugin_service(irccd& irccd) noexcept;

    /**
     * Destroy plugins.
     */
    ~plugin_service();

    /**
     * Get the list of plugins.
     *
     * \return the list of plugins
     */
    inline const std::vector<std::shared_ptr<plugin>>& list() const noexcept
    {
        return plugins_;
    }

    /**
     * Check if a plugin is loaded.
     *
     * \param name the plugin id
     * \return true if has plugin
     */
    bool has(const std::string& name) const noexcept;

    /**
     * Get a loaded plugin or null if not found.
     *
     * \param name the plugin id
     * \return the plugin or empty one if not found
     */
    std::shared_ptr<plugin> get(const std::string& name) const noexcept;

    /**
     * Find a loaded plugin.
     *
     * \param name the plugin id
     * \return the plugin
     * \throws std::out_of_range if not found
     */
    std::shared_ptr<plugin> require(const std::string& name) const;

    /**
     * Add the specified plugin to the registry.
     *
     * \pre plugin != nullptr
     * \param plugin the plugin
     * \note the plugin is only added to the list, no action is performed on it
     */
    void add(std::shared_ptr<plugin> plugin);

    /**
     * Add a loader.
     *
     * \param loader the loader
     */
    void add_loader(std::unique_ptr<plugin_loader> loader);

    /**
     * Get the configuration for the specified plugin.
     *
     * \return the configuration
     */
    plugin_config config(const std::string& id);

    /**
     * Get the formats for the specified plugin.
     *
     * \return the formats
     */
    plugin_formats formats(const std::string& id);

    /**
     * Get the paths for the specified plugin.
     *
     * If none is defined, return the default ones.
     *
     * \return the paths
     */
    plugin_paths paths(const std::string& id);

    /**
     * Generic function for opening the plugin at the given path.
     *
     * This function will search for every pluginLoader and call open() on it,
     * the first one that success will be returned.
     *
     * \param id the plugin id
     * \param path the path to the file
     * \return the plugin or nullptr on failures
     */
    std::shared_ptr<plugin> open(const std::string& id,
                                 const std::string& path);

    /**
     * Generic function for finding a plugin.
     *
     * \param id the plugin id
     * \return the plugin or nullptr on failures
     */
    std::shared_ptr<plugin> find(const std::string& id);

    /**
     * Convenient wrapper that loads a plugin, call onLoad and add it to the
     * registry.
     *
     * Any errors are printed using logger.
     *
     * \param name the name
     * \param path the optional path (searched if empty)
     */
    void load(std::string name, std::string path = "");

    /**
     * Unload a plugin and remove it.
     *
     * \param name the plugin id
     */
    void unload(const std::string& name);

    /**
     * Reload a plugin by calling onReload.
     *
     * \param name the plugin name
     * \throw std::exception on failures
     */
    void reload(const std::string& name);
};

/*
 * rule_service.
 * ------------------------------------------------------------------
 */

/**
 * \brief Store and solve rules.
 * \ingroup services
 */
class rule_service {
private:
    std::vector<rule> rules_;

public:
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
};

/*
 * server_service.
 * ------------------------------------------------------------------
 */

/**
 * \brief Manage IRC servers.
 * \ingroup services
 */
class server_service {
private:
    irccd& irccd_;
    std::vector<std::shared_ptr<server>> servers_;

    void handle_channel_mode(const channel_mode_event&);
    void handle_channel_notice(const channel_notice_event&);
    void handle_connect(const connect_event&);
    void handle_invite(const invite_event&);
    void handle_join(const join_event&);
    void handle_kick(const kick_event&);
    void handle_message(const message_event&);
    void handle_me(const me_event&);
    void handle_mode(const mode_event&);
    void handle_names(const names_event&);
    void handle_nick(const nick_event&);
    void handle_notice(const notice_event&);
    void handle_part(const part_event&);
    void handle_query(const query_event&);
    void handle_topic(const topic_event&);
    void handle_whois(const whois_event&);

public:
    /**
     * Create the server service.
     */
    server_service(irccd& instance);

    /**
     * \copydoc Service::prepare
     */
    void prepare(fd_set& in, fd_set& out, net::Handle& max);

    /**
     * \copydoc Service::sync
     */
    void sync(fd_set& in, fd_set& out);

    /**
     * Get the list of servers
     *
     * \return the servers
     */
    inline const std::vector<std::shared_ptr<server>>& servers() const noexcept
    {
        return servers_;
    }

    /**
     * Check if a server exists.
     *
     * \param name the name
     * \return true if exists
     */
    bool has(const std::string& name) const noexcept;

    /**
     * Add a new server to the application.
     *
     * \pre hasServer must return false
     * \param sv the server
     */
    void add(std::shared_ptr<server> sv);

    /**
     * Get a server or empty one if not found
     *
     * \param name the server name
     * \return the server or empty one if not found
     */
    std::shared_ptr<server> get(const std::string& name) const noexcept;

    /**
     * Find a server by name.
     *
     * \param name the server name
     * \return the server
     * \throw std::out_of_range if the server does not exist
     */
    std::shared_ptr<server> require(const std::string& name) const;

    /**
     * Remove a server from the irccd instance.
     *
     * The server if any, will be disconnected.
     *
     * \param name the server name
     */
    void remove(const std::string& name);

    /**
     * Remove all servers.
     *
     * All servers will be disconnected.
     */
    void clear() noexcept;
};

} // !irccd

#endif // !IRCCD_SERVICE_HPP
