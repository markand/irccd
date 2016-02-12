/*
 * irccd.h -- main irccd class
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

#ifndef _IRCCD_H_
#define _IRCCD_H_

#include <atomic>
#include <cassert>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

#include <irccd-config.h>

#include <logger.h>
#include <sockets.h>

#if defined(WITH_JS)
#  include "plugin.h"
#endif

#include "rule.h"
#include "server.h"
#include "transport-command.h"
#include "transport-server.h"

namespace irccd {

class Plugin;
class TransportCommand;

/**
 * Event to execute after the poll.
 */
using Event = std::function<void ()>;

/**
 * List of events.
 */
using Events = std::vector<Event>;

/**
 * Map of identities.
 */
using Identities = std::unordered_map<std::string, ServerIdentity>;

/**
 * List of rules.
 */
using Rules = std::vector<Rule>;

/**
 * @class ServerEvent
 * @brief Structure that owns several informations about an IRC event
 *
 * This structure is used to dispatch the IRC event to the plugins and the transports.
 */
class ServerEvent {
public:
	std::string server;
	std::string origin;
	std::string target;
	std::string json;
#if defined(WITH_JS)
	std::function<std::string (Plugin &)> name;
	std::function<void (Plugin &)> exec;
#endif
};

class TransportEvent {
public:
	std::string name;
	std::weak_ptr<TransportClient> client;
	std::function<std::string ()> exec;
};

/**
 * Map of servers.
 */
using Servers = std::unordered_map<std::string, std::shared_ptr<Server>>;

/**
 * Map of transport command handlers.
 */
using TransportCommands = std::unordered_map<std::string, std::unique_ptr<TransportCommand>>;

#if defined(WITH_JS)

/**
 * Map of plugins.
 */
using Plugins = std::unordered_map<std::string, std::shared_ptr<Plugin>>;

/**
 * Map of plugin configurations.
 */
using PluginConfigs = std::unordered_map<std::string, PluginConfig>;

#endif

/**
 * @class Irccd
 * @brief Irccd main instance
 *
 * This class is used as the main application event loop, it stores servers, plugins and transports.
 *
 * In a general manner, no code in irccd is thread-safe because irccd is mono-threaded except the JavaScript timer
 * API.
 *
 * If you plan to add more threads to irccd, then the simpliest and safest way to execute thread-safe code is to
 * register an event using Irccd::post function which will be called during the event loop dispatching.
 *
 * Thus, except noticed as thread-safe, no function is assumed to be.
 */
class Irccd {
private:
	template <typename T>
	using LookupTable = std::unordered_map<net::Handle, std::shared_ptr<T>>;

	/* Main loop */
	std::atomic<bool> m_running{true};

	/* Mutex for post() */
	std::mutex m_mutex;

	/* IPC */
	net::SocketTcp<net::address::Ip> m_socketServer;
	net::SocketTcp<net::address::Ip> m_socketClient;

	/* Event loop */
	Events m_events;

	/* Servers */
	Servers m_servers;

	/* Optional JavaScript plugins */
#if defined(WITH_JS)
	Plugins m_plugins;
	PluginConfigs m_pluginConf;
#endif

	/* Identities */
	Identities m_identities;

	/* Rules */
	Rules m_rules;

	/* Lookup tables */
	LookupTable<TransportClient> m_lookupTransportClients;
	LookupTable<TransportServer> m_lookupTransportServers;

	/* Transport commands handlers */
	TransportCommands m_transportCommands;

	/*
	 * Server slots
	 * ----------------------------------------------------------
	 */

	void handleServerChannelMode(std::weak_ptr<Server> server, std::string origin, std::string channel, std::string mode, std::string arg);
	void handleServerChannelNotice(std::weak_ptr<Server> server, std::string origin, std::string channel, std::string notice);
	void handleServerConnect(std::weak_ptr<Server> server);
	void handleServerInvite(std::weak_ptr<Server> server, std::string origin, std::string channel, std::string target);
	void handleServerJoin(std::weak_ptr<Server> server, std::string origin, std::string channel);
	void handleServerKick(std::weak_ptr<Server> server, std::string origin, std::string channel, std::string target, std::string reason);
	void handleServerMessage(std::weak_ptr<Server> server, std::string origin, std::string channel, std::string message);
	void handleServerMe(std::weak_ptr<Server> server, std::string origin, std::string target, std::string message);
	void handleServerMode(std::weak_ptr<Server> server, std::string origin, std::string mode);
	void handleServerNames(std::weak_ptr<Server> server, std::string channel, std::set<std::string> nicknames);
	void handleServerNick(std::weak_ptr<Server> server, std::string origin, std::string nickname);
	void handleServerNotice(std::weak_ptr<Server> server, std::string origin, std::string message);
	void handleServerPart(std::weak_ptr<Server> server, std::string origin, std::string channel, std::string reason);
	void handleServerQuery(std::weak_ptr<Server> server, std::string origin, std::string message);
	void handleServerTopic(std::weak_ptr<Server> server, std::string origin, std::string channel, std::string topic);
	void handleServerWhois(std::weak_ptr<Server> server, ServerWhois whois);

