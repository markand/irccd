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
 * \brief Irccd services.
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
 * CommandService.
 * ------------------------------------------------------------------
 */

/**
 * \brief Store remote commands.
 * \ingroup services
 */
class CommandService {
private:
    std::vector<std::shared_ptr<Command>> m_commands;

public:
    /**
     * Get all commands.
     *
     * \return the list of commands.
     */
    inline const std::vector<std::shared_ptr<Command>> &commands() const noexcept
    {
        return m_commands;
    }

    /**
     * Tells if a command exists.
     *
     * \param name the command name
     * \return true if the command exists
     */
    IRCCD_EXPORT bool contains(const std::string &name) const noexcept;

    /**
     * Find a command by name.
     *
     * \param name the command name
     * \return the command or empty one if not found
     */
    IRCCD_EXPORT std::shared_ptr<Command> find(const std::string &name) const noexcept;

    /**
     * Add a command or replace existing one.
     *
     * \pre command != nullptr
     * \param command the command name
     */
    IRCCD_EXPORT void add(std::shared_ptr<Command> command);
};

/*
 * InterruptService.
 * ------------------------------------------------------------------
 */

/**
 * \brief Interrupt irccd event loop.
 * \ingroup services
 */
class InterruptService {
private:
    net::TcpSocket m_in;
    net::TcpSocket m_out;

public:
    /**
     * Prepare the socket pair.
     *
     * \throw std::runtime_error on errors
     */
    IRCCD_EXPORT InterruptService();

    /**
     * \copydoc Service::prepare
     */
    IRCCD_EXPORT void prepare(fd_set &in, fd_set &out, net::Handle &max);

    /**
     * \copydoc Service::sync
     */
    IRCCD_EXPORT void sync(fd_set &in, fd_set &out);

    /**
     * Request interruption.
     */
    IRCCD_EXPORT void interrupt() noexcept;
};

/*
 * PluginService.
 * ------------------------------------------------------------------
 */

/**
 * \brief Manage plugins.
 * \ingroup services
 */
class PluginService {
private:
    Irccd &m_irccd;
    std::vector<std::shared_ptr<Plugin>> m_plugins;
    std::vector<std::unique_ptr<PluginLoader>> m_loaders;
    std::unordered_map<std::string, PluginConfig> m_config;
    std::unordered_map<std::string, PluginFormats> m_formats;

public:
    /**
     * Create the plugin service.
     *
     * \param irccd the irccd instance
     */
    IRCCD_EXPORT PluginService(Irccd &irccd) noexcept;

    /**
     * Destroy plugins.
     */
    IRCCD_EXPORT ~PluginService();

    /**
     * Get the list of plugins.
     *
     * \return the list of plugins
     */
    inline const std::vector<std::shared_ptr<Plugin>> &list() const noexcept
    {
        return m_plugins;
    }

    /**
     * Check if a plugin is loaded.
     *
     * \param name the plugin id
     * \return true if has plugin
     */
    IRCCD_EXPORT bool has(const std::string &name) const noexcept;

    /**
     * Get a loaded plugin or null if not found.
     *
     * \param name the plugin id
     * \return the plugin or empty one if not found
     */
    IRCCD_EXPORT std::shared_ptr<Plugin> get(const std::string &name) const noexcept;

    /**
     * Find a loaded plugin.
     *
     * \param name the plugin id
     * \return the plugin
     * \throws std::out_of_range if not found
     */
    IRCCD_EXPORT std::shared_ptr<Plugin> require(const std::string &name) const;

    /**
     * Add the specified plugin to the registry.
     *
     * \pre plugin != nullptr
     * \param plugin the plugin
     * \note the plugin is only added to the list, no action is performed on it
     */
    IRCCD_EXPORT void add(std::shared_ptr<Plugin> plugin);

    /**
     * Add a loader.
     *
     * \param loader the loader
     */
    IRCCD_EXPORT void addLoader(std::unique_ptr<PluginLoader> loader);

    /**
     * Configure a plugin.
     *
     * If the plugin is already loaded, its configuration is updated.
     *
     * \param name the plugin name
     * \param config the new configuration
     */
    IRCCD_EXPORT void setConfig(const std::string &name, PluginConfig config);

    /**
     * Get a configuration for a plugin.
     *
     * \param name the plugin name
     * \return the configuration or default one if not found
     */
    IRCCD_EXPORT PluginConfig config(const std::string &name) const;

    /**
     * Add formatting for a plugin.
     *
     * \param name the plugin name
     * \param formats the formats
     */
    IRCCD_EXPORT void setFormats(const std::string &name, PluginFormats formats);

    /**
     * Get formats for a plugin.
     *
     * \param name the plugin name
     * \return the formats
     */
    IRCCD_EXPORT PluginFormats formats(const std::string &name) const;

    /**
     * Generic function for opening the plugin at the given path.
     *
     * This function will search for every PluginLoader and call open() on it,
     * the first one that success will be returned.
     *
     * \param id the plugin id
     * \param path the path to the file
     * \return the plugin or nullptr on failures
     */
    IRCCD_EXPORT std::shared_ptr<Plugin> open(const std::string &id,
                                              const std::string &path);

    /**
     * Generic function for finding a plugin.
     *
     * \param id the plugin id
     * \return the plugin or nullptr on failures
     */
    IRCCD_EXPORT std::shared_ptr<Plugin> find(const std::string &id);

