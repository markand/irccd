/*
 * service-plugin.cpp -- manage plugins
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
#include <functional>
#include <stdexcept>

#include <format.h>

#include "irccd.hpp"
#include "logger.hpp"
#include "plugin.hpp"
#include "plugin-js.hpp"
#include "service-plugin.hpp"

using namespace fmt::literals;

namespace irccd {

PluginService::PluginService(Irccd &irccd) noexcept
	: m_irccd(irccd)
{
}

bool PluginService::has(const std::string &name) const noexcept
{
	return std::count_if(m_plugins.cbegin(), m_plugins.cend(), [&] (const auto &plugin) {
		return plugin->name() == name;
	}) > 0;
}

std::shared_ptr<Plugin> PluginService::get(const std::string &name) const noexcept
{
	auto it = std::find_if(m_plugins.begin(), m_plugins.end(), [&] (const auto &plugin) {
		return plugin->name() == name;
	});

	if (it == m_plugins.end()) {
		return nullptr;
	}

	return *it;
}

std::shared_ptr<Plugin> PluginService::require(const std::string &name) const
{
	auto plugin = get(name);

	if (!plugin) {
		throw std::invalid_argument("plugin {} not found"_format(name));
	}

	return plugin;
}

void PluginService::add(std::shared_ptr<Plugin> plugin)
{
	using namespace std::placeholders;

	// TODO: REMOVE WHEN WE GET THE JAVASCRIPT MODULES
	std::shared_ptr<JsPlugin> jsp = std::dynamic_pointer_cast<JsPlugin>(plugin);

	if (jsp) {
		std::weak_ptr<JsPlugin> ptr(jsp);

		jsp->onTimerSignal.connect(std::bind(&PluginService::handleTimerSignal, this, ptr, _1));
		jsp->onTimerEnd.connect(std::bind(&PluginService::handleTimerEnd, this, ptr, _1));

		// Store reference to irccd.
		duk::putGlobal(jsp->context(), "\xff""\xff""irccd", duk::RawPointer<Irccd>{&m_irccd});
	}

	// Initial load now.
	try {
		plugin->onLoad();
		m_plugins.push_back(std::move(plugin));
	} catch (const std::exception &ex) {
		log::warning("plugin {}: {}"_format(plugin->name(), ex.what()));
	}
}

void PluginService::load(std::string name, const std::string &source, bool find)
{
	// TODO: change with Plugin::find
	auto it = std::find_if(m_plugins.begin(), m_plugins.end(), [&] (const auto &plugin) {
		return plugin->name() == name;
	});

	if (it != m_plugins.end()) {
		throw std::invalid_argument("plugin already loaded");
	}

	std::vector<std::string> paths;
	std::shared_ptr<Plugin> plugin;

	if (find) {
		for (const std::string &dir : path::list(path::PathPlugins)) {
			paths.push_back(dir + source + ".js");
		}
	} else {
		paths.push_back(source);
	}

	// Iterate over all paths.
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
		add(std::move(plugin));
	} else {
		throw std::runtime_error("no suitable plugin found");
	}
}

void PluginService::reload(const std::string &name)
{
	auto plugin = get(name);

	if (plugin) {
		plugin->onReload();
	}
}

void PluginService::unload(const std::string &name)
{
	auto it = std::find_if(m_plugins.begin(), m_plugins.end(), [&] (const auto &plugin) {
		return plugin->name() == name;
	});

	if (it != m_plugins.end()) {
		(*it)->onUnload();
		m_plugins.erase(it);
	}
}

void PluginService::handleTimerSignal(std::weak_ptr<JsPlugin> ptr, std::shared_ptr<Timer> timer)
{
	m_irccd.post([this, ptr, timer] (Irccd &) {
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

void PluginService::handleTimerEnd(std::weak_ptr<JsPlugin> ptr, std::shared_ptr<Timer> timer)
{
	m_irccd.post([this, ptr, timer] (Irccd &) {
		auto plugin = ptr.lock();

		if (plugin) {
			log::debug() << "timer: finished, removing from plugin `" << plugin->name() << "'" << std::endl;
			plugin->removeTimer(timer);
		}
	});
}

} // !irccd