	/*
	 * Transport clients slots
	 * ----------------------------------------------------------
	 */
	void handleTransportCommand(std::weak_ptr<TransportClient>, const json::Value &);
	void handleTransportDie(std::weak_ptr<TransportClient>);

	/*
	 * Plugin timers slots
	 * ----------------------------------------------------------
	 *
	 * These handlers catch the timer signals and call the plugin function or remove the timer from the plugin.
	 */

#if defined(WITH_JS)
	void handleTimerSignal(std::weak_ptr<Plugin>, std::shared_ptr<Timer>);
	void handleTimerEnd(std::weak_ptr<Plugin>, std::shared_ptr<Timer>);
#endif

	/*
	 * Process the socket sets.
	 * ----------------------------------------------------------
	 *
	 * These functions are called after polling which sockets are ready for reading/writing.
	 */

	void processIpc(fd_set &input);
	void processTransportClients(fd_set &input, fd_set &output);
	void processTransportServers(fd_set &input);
	void processServers(fd_set &input, fd_set &output);
	void process(fd_set &setinput, fd_set &setoutput);

public:
	/**
	 * Constructor that instanciate IPC.
	 */
	Irccd();

	/**
	 * Load a configuration into irccd. Added as convenience to allow expressions like `irccd.load(Config{"foo"})`.
	 *
	 * @param config the configuration loader
	 */
	template <typename T>
	inline void load(T &&config)
	{
		config.load(*this);
	}

	/**
	 * Add an event to the queue. This will immediately signals the event loop to interrupt itself to dispatch
	 * the pending events.
	 *
	 * @param ev the event
	 * @note Thread-safe
	 */
	void post(Event ev) noexcept;

	/*
	 * This function wraps post() to iterate over all plugins to call the function and to send to all
	 * connected transport the event.
	 */
	void postServerEvent(ServerEvent) noexcept;

	/*
	 * Identity management
	 * ----------------------------------------------------------
	 *
	 * Functions to get or add new identities.
	 */

	/**
	 * Add an identity.
	 *
	 * @param identity the identity
	 * @note If the identity already exists, it is overriden
	 */
	inline void addIdentity(ServerIdentity identity) noexcept
	{
		m_identities.emplace(identity.name, std::move(identity));
	}

	/**
	 * Get an identity, if not found, the default one is used.
	 *
	 * @param name the identity name
	 * @return the identity or default one
	 */
	inline ServerIdentity findIdentity(const std::string &name) const noexcept
	{
		auto it = m_identities.find(name);

		return it == m_identities.end() ? ServerIdentity() : it->second;
	}

	/*
	 * Server management
	 * ----------------------------------------------------------
	 *
	 * Functions to get or create new servers.
	 *
	 * Servers that are added to this instance are automatically polled when run() is called.
	 */

	/**
	 * Check if a server exists.
	 *
	 * @param name the name
	 * @return true if exists
	 */
	inline bool hasServer(const std::string &name) const noexcept
	{
		return m_servers.count(name) > 0;
	}

	/**
	 * Add a new server to the application.
	 *
	 * @pre hasServer must return false
	 * @param sv the server
	 */
	void addServer(std::shared_ptr<Server> sv) noexcept;

	/**
	 * Get a server or empty one if not found
	 *
	 * @param name the server name
	 * @return the server or empty one if not found
	 */
	std::shared_ptr<Server> getServer(const std::string &name) const noexcept;

	/**
	 * Find a server by name.
	 *
	 * @param name the server name
	 * @return the server
	 * @throw std::out_of_range if the server does not exist
	 */
	std::shared_ptr<Server> requireServer(const std::string &name) const;

	/**
	 * Get the map of loaded servers.
	 *
	 * @return the servers
	 */
	inline const Servers &servers() const noexcept
	{
		return m_servers;
	}

	/**
	 * Remove a server from the irccd instance.
	 *
	 * The server if any, will be disconnected.
	 *
	 * @param name the server name
	 */
	void removeServer(const std::string &name);

