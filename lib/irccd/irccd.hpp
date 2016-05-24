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

#include "application.hpp"
#include "sysconfig.hpp"

namespace irccd {

class InterruptService;
class Irccd;
class ModuleService;
class PluginService;
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

	// Services.
	std::shared_ptr<InterruptService> m_interruptService;
	std::shared_ptr<ServerService> m_serverService;
	std::shared_ptr<TransportService> m_transportService;
	std::shared_ptr<RuleService> m_ruleService;
	std::shared_ptr<ModuleService> m_moduleService;
	std::shared_ptr<PluginService> m_pluginService;
	std::vector<std::shared_ptr<Service>> m_services;

	// Not copyable and not movable because services has references to irccd.
	Irccd(const Irccd &) = delete;
	Irccd(Irccd &&) = delete;

	Irccd &operator=(const Irccd &) = delete;
	Irccd &operator=(Irccd &&) = delete;

public:
	/**
	 * Prepare standard services.
	 */
	IRCCD_EXPORT Irccd();

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
	 * Access the module service.
	 *
	 * \return the service
	 */
	inline ModuleService &moduleService() noexcept
	{
		return *m_moduleService;
	}

	/**
	 * Access the plugin service.
	 *
	 * \return the service
	 */
	inline PluginService &pluginService() noexcept
	{
		return *m_pluginService;
	}

	/**
	 * Add an event to the queue. This will immediately signals the event loop to interrupt itself to dispatch
	 * the pending events.
	 *
	 * \param ev the event
	 * \note Thread-safe
	 */
	IRCCD_EXPORT void post(std::function<void (Irccd &)> ev) noexcept;

	/**
	 * Loop forever by calling poll() and dispatch() indefinitely.
	 */
	IRCCD_EXPORT void run();

	/**
	 * Poll the next events without blocking (250 ms max).
	 */
	IRCCD_EXPORT void poll();

	/**
	 * Dispatch the pending events, usually after calling poll().
	 */
	IRCCD_EXPORT void dispatch();

	/**
	 * Request to stop, usually from a signal.
	 */
	IRCCD_EXPORT void stop();
};

} // !irccd

#endif // !IRCCD_HPP
