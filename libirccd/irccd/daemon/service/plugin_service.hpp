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
#include <unordered_map>
#include <vector>

#include <irccd/daemon/plugin.hpp>

namespace irccd {

class irccd;
class config;

/**
 * \brief Manage plugins.
 * \ingroup services
 */
class plugin_service {
public:
    /**
     * \brief Map of plugins.
     */
    using plugins = std::unordered_map<std::string, std::shared_ptr<plugin>>;

    /**
     * \brief List of loaders.
     */
    using plugin_loaders = std::vector<std::unique_ptr<plugin_loader>>;

private:
    irccd& irccd_;
    plugins plugins_;
    plugin_loaders loaders_;

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
    virtual ~plugin_service();

    /**
     * Get the list of plugins.
     *
     * \return the list of plugins
     */
    auto all() const noexcept -> plugins;

    /**
     * Check if a plugin is loaded.
     *
     * \param name the plugin id
     * \return true if has plugin
     */
    auto has(const std::string& name) const noexcept -> bool;

    /**
     * Get a loaded plugin or null if not found.
     *
     * \param name the plugin id
     * \return the plugin or empty one if not found
     */
    auto get(const std::string& name) const noexcept -> std::shared_ptr<plugin>;

    /**
     * Find a loaded plugin.
     *
     * \param name the plugin id
     * \return the plugin
     * \throw plugin_error on errors
     */
    auto require(const std::string& name) const -> std::shared_ptr<plugin>;

    /**
     * Add the specified plugin to the registry.
     *
     * \pre id is valid
     * \pre plugin != nullptr
     * \param id the unique plugin identifier
     * \param plugin the plugin
     * \note the plugin is only added to the list, no action is performed on it
     */
    void add(std::string id, std::shared_ptr<plugin> plugin);

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
     * \return the configuration
     */
    auto get_options(const std::string& id) -> plugin::map;

    /**
     * Get the formats for the specified plugin.
     *
     * \return the formats
     */
    auto get_formats(const std::string& id) -> plugin::map;

    /**
     * Get the paths for the specified plugin.
     *
     * If none is defined, return the default ones.
     *
     * \return the paths
     */
    auto get_paths(const std::string& id) -> plugin::map;

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
    auto open(const std::string& id,
              const std::string& path) -> std::shared_ptr<plugin>;

    /**
     * Generic function for finding a plugin.
     *
     * \param id the plugin id
     * \return the plugin or nullptr on failures
     */
    auto find(const std::string& id) -> std::shared_ptr<plugin>;

    /**
     * Convenient wrapper that loads a plugin, call handle_load and add it to
     * the registry.
     *
     * Any errors are printed using logger.
     *
     * \param name the name
     * \param path the optional path (searched if empty)
     */
    void load(const std::string& name, const std::string& path = "");

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
     * Load all plugins.
     *
     * \param cfg the config
     */
    void load(const config& cfg) noexcept;
};

} // !irccd

#endif // !IRCCD_DAEMON_PLUGIN_SERVICE_HPP
