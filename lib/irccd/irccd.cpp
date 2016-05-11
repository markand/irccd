/*
 * irccd.cpp -- main irccd class
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

#include <algorithm>
#include <stdexcept>

#include <format.h>

#include "fs.hpp"
#include "irccd.hpp"
#include "logger.hpp"
#include "path.hpp"
#include "service-interrupt.hpp"
#include "service-server.hpp"
#include "service-transport.hpp"
#include "util.hpp"

using namespace std;
using namespace std::placeholders;
using namespace std::string_literals;

using namespace fmt::literals;

namespace irccd {

Irccd::Irccd()
	: m_interruptService(std::make_shared<InterruptService>())
	, m_serverService(std::make_shared<ServerService>(*this))
	, m_transportService(std::make_shared<TransportService>(*this))
{
	m_services.push_back(m_interruptService);
	m_services.push_back(m_serverService);
	m_services.push_back(m_transportService);
}

void Irccd::post(std::function<void (Irccd &)> ev) noexcept
{
	std::lock_guard<mutex> lock(m_mutex);

	m_events.push_back(move(ev));
	m_interruptService->interrupt();
}

#if defined(WITH_JS)

std::shared_ptr<Plugin> Irccd::getPlugin(const std::string &name) const noexcept
{
	auto it = std::find_if(m_plugins.begin(), m_plugins.end(), [&] (const auto &plugin) {
		return plugin->name() == name;
	});

	if (it == m_plugins.end()) {
		return nullptr;
	}

	return *it;
}

std::shared_ptr<Plugin> Irccd::requirePlugin(const std::string &name) const
{
	auto plugin = getPlugin(name);

	if (!plugin) {
		throw std::invalid_argument("plugin {} not found"_format(name));
	}

	return plugin;
}

void Irccd::addPlugin(std::shared_ptr<Plugin> plugin)
{
	std::weak_ptr<Plugin> ptr(plugin);

	plugin->onTimerSignal.connect(std::bind(&Irccd::handleTimerSignal, this, ptr, _1));
	plugin->onTimerEnd.connect(std::bind(&Irccd::handleTimerEnd, this, ptr, _1));

	/* Store reference to irccd */
	duk::putGlobal(plugin->context(), "\xff""\xff""irccd", duk::RawPointer<Irccd>{this});

	/* Initial load now */
	try {
		plugin->onLoad();
		m_plugins.push_back(std::move(plugin));
	} catch (const std::exception &ex) {
		log::warning("plugin {}: {}"_format(plugin->name(), ex.what()));
	}
}

void Irccd::loadPlugin(std::string name, const std::string &source, bool find)
{
	// TODO: change with Plugin::find
	auto it = std::find_if(m_plugins.begin(), m_plugins.end(), [&] (const auto &plugin) {
		return plugin->name() == name;
	});

	if (it != m_plugins.end()) {
		throw std::invalid_argument("plugin already loaded");
	}

	std::vector<string> paths;
	std::shared_ptr<Plugin> plugin;

	if (find) {
		for (const std::string &dir : path::list(path::PathPlugins)) {
			paths.push_back(dir + source + ".js");
		}
	} else {
		paths.push_back(source);
	}

	/* Iterate over all paths */
	log::info("plugin {}: trying to load:"_format(name));

	for (const auto &path : paths) {
		log::info() << "  from " << path << std::endl;

		try {
			plugin = std::make_shared<Plugin>(name, path /*, m_pluginConf[name] */);
			break;
		} catch (const std::exception &ex) {
			log::info(fmt::format("    error: {}", ex.what()));
		}
	}

	if (plugin) {
		addPlugin(std::move(plugin));
	} else {
		throw std::runtime_error("no suitable plugin found");
	}
}

void Irccd::reloadPlugin(const std::string &name)
{
	auto plugin = getPlugin(name);

	if (plugin) {
		plugin->onReload();
	}
}

void Irccd::unloadPlugin(const std::string &name)
{
	auto it = std::find_if(m_plugins.begin(), m_plugins.end(), [&] (const auto &plugin) {
		return plugin->name() == name;
	});

	if (it != m_plugins.end()) {
		(*it)->onUnload();
		m_plugins.erase(it);
	}
}

#endif // !WITH_JS

/*
 * Timer slots
 * ------------------------------------------------------------------
 *
 * These slots are called from timer threads.
 */

#if defined(WITH_JS)

void Irccd::handleTimerSignal(std::weak_ptr<Plugin> ptr, std::shared_ptr<Timer> timer)
{
	post([this, ptr, timer] (Irccd &) {
		auto plugin = ptr.lock();

		if (!plugin) {
			return;
		}

		auto &ctx = plugin->context();

		duk::StackAssert sa(ctx);

		// TODO: improve this
		try {
			duk::getGlobal<void>(ctx, "\xff""\xff""timer-" + std::to_string(reinterpret_cast<std::intptr_t>(timer.get())));
			duk::pcall(ctx, 0);
			duk::pop(ctx);
		} catch (const std::exception &) {
		}
	});
}

void Irccd::handleTimerEnd(std::weak_ptr<Plugin> ptr, std::shared_ptr<Timer> timer)
{
	post([this, ptr, timer] (Irccd &) {
		auto plugin = ptr.lock();

		if (plugin) {
			log::debug() << "timer: finished, removing from plugin `" << plugin->name() << "'" << std::endl;
			plugin->removeTimer(timer);
		}
	});
}

#endif

void Irccd::run()
{
	while (m_running) {
		poll();
		dispatch();
	}
}

void Irccd::poll()
{
	fd_set setinput;
	fd_set setoutput;
	net::Handle max = 0;

	FD_ZERO(&setinput);
	FD_ZERO(&setoutput);

	for (const auto &service : m_services) {
		service->prepare(setinput, setoutput, max);
	}

	// Do the selection.
	struct timeval tv;

	tv.tv_sec = 5;
	tv.tv_usec = 250000;

	int error = select(max + 1, &setinput, &setoutput, nullptr, &tv);

	// Skip anyway if requested to stop
	if (!m_running) {
		return;
	}

	// Skip on error.
	if (error < 0 && errno != EINTR) {
		log::warning() << "irccd: " << net::error(error) << endl;
		return;
	}

	// Process after selection.
	for (const auto &service : m_services) {
		service->sync(setinput, setoutput);
	}
}

void Irccd::dispatch()
{
	/*
	 * Make a copy because the events can add other events while we are iterating it. Also lock because the timers
	 * may alter these events too.
	 */
	std::vector<std::function<void (Irccd &)>> copy;

	{
		std::lock_guard<mutex> lock(m_mutex);

		copy = move(m_events);
		m_events.clear();
	}

	if (copy.size() > 0) {
		log::debug() << "irccd: dispatching " << copy.size() << " event" << (copy.size() > 1 ? "s" : "") << endl;
	}

	for (auto &ev : copy) {
		ev(*this);
	}
}

void Irccd::stop()
{
	log::debug() << "irccd: requesting to stop now" << endl;

	m_running = false;
	m_interruptService->interrupt();
}

} // !irccd
