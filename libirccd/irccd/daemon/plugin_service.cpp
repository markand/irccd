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

#include <boost/format.hpp>

#include <irccd/config.hpp>
#include <irccd/string_util.hpp>
#include <irccd/system.hpp>

#include "irccd.hpp"
#include "logger.hpp"
#include "plugin_service.hpp"

using boost::format;
using boost::str;

namespace irccd {

namespace {

auto to_map(const config& conf, const std::string& section) -> plugin::map
{
	plugin::map ret;

	for (const auto& opt : conf.get(section))
		ret.emplace(opt.get_key(), opt.get_value());

	return ret;
}

} // !namespace

plugin_service::plugin_service(irccd& irccd) noexcept
	: irccd_(irccd)
{
}

plugin_service::~plugin_service()
{
	for (const auto& plg : plugins_) {
		try {
			plg->handle_unload(irccd_);
		} catch (const std::exception& ex) {
			irccd_.get_log().warning(*plg) << ex.what() << std::endl;
		}
	}
}

auto plugin_service::list() const noexcept -> plugins
{
	return plugins_;
}

auto plugin_service::has(std::string_view id) const noexcept -> bool
{
	return get(id) != nullptr;
}

auto plugin_service::get(std::string_view id) const noexcept -> std::shared_ptr<plugin>
{
	const auto find = [id] (const auto& plg) {
		return plg->get_id() == id;
	};

	if (const auto it = std::find_if(plugins_.begin(), plugins_.end(), find); it != plugins_.end())
		return *it;

	return nullptr;
}

auto plugin_service::require(std::string_view id) const -> std::shared_ptr<plugin>
{
	auto plugin = get(id);

	if (!plugin)
		throw plugin_error(plugin_error::not_found, id);

	return plugin;
}

void plugin_service::add(std::shared_ptr<plugin> plugin)
{
	assert(plugin);

	plugins_.push_back(std::move(plugin));
}

void plugin_service::add_loader(std::unique_ptr<plugin_loader> loader)
{
	assert(loader);

	loaders_.push_back(std::move(loader));
}

auto plugin_service::get_options(std::string_view id) -> plugin::map
{
	return to_map(irccd_.get_config(), str(format("plugin.%1%") % id));
}

auto plugin_service::get_formats(std::string_view id) -> plugin::map
{
	return to_map(irccd_.get_config(), str(format("format.%1%") % id));
}

auto plugin_service::get_paths(std::string_view id) -> plugin::map
{
	auto defaults = to_map(irccd_.get_config(), "paths");
	auto paths = to_map(irccd_.get_config(), str(format("paths.%1%") % id));

	// Fill defaults paths.
	if (!defaults.count("cache"))
		defaults.emplace("cache", sys::cachedir().string());
	if (!defaults.count("data"))
		defaults.emplace("data", sys::datadir().string());
	if (!defaults.count("config"))
		defaults.emplace("config", sys::sysconfdir().string());

	const auto join = [id] (auto path) {
		return (boost::filesystem::path(path) / "plugin" / std::string(id)).string();
	};

	// Now fill missing fields.
	if (!paths.count("cache"))
		paths.emplace("cache", join(defaults["cache"]));
	if (!paths.count("data"))
		paths.emplace("data", join(defaults["data"]));
	if (!paths.count("config"))
		paths.emplace("config", join(defaults["config"]));

	return paths;
}

auto plugin_service::open(std::string_view id, std::string_view path) -> std::shared_ptr<plugin>
{
	for (const auto& loader : loaders_) {
		auto plugin = loader->open(id, path);

		if (plugin)
			return plugin;
	}

	return nullptr;
}

auto plugin_service::find(std::string_view id) -> std::shared_ptr<plugin>
{
	for (const auto& loader : loaders_) {
		try {
			auto plugin = loader->find(id);

			if (plugin)
				return plugin;
		} catch (const std::exception& ex) {
			irccd_.get_log().warning("plugin", id) << ex.what() << std::endl;
		}
	}

	return nullptr;
}

void plugin_service::load(std::string_view id, std::string_view path)
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
	add(std::move(plugin));
}

void plugin_service::reload(std::string_view id)
{
	auto plugin = get(id);

	if (!plugin)
		throw plugin_error(plugin_error::not_found, id);

	exec(plugin, &plugin::handle_reload, irccd_);
}

void plugin_service::unload(std::string_view id)
{
	const auto find = [id] (const auto& plg) {
		return plg->get_id() == id;
	};

	const auto it = std::find_if(plugins_.begin(), plugins_.end(), find);

	if (it == plugins_.end())
		throw plugin_error(plugin_error::not_found, id);

	// Erase first, in case of throwing.
	const auto save = *it;

	plugins_.erase(it);
	exec(save, &plugin::handle_unload, irccd_);
}

void plugin_service::clear() noexcept
{
	while (plugins_.size() > 0) {
		const auto plugin = plugins_[0];

		try {
			unload(plugin->get_id());
		} catch (const std::exception& ex) {
			irccd_.get_log().warning(*plugin) << ex.what() << std::endl;
		}
	}
}

void plugin_service::load(const config& cfg) noexcept
{
	for (const auto& option : cfg.get("plugins")) {
		if (!string_util::is_identifier(option.get_key()))
			continue;

		auto id = option.get_key();
		auto p = get(id);

		// Reload the plugin if already loaded.
		if (p) {
			p->set_options(get_options(id));
			p->set_formats(get_formats(id));
			p->set_paths(get_paths(id));
		} else {
			try {
				load(id, option.get_value());
			} catch (const std::exception& ex) {
				irccd_.get_log().warning("plugin", id) << ex.what() << std::endl;
			}
		}
	}
}

namespace logger {

auto loggable_traits<plugin>::get_category(const plugin&) -> std::string_view
{
	return "plugin";
}

auto loggable_traits<plugin>::get_component(const plugin& plugin) -> std::string_view
{
	return plugin.get_id();
}

} // !logger

} // !irccd
