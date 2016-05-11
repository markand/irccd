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
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

#include "sysconfig.hpp"

#if defined(WITH_JS)
#  include "plugin.hpp"
#endif

#include "application.hpp"

namespace irccd {

class InterruptService;
class Irccd;
class Plugin;
class RuleService;
class ServerService;
class Service;
class TransportService;

/**
 * \class Irccd
 * \brief Irccd main instance.
 */
class Irccd : public Application {
private:
	// Main loop stuff.
	std::atomic<bool> m_running{true};
	std::mutex m_mutex;
	std::vector<std::function<void (Irccd &)>> m_events;

	// Optional plugins.
#if defined(WITH_JS)
	std::vector<std::shared_ptr<Plugin>> m_plugins;
#endif

	// Services.
	std::shared_ptr<InterruptService> m_interruptService;
	std::shared_ptr<ServerService> m_serverService;
	std::shared_ptr<TransportService> m_transportService;
	std::shared_ptr<RuleService> m_ruleService;
	std::vector<std::shared_ptr<Service>> m_services;

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

	// Not copyable and not movable because services has references to irccd.
	Irccd(const Irccd &) = delete;
	Irccd(Irccd &&) = delete;

	Irccd &operator=(const Irccd &) = delete;
	Irccd &operator=(Irccd &&) = delete;

public:
	/**
	 * Prepare standard services.
	 */
	Irccd();

	/**
	 * Add a generic service.
	 *
	 * \param service the service
	 */
	inline void addService(std::shared_ptr<Service> service)
	{
		m_services.push_back(std::move(service));
	}

	/**
	 * Access the server service.
	 *
	 * \return the service
	 */
	inline ServerService &serverService() noexcept
	{
		return *m_serverService;
	}

	/**
	 * Access the transport service.
	 *
	 * \return the service
	 */
	inline TransportService &transportService() noexcept
	{
		return *m_transportService;
	}

	/**
	 * Access the rule service.
	 *
	 * \return the service
	 */
	inline RuleService &ruleService() noexcept
	{
		return *m_ruleService;
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