	/**
	 * Remove all servers.
	 *
	 * All servers will be disconnected.
	 */
	void clearServers() noexcept;

	/*
	 * Transport management
	 * ----------------------------------------------------------
	 *
	 * Functions for adding new transport servers.
	 */

	/**
	 * Add a transport server.
	 *
	 * @param ts the transport server
	 */
	void addTransport(std::shared_ptr<TransportServer> ts);

	/**
	 * Register a new transport command.
	 *
	 * @param key the transport command name
	 */
	template <typename Cmd>
	inline void addTransportCommand(std::string key)
	{
		m_transportCommands.emplace(std::move(key), std::make_unique<Cmd>());
	}

	/*
	 * Plugin management
	 * ----------------------------------------------------------
	 *
	 * Functions for loading JavaScript plugins.
	 */

#if defined(WITH_JS)
	/**
	 * Check if a plugin is loaded.
	 *
	 * @param name the plugin id
	 * @return true if has plugin
	 */
	inline bool hasPlugin(const std::string &name) const noexcept
	{
		return m_plugins.count(name) > 0;
	}

	/**
	 * Get a plugin or empty one if not found.
	 *
	 * @param name the plugin id
	 * @return the plugin or empty one if not found
	 */
	std::shared_ptr<Plugin> getPlugin(const std::string &name) const noexcept;

	/**
	 * Find a plugin.
	 *
	 * @param name the plugin id
	 * @return the plugin
	 * @throws std::out_of_range if not found
	 */
	std::shared_ptr<Plugin> requirePlugin(const std::string &name) const;

	/**
	 * Add plugin configuration for the specified plugin.
	 *
	 * @param name
	 * @param config
	 */
	inline void addPluginConfig(std::string name, PluginConfig config)
	{
		m_pluginConf.emplace(std::move(name), std::move(config));
	}

	/**
	 * Add a loaded plugin.
	 *
	 * Plugins signals will be connected to the irccd main loop. The onLoad function will also be called and the
	 * plugin is not added on errors.
	 *
	 * @pre plugin must not be empty
	 * @param plugin the plugin
	 */
	void addPlugin(std::shared_ptr<Plugin> plugin);

	/**
	 * Load a plugin by path or by searching through directories.
	 *
	 * TODO: Move this somewhere else (e.g. Plugin::find).
	 *
	 * @param source the path or the plugin id to search
	 * @param find set to true for searching by id
	 */
	void loadPlugin(std::string name, const std::string &source, bool find);

	/**
	 * Unload a plugin and remove it.
	 *
	 * @param name the plugin id
	 */
	void unloadPlugin(const std::string &name);

	/**
	 * Reload a plugin by calling onReload.
	 *
	 * @param name the plugin name
	 * @throw std::exception on failures
	 */
	void reloadPlugin(const std::string &name);

	/**
	 * Get the map of plugins.
	 *
	 * @return the map of plugins
	 */
	inline const Plugins &plugins() const noexcept
	{
		return m_plugins;
	}

#endif // !WITH_JS

	/*
	 * Rule management
	 * ----------------------------------------------------------
	 *
	 * Functions for adding, creating new rules that are used to filter IRC events before being processed
	 * by JavaScript plugins.
	 */

	/**
	 * Append a rule.
	 *
	 * @param rule the rule to append
	 */
	inline void addRule(Rule rule)
	{
		m_rules.push_back(std::move(rule));
	}

	/**
	 * Insert a new rule at the specified position.
	 *
	 * @param rule the rule
	 * @param position the position
	 */
	inline void insertRule(Rule rule, unsigned position)
	{
		assert(position <= m_rules.size());

		m_rules.insert(m_rules.begin() + position, std::move(rule));
	}

	/**
	 * Get the list of rules.
	 *
	 * @return the list of rules
	 */
	inline const std::vector<Rule> &rules() const noexcept
	{
		return m_rules;
	}

	/**
	 * Remove a new rule from the specified position.
	 *
	 * @param rule the rule
	 * @param position the position
	 */
	inline void removeRule(unsigned position)
	{
		assert(position < m_rules.size());

		m_rules.erase(m_rules.begin() + position);
	}

	/**
	 * Loop forever by calling poll() and dispatch() indefinitely.
	 */
	void run();

	/**
	 * Poll the next events without blocking (250 ms max).
	 */
	void poll();

	/**
	 * Dispatch the pending events, usually after calling poll().
	 */
	void dispatch();

	/**
	 * Request to stop, usually from a signal.
	 */
	void stop();
};

} // !irccd

#endif // !_IRCCD_H_