    /**
     * Convenient wrapper that loads a plugin, call onLoad and add it to the
     * registry.
     *
     * Any errors are printed using logger.
     *
     * \param name the name
     * \param path the optional path (searched if empty)
     */
    IRCCD_EXPORT void load(std::string name, std::string path = "");

    /**
     * Unload a plugin and remove it.
     *
     * \param name the plugin id
     */
    IRCCD_EXPORT void unload(const std::string &name);

    /**
     * Reload a plugin by calling onReload.
     *
     * \param name the plugin name
     * \throw std::exception on failures
     */
    IRCCD_EXPORT void reload(const std::string &name);
};

/*
 * RuleService.
 * ------------------------------------------------------------------
 */

/**
 * \brief Store and solve rules.
 * \ingroup services
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
    inline const std::vector<Rule> &list() const noexcept
    {
        return m_rules;
    }

    /**
     * Get the number of rules.
     *
     * \return the number of rules
     */
    inline std::size_t length() const noexcept
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
     * Get a rule at the specified index or throw an exception if not found.
     *
     * \param position the position
     * \return the rule
     * \throw std::out_of_range if position is invalid
     */
    IRCCD_EXPORT const Rule &require(unsigned position) const;

    /**
     * Overloaded function.
     *
     * \copydoc require
     */
    IRCCD_EXPORT Rule& require(unsigned position);

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

/*
 * ServerService.
 * ------------------------------------------------------------------
 */

/**
 * \brief Manage IRC servers.
 * \ingroup services
 */
class ServerService {
private:
    Irccd &m_irccd;
    std::vector<std::shared_ptr<Server>> m_servers;

    void handleChannelMode(const ChannelModeEvent &);
    void handleChannelNotice(const ChannelNoticeEvent &);
    void handleConnect(const ConnectEvent &);
    void handleInvite(const InviteEvent &);
    void handleJoin(const JoinEvent &);
    void handleKick(const KickEvent &);
    void handleMessage(const MessageEvent &);
    void handleMe(const MeEvent &);
    void handleMode(const ModeEvent &);
    void handleNames(const NamesEvent &);
    void handleNick(const NickEvent &);
    void handleNotice(const NoticeEvent &);
    void handlePart(const PartEvent &);
    void handleQuery(const QueryEvent &);
    void handleTopic(const TopicEvent &);
    void handleWhois(const WhoisEvent &);

public:
    /**
     * Create the server service.
     */
    IRCCD_EXPORT ServerService(Irccd &instance);

    /**
     * \copydoc Service::prepare
     */
    IRCCD_EXPORT void prepare(fd_set &in, fd_set &out, net::Handle &max);

    /**
     * \copydoc Service::sync
     */
    IRCCD_EXPORT void sync(fd_set &in, fd_set &out);

    /**
     * Get the list of servers
     *
     * \return the servers
     */
    inline const std::vector<std::shared_ptr<Server>> &servers() const noexcept
    {
        return m_servers;
    }

    /**
     * Check if a server exists.
     *
     * \param name the name
     * \return true if exists
     */
    IRCCD_EXPORT bool has(const std::string &name) const noexcept;

    /**
     * Add a new server to the application.
     *
     * \pre hasServer must return false
     * \param sv the server
     */
    IRCCD_EXPORT void add(std::shared_ptr<Server> sv);

    /**
     * Get a server or empty one if not found
     *
     * \param name the server name
     * \return the server or empty one if not found
     */
    IRCCD_EXPORT std::shared_ptr<Server> get(const std::string &name) const noexcept;

    /**
     * Find a server by name.
     *
     * \param name the server name
     * \return the server
     * \throw std::out_of_range if the server does not exist
     */
    IRCCD_EXPORT std::shared_ptr<Server> require(const std::string &name) const;

    /**
     * Remove a server from the irccd instance.
     *
     * The server if any, will be disconnected.
     *
     * \param name the server name
     */
    IRCCD_EXPORT void remove(const std::string &name);

    /**
     * Remove all servers.
     *
     * All servers will be disconnected.
     */
    IRCCD_EXPORT void clear() noexcept;
};

/*
 * TransportService.
 * ------------------------------------------------------------------
 */

class TransportServer;
class TransportClient;

/**
 * \brief manage transport servers and clients.
 * \ingroup services
 */
class TransportService {
private:
    Irccd &m_irccd;

    std::vector<std::shared_ptr<TransportServer>> m_servers;
    std::vector<std::shared_ptr<TransportClient>> m_clients;

    void handleCommand(std::weak_ptr<TransportClient>, const nlohmann::json &);
    void handleDie(std::weak_ptr<TransportClient>);

public:
    /**
     * Create the transport service.
     *
     * \param irccd the irccd instance
     */
    IRCCD_EXPORT TransportService(Irccd &irccd) noexcept;

    /**
     * \copydoc Service::prepare
     */
    IRCCD_EXPORT void prepare(fd_set &in, fd_set &out, net::Handle &max);

    /**
     * \copydoc Service::sync
     */
    IRCCD_EXPORT void sync(fd_set &in, fd_set &out);

    /**
     * Add a transport server.
     *
     * \param ts the transport server
     */
    IRCCD_EXPORT void add(std::shared_ptr<TransportServer> ts);

    /**
     * Send data to all clients.
     *
     * \pre object.is_object()
     * \param object the json object
     */
    IRCCD_EXPORT void broadcast(const nlohmann::json &object);
};

} // !irccd

#endif // !IRCCD_SERVICE_HPP
