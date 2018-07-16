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

auto to_map(const config& conf, const std::string& section) -> plugin::map
{
    plugin::map ret;

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
    for (const auto& [_, plugin] : plugins_) {
        try {
            plugin->handle_unload(irccd_);
        } catch (const std::exception& ex) {
            irccd_.get_log().warning() << "plugin: " << plugin->get_name() << ": " << ex.what() << std::endl;
        }
    }
}

auto plugin_service::all() const noexcept -> plugins
{
    return plugins_;
}

auto plugin_service::has(const std::string& name) const noexcept -> bool
{
    return plugins_.find(name) != plugins_.end();
}

auto plugin_service::get(const std::string& name) const noexcept -> std::shared_ptr<plugin>
{
    if (auto it = plugins_.find(name); it != plugins_.end())
        return it->second;

    return nullptr;
}

auto plugin_service::require(const std::string& name) const -> std::shared_ptr<plugin>
{
    auto plugin = get(name);

    if (!plugin)
        throw plugin_error(plugin_error::not_found, name);

    return plugin;
}

void plugin_service::add(std::string id, std::shared_ptr<plugin> plugin)
{
    plugins_.emplace(std::move(id), std::move(plugin));
}

void plugin_service::add_loader(std::unique_ptr<plugin_loader> loader)
{
    assert(loader);

    loaders_.push_back(std::move(loader));
}

auto plugin_service::get_options(const std::string& id) -> plugin::map
{
    return to_map(irccd_.get_config(), string_util::sprintf("plugin.%s", id));
}

auto plugin_service::get_formats(const std::string& id) -> plugin::map
{
    return to_map(irccd_.get_config(), string_util::sprintf("format.%s", id));
}

auto plugin_service::get_paths(const std::string& id) -> plugin::map
{
    auto defaults = to_map(irccd_.get_config(), "paths");
    auto paths = to_map(irccd_.get_config(), string_util::sprintf("paths.%s", id));

    // Fill defaults paths.
    if (!defaults.count("cache"))
        defaults.emplace("cache", (sys::cachedir() / "plugin" / std::string(id)).string());
    if (!defaults.count("data"))
        paths.emplace("data", (sys::datadir() / "plugin" / std::string(id)).string());
    if (!defaults.count("config"))
        paths.emplace("config", (sys::sysconfdir() / "plugin" / std::string(id)).string());

    // Now fill missing fields.
    if (!paths.count("cache"))
        paths.emplace("cache", defaults["cache"]);
    if (!paths.count("data"))
        paths.emplace("data", defaults["data"]);
    if (!paths.count("config"))
        paths.emplace("config", defaults["config"]);

    return paths;
}

auto plugin_service::open(const std::string& id,
                          const std::string& path) -> std::shared_ptr<plugin>
{
    for (const auto& loader : loaders_) {
        auto plugin = loader->open(id, path);

        if (plugin)
            return plugin;
    }

    return nullptr;
}

auto plugin_service::find(const std::string& id) -> std::shared_ptr<plugin>
{
    for (const auto& loader : loaders_) {
        try {
            auto plugin = loader->find(id);

            if (plugin)
                return plugin;
        } catch (const std::exception& ex) {
            irccd_.get_log().warning() << "plugin " << id << ": " << ex.what() << std::endl;
        }
    }

    return nullptr;
}

void plugin_service::load(const std::string& id, const std::string& path)
{
    if (has(id))
        throw plugin_error(plugin_error::already_exists, id);

    std::shared_ptr<plugin> plugin;

    if (path.empty())
        plugin = find(id);
    else
        plugin = open(id, std::move(path));

    if (!plugin)
        throw plugin_error(plugin_error::not_found, id);

    plugin->set_options(get_options(id));
    plugin->set_formats(get_formats(id));
    plugin->set_paths(get_paths(id));

    exec(plugin, &plugin::handle_load, irccd_);
    add(std::move(id), std::move(plugin));
}

void plugin_service::reload(const std::string& name)
{
    auto plugin = get(name);

    if (!plugin)
        throw plugin_error(plugin_error::not_found, name);

    exec(plugin, &plugin::handle_reload, irccd_);
}

void plugin_service::unload(const std::string& id)
{
    const auto it = plugins_.find(id);

    if (it == plugins_.end())
        throw plugin_error(plugin_error::not_found, id);

    // Erase first, in case of throwing.
    const auto save = it->second;

    plugins_.erase(it);
    exec(it->second, &plugin::handle_unload, irccd_);
}

void plugin_service::load(const config& cfg) noexcept
{
    for (const auto& option : cfg.get("plugins")) {
        if (!string_util::is_identifier(option.key()))
            continue;

        auto name = option.key();
        auto p = get(name);

        // Reload the plugin if already loaded.
        if (p) {
            p->set_options(get_options(name));
            p->set_formats(get_formats(name));
            p->set_paths(get_paths(name));
        } else {
            try {
                load(name, option.value());
            } catch (const std::exception& ex) {
                irccd_.get_log().warning(ex.what());
            }
        }
    }
}

} // !irccd
