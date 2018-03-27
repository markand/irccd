/*
 * plugin_service.cpp -- plugin service
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

#include <irccd/config.hpp>
#include <irccd/string_util.hpp>
#include <irccd/system.hpp>

#include <irccd/daemon/irccd.hpp>
#include <irccd/daemon/logger.hpp>

#include <irccd/daemon/service/plugin_service.hpp>

namespace irccd {

namespace {

template <typename Map>
Map to_map(const config& conf, const std::string& section)
{
    Map ret;

    for (const auto& opt : conf.get(section))
        ret.emplace(opt.key(), opt.value());

    return ret;
}

} // !namespace

plugin_service::plugin_service(irccd& irccd) noexcept
    : irccd_(irccd)
{
}

plugin_service::~plugin_service()
{
    for (const auto& plugin : plugins_) {
        try {
            plugin->handle_unload(irccd_);
        } catch (const std::exception& ex) {
            irccd_.log().warning() << "plugin: " << plugin->get_name() << ": " << ex.what() << std::endl;
        }
    }
}

bool plugin_service::has(const std::string& name) const noexcept
{
    return std::count_if(plugins_.cbegin(), plugins_.cend(), [&] (const auto& plugin) {
        return plugin->get_name() == name;
    }) > 0;
}

std::shared_ptr<plugin> plugin_service::get(const std::string& name) const noexcept
{
    auto it = std::find_if(plugins_.begin(), plugins_.end(), [&] (const auto& plugin) {
        return plugin->get_name() == name;
    });

    if (it == plugins_.end())
        return nullptr;

    return *it;
}

std::shared_ptr<plugin> plugin_service::require(const std::string& name) const
{
    auto plugin = get(name);

    if (!plugin)
        throw plugin_error(plugin_error::not_found, name);

    return plugin;
}

void plugin_service::add(std::shared_ptr<plugin> plugin)
{
    plugins_.push_back(std::move(plugin));
}

void plugin_service::add_loader(std::unique_ptr<plugin_loader> loader)
{
    loaders_.push_back(std::move(loader));
}

plugin_config plugin_service::config(const std::string& id)
{
    return to_map<plugin_config>(irccd_.config(), string_util::sprintf("plugin.%s", id));
}

plugin_formats plugin_service::formats(const std::string& id)
{
    return to_map<plugin_formats>(irccd_.config(), string_util::sprintf("format.%s", id));
}

plugin_paths plugin_service::paths(const std::string& id)
{
    auto defaults = to_map<plugin_paths>(irccd_.config(), "paths");
    auto paths = to_map<plugin_paths>(irccd_.config(), string_util::sprintf("paths.%s", id));

    // Fill defaults paths.
    if (!defaults.count("cache"))
        defaults.emplace("cache", sys::cachedir() + "/plugin/" + id);
    if (!defaults.count("data"))
        paths.emplace("data", sys::datadir() + "/plugin/" + id);
    if (!defaults.count("config"))
        paths.emplace("config", sys::sysconfigdir() + "/plugin/" + id);

    // Now fill missing fields.
    if (!paths.count("cache"))
        paths.emplace("cache", defaults["cache"]);
    if (!paths.count("data"))
        paths.emplace("data", defaults["data"]);
    if (!paths.count("config"))
        paths.emplace("config", defaults["config"]);

    return paths;
}

std::shared_ptr<plugin> plugin_service::open(const std::string& id,
                                             const std::string& path)
{
    for (const auto& loader : loaders_) {
        auto plugin = loader->open(id, path);

        if (plugin)
            return plugin;
    }

    return nullptr;
}

std::shared_ptr<plugin> plugin_service::find(const std::string& id)
{
    for (const auto& loader : loaders_) {
        auto plugin = loader->find(id);

        if (plugin)
            return plugin;
    }

    return nullptr;
}

void plugin_service::load(std::string name, std::string path)
{
    if (has(name))
        throw plugin_error(plugin_error::already_exists, name);

    std::shared_ptr<plugin> plugin;

    if (path.empty())
        plugin = find(name);
    else
        plugin = open(name, std::move(path));

    if (!plugin)
        throw plugin_error(plugin_error::not_found, name);

    plugin->set_config(config(name));
    plugin->set_formats(formats(name));
    plugin->set_paths(paths(name));

    exec(plugin, &plugin::handle_load, irccd_);
    add(std::move(plugin));
}

void plugin_service::reload(const std::string& name)
{
    auto plugin = get(name);

    if (!plugin)
        throw plugin_error(plugin_error::not_found, name);

    exec(plugin, &plugin::handle_reload, irccd_);
}

void plugin_service::unload(const std::string& name)
{
    const auto it = std::find_if(plugins_.begin(), plugins_.end(), [&] (const auto& plugin) {
        return plugin->get_name() == name;
    });

    if (it == plugins_.end())
        throw plugin_error(plugin_error::not_found, name);

    // Erase first, in case of throwing.
    const auto save = *it;

    plugins_.erase(it);
    exec(save, &plugin::handle_unload, irccd_);
}

void plugin_service::load(const class config& cfg) noexcept
{
    for (const auto& option : cfg.get("plugins")) {
        if (!string_util::is_identifier(option.key()))
            continue;

        auto name = option.key();
        auto p = get(name);

        // Reload the plugin if already loaded.
        if (p) {
            p->set_config(config(name));
            p->set_formats(formats(name));
            p->set_paths(paths(name));
        } else {
            try {
                load(name, option.value());
            } catch (const std::exception& ex) {
                irccd_.log().warning(ex.what());
            }
        }
    }
}

} // !irccd
