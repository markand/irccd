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
#include <regex>
#include <stdexcept>

#include <format.h>

#include "logger.hpp"
#include "service-plugin.hpp"

using namespace fmt::literals;

namespace irccd {

PluginService::PluginService(Irccd &irccd) noexcept
    : m_irccd(irccd)
{
}

PluginService::~PluginService()
{
    for (const auto &plugin : m_plugins)
        plugin->onUnload(m_irccd);
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

    if (it == m_plugins.end())
        return nullptr;

    return *it;
}

std::shared_ptr<Plugin> PluginService::require(const std::string &name) const
{
    auto plugin = get(name);

    if (!plugin)
        throw std::invalid_argument("plugin {} not found"_format(name));

    return plugin;
}

void PluginService::add(std::shared_ptr<Plugin> plugin)
{
    m_plugins.push_back(std::move(plugin));
}

void PluginService::addLoader(std::unique_ptr<PluginLoader> loader)
{
    m_loaders.push_back(std::move(loader));
}

void PluginService::setConfig(const std::string &name, PluginConfig config)
{
    m_config.emplace(name, std::move(config));
}

PluginConfig PluginService::config(const std::string &name) const
{
    auto it = m_config.find(name);

    if (it != m_config.end())
        return it->second;

    return PluginConfig();
}

void PluginService::setFormats(const std::string &name, PluginFormats formats)
{
    m_formats.emplace(name, std::move(formats));
}

PluginFormats PluginService::formats(const std::string &name) const
{
    auto it = m_formats.find(name);

    if (it != m_formats.end())
        return it->second;

    return PluginFormats();
}

std::shared_ptr<Plugin> PluginService::open(const std::string &id,
                                            const std::string &path)
{
    for (const auto &loader : m_loaders) {
        auto plugin = loader->open(id, path);

        if (plugin)
            return plugin;
    }

    return nullptr;
}

std::shared_ptr<Plugin> PluginService::find(const std::string &id)
{
    for (const auto &loader : m_loaders) {
        auto plugin = loader->find(id);

        if (plugin)
            return plugin;
    }

    return nullptr;
}

void PluginService::load(std::string name, std::string path)
{
    if (has(name))
        return;

    try {
        std::shared_ptr<Plugin> plugin;

        if (path.empty())
            plugin = find(name);
        else
            plugin = open(name, std::move(path));

        if (plugin) {
            plugin->setConfig(m_config[name]);
            plugin->setFormats(m_formats[name]);
            plugin->onLoad(m_irccd);

            add(std::move(plugin));
        }
    } catch (const std::exception &ex) {
        log::warning("plugin {}: {}"_format(name, ex.what()));
    }
}

void PluginService::reload(const std::string &name)
{
    auto plugin = get(name);

    if (plugin)
        plugin->onReload(m_irccd);
}

void PluginService::unload(const std::string &name)
{
    auto it = std::find_if(m_plugins.begin(), m_plugins.end(), [&] (const auto &plugin) {
        return plugin->name() == name;
    });

    if (it != m_plugins.end()) {
        (*it)->onUnload(m_irccd);
        m_plugins.erase(it);
    }
}

} // !irccd
