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

#include <unordered_map>
#include <memory>
#include <vector>

#include "plugin-js.hpp"

namespace irccd {

class Irccd;

/**
 * \brief Manage plugins.
 * \ingroup services
 */
class PluginService {
private:
    Irccd &m_irccd;
    std::vector<std::shared_ptr<Plugin>> m_plugins;
    std::vector<std::unique_ptr<PluginLoader>> m_loaders;
    std::unordered_map<std::string, PluginConfig> m_config;
    std::unordered_map<std::string, PluginFormats> m_formats;

public:
    /**
     * Create the plugin service.
     *
     * \param irccd the irccd instance
     */
    IRCCD_EXPORT PluginService(Irccd &irccd) noexcept;

    /**
     * Destroy plugins.
     */
    IRCCD_EXPORT ~PluginService();

    /**
     * Get the list of plugins.
     *
     * \return the list of plugins
     */
    inline const std::vector<std::shared_ptr<Plugin>> &list() const noexcept
    {
        return m_plugins;
    }

    /**
     * Check if a plugin is loaded.
     *
     * \param name the plugin id
     * \return true if has plugin
     */
    IRCCD_EXPORT bool has(const std::string &name) const noexcept;

    /**
     * Get a loaded plugin or null if not found.
     *
     * \param name the plugin id
     * \return the plugin or empty one if not found
     */
    IRCCD_EXPORT std::shared_ptr<Plugin> get(const std::string &name) const noexcept;

    /**
     * Find a loaded plugin.
     *
     * \param name the plugin id
     * \return the plugin
     * \throws std::out_of_range if not found
     */
    IRCCD_EXPORT std::shared_ptr<Plugin> require(const std::string &name) const;

    /**
     * Add the specified plugin to the registry.
     *
     * \pre plugin != nullptr
     * \param plugin the plugin
     * \note the plugin is only added to the list, no action is performed on it
     */
    IRCCD_EXPORT void add(std::shared_ptr<Plugin> plugin);

    /**
     * Configure a plugin.
     *
     * If the plugin is already loaded, its configuration is updated.
     *
     * \param name the plugin name
     * \param config the new configuration
     */
    IRCCD_EXPORT void setConfig(const std::string &name, PluginConfig config);

    /**
     * Get a configuration for a plugin.
     *
     * \param name the plugin name
     * \return the configuration or default one if not found
     */
    IRCCD_EXPORT PluginConfig config(const std::string &name) const;

    /**
     * Add formatting for a plugin.
     *
     * \param name the plugin name
     * \param formats the formats
     */
    IRCCD_EXPORT void setFormats(const std::string &name, PluginFormats formats);

    /**
     * Get formats for a plugin.
     *
     * \param name the plugin name
     * \return the formats
     */
    IRCCD_EXPORT PluginFormats formats(const std::string &name) const;

    /**
     * Generic function for opening the plugin at the given path.
     *
     * This function will search for every PluginLoader and call open() on it,
     * the first one that success will be returned.
     *
     * \param id the plugin id
     * \param path the path to the file
     * \return the plugin or nullptr on failures
     */
    IRCCD_EXPORT std::shared_ptr<Plugin> open(const std::string &id,
                                              const std::string &path);

    /**
     * Generic function for finding a plugin.
     *
     * \param id the plugin id
     * \return the plugin or nullptr on failures
     */
    IRCCD_EXPORT std::shared_ptr<Plugin> find(const std::string &id);

    /**
     * Convenient wrapper that loads a plugin, call onLoad and add it to the
     * registry.
     *
     * Any errors are printed using logger.
     *
     * \param name the name
     * \param path the optional path (searched if empty)
     */
    IRCCD_EXPORT void load(std::string name, std::string path = "");

    /**
     * Unload a plugin and remove it.
     *
     * \param name the plugin id
     */
    IRCCD_EXPORT void unload(const std::string &name);

    /**
     * Reload a plugin by calling onReload.
     *
     * \param name the plugin name
     * \throw std::exception on failures
     */
    IRCCD_EXPORT void reload(const std::string &name);
};

} // !irccd

#endif // !IRCCD_SERVICE_PLUGIN_HPP
