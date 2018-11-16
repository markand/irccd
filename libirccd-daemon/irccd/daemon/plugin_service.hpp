/*
 * plugin_service.hpp -- plugin service
 *
 * Copyright (c) 2013-2018 David Demelier <markand@malikania.fr>
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

#ifndef IRCCD_DAEMON_PLUGIN_SERVICE_HPP
#define IRCCD_DAEMON_PLUGIN_SERVICE_HPP

/**
 * \file plugin_service.hpp
 * \brief Plugin service.
 */

#include <cassert>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "plugin.hpp"

namespace irccd {

class config;

namespace daemon {

class bot;

/**
 * \brief Manage plugins.
 * \ingroup plugins
 * \ingroup services
 */
class plugin_service {
public:
	/**
	 * \brief Map of plugins.
	 */
	using plugins = std::vector<std::shared_ptr<plugin>>;

	/**
	 * \brief List of loaders.
	 */
	using plugin_loaders = std::vector<std::unique_ptr<plugin_loader>>;

private:
	bot& bot_;
	plugins plugins_;
	plugin_loaders loaders_;

public:
	/**
	 * Create the plugin service.
	 *
	 * \param bot the irccd instance
	 */
	plugin_service(bot& bot) noexcept;

	/**
	 * Destroy plugins.
	 */
	virtual ~plugin_service();

	/**
	 * Get the list of plugins.
	 *
	 * \return the list of plugins
	 */
	auto list() const noexcept -> plugins;

	/**
	 * Check if a plugin is loaded.
	 *
	 * \param id the plugin id
	 * \return true if has plugin
	 */
	auto has(std::string_view id) const noexcept -> bool;

	/**
	 * Get a loaded plugin or null if not found.
	 *
	 * \param id the plugin id
	 * \return the plugin or empty one if not found
	 */
	auto get(std::string_view id) const noexcept -> std::shared_ptr<plugin>;

	/**
	 * Find a loaded plugin.
	 *
	 * \param id the plugin id
	 * \return the plugin
	 * \throw plugin_error on errors
	 */
	auto require(std::string_view id) const -> std::shared_ptr<plugin>;

	/**
	 * Add the specified plugin to the registry.
	 *
	 * \pre plg != nullptr
	 * \param plg the plugin
	 * \note the plugin is only added to the list, no action is performed on it
	 */
	void add(std::shared_ptr<plugin> plg);

	/**
	 * Add a loader.
	 *
	 * \pre loader != nullptr
	 * \param loader the loader
	 */
	void add_loader(std::unique_ptr<plugin_loader> loader);

	/**
	 * Get the configuration for the specified plugin.
	 *
	 * \param id the plugin id
	 * \return the configuration
	 */
	auto get_options(std::string_view id) -> plugin::map;

	/**
	 * Get the formats for the specified plugin.
	 *
	 * \param id the plugin id
	 * \return the formats
	 */
	auto get_formats(std::string_view id) -> plugin::map;

	/**
	 * Get the paths for the specified plugin.
	 *
	 * If none is defined, return the default ones.
	 *
	 * \param id the plugin id
	 * \return the paths
	 */
	auto get_paths(std::string_view id) -> plugin::map;

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
	auto open(std::string_view id, std::string_view path) -> std::shared_ptr<plugin>;

	/**
	 * Generic function for finding a plugin.
	 *
	 * \param id the plugin id
	 * \return the plugin or nullptr on failures
	 */
	auto find(std::string_view id) -> std::shared_ptr<plugin>;

	/**
	 * Convenient wrapper that loads a plugin, call handle_load and add it
	 * to the registry.
	 *
	 * Any errors are printed using logger.
	 *
	 * \param id the plugin id
	 * \param path the optional path (searched if empty)
	 */
	void load(std::string_view id, std::string_view path = "");

	/**
	 * Unload a plugin and remove it.
	 *
	 * \param id the plugin id
	 */
	void unload(std::string_view id);

	/**
	 * Reload a plugin by calling onReload.
	 *
	 * \param id the plugin id
	 * \throw std::exception on failures
	 */
	void reload(std::string_view id);

	/**
	 * Call a plugin function and throw an exception with the following errors:
	 *
	 *   - plugin_error::not_found if not loaded
	 *   - plugin_error::exec_error if function failed
	 *
	 * \pre plugin != nullptr
	 * \param plugin the plugin
	 * \param fn the plugin member function (pointer to member)
	 * \param args the arguments to pass
	 */
	template <typename Func, typename... Args>
	void exec(std::shared_ptr<plugin> plugin, Func fn, Args&&... args)
	{
		assert(plugin);

		// TODO: replace with C++17 std::invoke.
		try {
			((*plugin).*(fn))(std::forward<Args>(args)...);
		} catch (const std::exception& ex) {
			throw plugin_error(plugin_error::exec_error, plugin->get_name(), ex.what());
		} catch (...) {
			throw plugin_error(plugin_error::exec_error, plugin->get_name());
		}
	}

	/**
	 * Overloaded function.
	 *
	 * \param name the plugin name
	 * \param fn the plugin member function (pointer to member)
	 * \param args the arguments to pass
	 */
	template <typename Func, typename... Args>
	void exec(const std::string& name, Func fn, Args&&... args)
	{
		auto plugin = find(name);

		if (!plugin)
			throw plugin_error(plugin_error::not_found, plugin->get_name());

		exec(plugin, fn, std::forward<Args>(args)...);
	}

	/**
	 * Remove all plugins.
	 */
	void clear() noexcept;

	/**
	 * Load all plugins.
	 *
	 * \param cfg the config
	 */
	void load(const config& cfg) noexcept;
};

namespace logger {

template <typename T>
struct loggable_traits;

/**
 * \brief Implement Loggable traits for plugin.
 * \ingroup logger-traits
 */
template <>
struct loggable_traits<plugin> {
	/**
	 * Return "plugin"
	 *
	 * \param plugin the plugin
	 * \return the category
	 */
	static auto get_category(const plugin& plugin) -> std::string_view;

	/**
	 * Return the plugin id.
	 *
	 * \param plugin the plugin
	 * \return the plugin id
	 */
	static auto get_component(const plugin& plugin) -> std::string_view;
};

} // !logger

} // !daemon

} // !irccd

#endif // !IRCCD_DAEMON_PLUGIN_SERVICE_HPP
