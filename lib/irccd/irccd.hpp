/*
 * irccd.hpp -- main irccd class
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

#ifndef IRCCD_HPP
#define IRCCD_HPP

/**
 * \file irccd.hpp
 * \brief Base class for irccd front end.
 */

#include <algorithm>
#include <atomic>
#include <cassert>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

#include "sockets.hpp"
#include "sysconfig.hpp"

#if defined(WITH_JS)
#  include "plugin.hpp"
#endif

#include "application.hpp"
#include "logger.hpp"
#include "rule.hpp"
#include "server.hpp"
#include "transport-server.hpp"

namespace irccd {

class InterruptService;
class Irccd;
class Plugin;
class Service;
class TransportCommand;

/**
 * \class Irccd
 * \brief Irccd main instance
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
class Irccd : public Application {
private:
	// Main loop stuff.
	std::atomic<bool> m_running{true};
	std::mutex m_mutex;
	std::vector<std::function<void (Irccd &)>> m_events;

	// Servers.
	std::vector<std::shared_ptr<Server>> m_servers;

	// Optional plugins.
#if defined(WITH_JS)
	std::vector<std::shared_ptr<Plugin>> m_plugins;
#endif

	// Rules.
	std::vector<Rule> m_rules;

	// Transports.
	std::vector<std::shared_ptr<TransportClient>> m_transportClients;
	std::vector<std::shared_ptr<TransportServer>> m_transportServers;

	// Services
	std::shared_ptr<InterruptService> m_interruptService;
	std::vector<std::shared_ptr<Service>> m_services;

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

	void processTransportClients(fd_set &input, fd_set &output);
	void processTransportServers(fd_set &input);
	void processServers(fd_set &input, fd_set &output);
	void process(fd_set &setinput, fd_set &setoutput);

public:
	/**
	 * Prepare standard services.
	 */
	Irccd();

	/**
	 * Add a generic service.
	 */
	inline void addService(std::shared_ptr<Service> service)
	{
		m_services.push_back(std::move(service));
	}

	/**
	 * Add an event to the queue. This will immediately signals the event loop to interrupt itself to dispatch
	 * the pending events.
	 *
	 * \param ev the event
	 * \note Thread-safe
	 */
	void post(std::function<void (Irccd &)> ev) noexcept;

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
	 * \param name the name
	 * \return true if exists
	 */
	inline bool hasServer(const std::string &name) const noexcept
	{
		return std::count_if(m_servers.cbegin(), m_servers.end(), [&] (const auto &sv) {
			return sv->info().name == name;
		}) > 0;
	}

	/**
	 * Add a new server to the application.
	 *
	 * \pre hasServer must return false
	 * \param sv the server
	 */
	void addServer(std::shared_ptr<Server> sv) noexcept;

	/**
	 * Get a server or empty one if not found
	 *
	 * \param name the server name
	 * \return the server or empty one if not found
	 */
	std::shared_ptr<Server> getServer(const std::string &name) const noexcept;

	/**
	 * Find a server by name.
	 *
	 * \param name the server name
	 * \return the server
	 * \throw std::out_of_range if the server does not exist
	 */
	std::shared_ptr<Server> requireServer(const std::string &name) const;

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
	 * Remove a server from the irccd instance.
	 *
	 * The server if any, will be disconnected.
	 *
	 * \param name the server name
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
	 * \param ts the transport server
	 */
	void addTransport(std::shared_ptr<TransportServer> ts);

	/**
	 * Send data to all clients.
	 *
	 * \param data the data
	 */
	void broadcast(std::string data);

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
	 * \param name the plugin id
	 * \return true if has plugin
	 */
	inline bool hasPlugin(const std::string &name) const noexcept
	{
		return std::count_if(m_plugins.cbegin(), m_plugins.cend(), [&] (const auto &plugin) {
			return plugin->name() == name;
		}) > 0;
	}

	/**
	 * Get a plugin or empty one if not found.
	 *
	 * \param name the plugin id
	 * \return the plugin or empty one if not found
	 */
	std::shared_ptr<Plugin> getPlugin(const std::string &name) const noexcept;

	/**
	 * Find a plugin.
	 *
	 * \param name the plugin id
	 * \return the plugin
	 * \throws std::out_of_range if not found
	 */
	std::shared_ptr<Plugin> requirePlugin(const std::string &name) const;

	/**
	 * Add a loaded plugin.
	 *
	 * Plugins signals will be connected to the irccd main loop. The onLoad function will also be called and the
	 * plugin is not added on errors.
	 *
	 * \pre plugin must not be empty
	 * \param plugin the plugin
	 */
	void addPlugin(std::shared_ptr<Plugin> plugin);

	/**
	 * Load a plugin by path or by searching through directories.
	 *
	 * TODO: Move this somewhere else (e.g. Plugin::find).
	 *
	 * \param source the path or the plugin id to search
	 * \param find set to true for searching by id
	 */
	void loadPlugin(std::string name, const std::string &source, bool find);

	/**
	 * Unload a plugin and remove it.
	 *
	 * \param name the plugin id
	 */
	void unloadPlugin(const std::string &name);

	/**
	 * Reload a plugin by calling onReload.
	 *
	 * \param name the plugin name
	 * \throw std::exception on failures
	 */
	void reloadPlugin(const std::string &name);

	/**
	 * Get the map of plugins.
	 *
	 * \return the map of plugins
	 */
	inline const std::vector<std::shared_ptr<Plugin>> &plugins() const noexcept
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
	 * \param rule the rule to append
	 */
	inline void addRule(Rule rule)
	{
		m_rules.push_back(std::move(rule));
	}

	/**
	 * Insert a new rule at the specified position.
	 *
	 * \param rule the rule
	 * \param position the position
	 */
	inline void insertRule(Rule rule, unsigned position)
	{
		assert(position <= m_rules.size());

		m_rules.insert(m_rules.begin() + position, std::move(rule));
	}

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
	 * Remove a new rule from the specified position.
	 *
	 * \pre position must be valid
	 * \param position the position
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

#endif // !IRCCD_HPP
