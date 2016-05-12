/*
 * service-plugin.hpp -- manage plugins
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

#ifndef IRCCD_SERVICE_PLUGIN_HPP
#define IRCCD_SERVICE_PLUGIN_HPP

/**
 * \file service-plugin.hpp
 * \brief Manage plugins.
 */

#include <memory>
#include <vector>

namespace irccd {

class Irccd;
class Plugin;
class JsPlugin;
class Timer;

/**
 * \brief Manage plugins.
 */
class PluginService {
private:
	Irccd &m_irccd;
	std::vector<std::shared_ptr<Plugin>> m_plugins;

	// TODO: get rid of this with future JavaScript modules.
	void handleTimerSignal(std::weak_ptr<JsPlugin>, std::shared_ptr<Timer>);
	void handleTimerEnd(std::weak_ptr<JsPlugin>, std::shared_ptr<Timer>);

public:
	/**
	 * Create the plugin service.
	 *
	 * \param irccd the irccd instance
	 */
	PluginService(Irccd &irccd) noexcept;

	/**
	 * Get the list of plugins.
	 *
	 * \return the list of plugins
	 */
	inline const std::vector<std::shared_ptr<Plugin>> &plugins() const noexcept
	{
		return m_plugins;
	}

	/**
	 * Check if a plugin is loaded.
	 *
	 * \param name the plugin id
	 * \return true if has plugin
	 */
	bool has(const std::string &name) const noexcept;

	/**
	 * Get a plugin or empty one if not found.
	 *
	 * \param name the plugin id
	 * \return the plugin or empty one if not found
	 */
	std::shared_ptr<Plugin> get(const std::string &name) const noexcept;

	/**
	 * Find a plugin.
	 *
	 * \param name the plugin id
	 * \return the plugin
	 * \throws std::out_of_range if not found
	 */
	std::shared_ptr<Plugin> require(const std::string &name) const;

	/**
	 * Add a loaded plugin.
	 *
	 * Plugins signals will be connected to the irccd main loop. The onLoad function will also be called and the
	 * plugin is not added on errors.
	 *
	 * \pre plugin must not be empty
	 * \param plugin the plugin
	 */
	void add(std::shared_ptr<Plugin> plugin);

	/**
	 * Load a plugin by path or by searching through directories.
	 *
	 * \param source the path or the plugin id to search
	 * \param find set to true for searching by id
	 */
	void load(std::string name, const std::string &source, bool find);

	/**
	 * Unload a plugin and remove it.
	 *
	 * \param name the plugin id
	 */
	void unload(const std::string &name);

	/**
	 * Reload a plugin by calling onReload.
	 *
	 * \param name the plugin name
	 * \throw std::exception on failures
	 */
	void reload(const std::string &name);
};

} // !irccd

#endif // !IRCCD_SERVICE_PLUGIN_HPP
