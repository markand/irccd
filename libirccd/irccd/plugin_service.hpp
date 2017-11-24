/*
 * plugin_service.hpp -- plugin service
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

#ifndef IRCCD_PLUGIN_SERVICE_HPP
#define IRCCD_PLUGIN_SERVICE_HPP

/**
 * \file plugin_service.hpp
 * \brief Plugin service.
 */

#include <memory>
#include <string>
#include <vector>

#include "plugin.hpp"

namespace irccd {

class irccd;

/**
 * \brief Manage plugins.
 * \ingroup services
 */
class plugin_service {
private:
    irccd& irccd_;
    std::vector<std::shared_ptr<plugin>> plugins_;
    std::vector<std::unique_ptr<plugin_loader>> loaders_;

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
    ~plugin_service();

    /**
     * Get the list of plugins.
     *
     * \return the list of plugins
     */
    inline const std::vector<std::shared_ptr<plugin>>& list() const noexcept
    {
        return plugins_;
    }

    /**
     * Check if a plugin is loaded.
     *
     * \param name the plugin id
     * \return true if has plugin
     */
    bool has(const std::string& name) const noexcept;

    /**
     * Get a loaded plugin or null if not found.
     *
     * \param name the plugin id
     * \return the plugin or empty one if not found
     */
    std::shared_ptr<plugin> get(const std::string& name) const noexcept;

    /**
     * Find a loaded plugin.
     *
     * \param name the plugin id
     * \return the plugin
     * \throws std::out_of_range if not found
     */
    std::shared_ptr<plugin> require(const std::string& name) const;

    /**
     * Add the specified plugin to the registry.
     *
     * \pre plugin != nullptr
     * \param plugin the plugin
     * \note the plugin is only added to the list, no action is performed on it
     */
    void add(std::shared_ptr<plugin> plugin);

    /**
     * Add a loader.
     *
     * \param loader the loader
     */
    void add_loader(std::unique_ptr<plugin_loader> loader);

    /**
     * Get the configuration for the specified plugin.
     *
     * \return the configuration
     */
    plugin_config config(const std::string& id);

    /**
     * Get the formats for the specified plugin.
     *
     * \return the formats
     */
    plugin_formats formats(const std::string& id);

    /**
     * Get the paths for the specified plugin.
     *
     * If none is defined, return the default ones.
     *
     * \return the paths
     */
    plugin_paths paths(const std::string& id);

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
    std::shared_ptr<plugin> open(const std::string& id,
                                 const std::string& path);

    /**
     * Generic function for finding a plugin.
     *
     * \param id the plugin id
     * \return the plugin or nullptr on failures
     */
    std::shared_ptr<plugin> find(const std::string& id);

    /**
     * Convenient wrapper that loads a plugin, call onLoad and add it to the
     * registry.
     *
     * Any errors are printed using logger.
     *
     * \param name the name
     * \param path the optional path (searched if empty)
     */
    void load(std::string name, std::string path = "");

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
};

} // !irccd

#endif // !IRCCD_PLUGIN_SERVICE_HPP
