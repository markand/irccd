/*
 * plugin_api.cpp -- Irccd.Plugin API
 *
 * Copyright (c) 2013-2020 David Demelier <markand@malikania.fr>
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

#include <irccd/daemon/bot.hpp>
#include <irccd/daemon/plugin_service.hpp>

#include "irccd_api.hpp"
#include "plugin.hpp"
#include "plugin_api.hpp"

using irccd::daemon::bot;
using irccd::daemon::plugin_error;

namespace irccd::js {

namespace {

const std::string_view signature(DUK_HIDDEN_SYMBOL("Irccd.Plugin"));

template <typename Handler>
auto wrap(duk_context* ctx, Handler handler) -> duk_idx_t
{
	try {
		return handler();
	} catch (const plugin_error& ex) {
		duk::raise(ctx, ex);
	} catch (const std::system_error& ex) {
		duk::raise(ctx, ex);
	} catch (const std::exception& ex) {
		duk::raise(ctx, ex);
	}

	return 0;
}

/*
 * set
 * ------------------------------------------------------------------
 *
 * This setter is used to replace the Irccd.Plugin.(config|templates|paths)
 * property when the plugin assign a new one.
 *
 * Because the plugin configuration always has higher priority, when a new
 * object is assigned to 'config' or to the 'templates' property, the plugin
 * configuration is merged to the assigned one, adding or replacing any values.
 *
 * Example:
 *
 * Plugin 'xyz' does:
 *
 * Irccd.Plugin.config = {
 *	  mode: "simple",
 *	  level: "123"
 * };
 *
 * The user configuration is:
 *
 * [plugin.xyz]
 * mode = "hard"
 * path = "/var"
 *
 * The final user table looks like this:
 *
 * Irccd.Plugin.Config = {
 *	  mode: "hard",
 *	  level: "123",
 *	  path: "/var"
 * };
 */
auto set(duk_context* ctx, std::string_view name) -> duk_ret_t
{
	if (!duk_is_object(ctx, 0))
		duk_error(ctx, DUK_ERR_TYPE_ERROR, "'%s' property must be object", name.data());

	// Merge old table with new one.
	duk_get_global_string(ctx, name.data());
	duk_enum(ctx, -1, 0);

	while (duk_next(ctx, -1, true))
		duk_put_prop(ctx, 0);

	// Pop enum and old table.
	duk_pop_2(ctx);

	// Replace the old table with the new assigned one.
	duk_put_global_string(ctx, name.data());

	return 0;
}

/*
 * get
 * ------------------------------------------------------------------
 *
 * Get the Irccd.Plugin.(config|templates|paths) property.
 */
auto get(duk_context* ctx, std::string_view name) -> duk_ret_t
{
	duk_get_global_string(ctx, name.data());

	return 1;
}

/*
 * set_config
 * ------------------------------------------------------------------
 *
 * Wrap setter for Irccd.Plugin.config property.
 */
auto set_config(duk_context* ctx) -> duk_ret_t
{
	return set(ctx, plugin::config_property);
}

/*
 * get_config
 * ------------------------------------------------------------------
 *
 * Wrap getter for Irccd.Plugin.config property.
 */
auto get_config(duk_context* ctx) -> duk_ret_t
{
	return get(ctx, plugin::config_property);
}

/*
 * set_template
 * ------------------------------------------------------------------
 *
 * Wrap setter for Irccd.Plugin.templates property.
 */
auto set_template(duk_context* ctx) -> duk_ret_t
{
	return set(ctx, plugin::templates_property);
}

/*
 * get_template
 * ------------------------------------------------------------------
 *
 * Wrap getter for Irccd.Plugin.templates property.
 */
auto get_template(duk_context* ctx) -> duk_ret_t
{
	return get(ctx, plugin::templates_property);
}

/*
 * set_paths
 * ------------------------------------------------------------------
 *
 * Wrap setter for Irccd.Plugin.paths property.
 */
auto set_paths(duk_context* ctx) -> duk_ret_t
{
	return set(ctx, plugin::paths_property);
}

/*
 * get_paths
 * ------------------------------------------------------------------
 *
 * Wrap getter for Irccd.Plugin.paths property.
 */
auto get_paths(duk_context* ctx) -> duk_ret_t
{
	return get(ctx, plugin::paths_property);
}

// {{{ Irccd.Plugin.info

/*
 * Function: Irccd.Plugin.info([name])
 * ------------------------------------------------------------------
 *
 * Get information about a plugin.
 *
 * The returned object as the following properties:
 *
 * - name: (string) the plugin identifier,
 * - author: (string) the author,
 * - license: (string) the license,
 * - summary: (string) a short description,
 * - version: (string) the version
 *
 * Arguments:
 *   - name, the plugin identifier, if not specified the current plugin is
 *	 selected.
 * Returns:
 *   The plugin information or undefined if the plugin was not found.
 * Throws:
 *   - Irccd.SystemError on errors.
 */
auto Plugin_info(duk_context* ctx) -> duk_idx_t
{
	return wrap(ctx, [&] {
		daemon::plugin* plg;

		if (duk_get_top(ctx) >= 1)
			plg = duk::type_traits<bot>::self(ctx).get_plugins().get(duk_require_string(ctx, 0)).get();
		else
			plg = std::addressof(duk::type_traits<plugin>::self(ctx));

		if (!plg)
			return 0;

		duk_push_object(ctx);
		duk::push(ctx, plg->get_name());
		duk_put_prop_string(ctx, -2, "name");
		duk::push(ctx, plg->get_author());
		duk_put_prop_string(ctx, -2, "author");
		duk::push(ctx, plg->get_license());
		duk_put_prop_string(ctx, -2, "license");
		duk::push(ctx, plg->get_summary());
		duk_put_prop_string(ctx, -2, "summary");
		duk::push(ctx, plg->get_version());
		duk_put_prop_string(ctx, -2, "version");

		return 1;
	});
}

// }}}

// {{{ Irccd.Plugin.list

/*
 * Function: Irccd.Plugin.list()
 * ------------------------------------------------------------------
 *
 * Get the list of plugins, the array returned contains all plugin names.
 *
 * Returns:
 *   The list of all plugin names.
 */
auto Plugin_list(duk_context* ctx) -> duk_idx_t
{
	int i = 0;

	duk_push_array(ctx);

	for (const auto& plg : duk::type_traits<bot>::self(ctx).get_plugins().list()) {
		duk::push(ctx, plg->get_id());
		duk_put_prop_index(ctx, -2, i++);
	}

	return 1;
}

// }}}

// {{{ Irccd.Plugin.load

/*
 * Function: Irccd.Plugin.load(name)
 * ------------------------------------------------------------------
 *
 * Load a plugin by name. This function will search through the standard
 * directories.
 *
 * Arguments:
 *   - name, the plugin identifier.
 * Throws:
 *   - Irccd.PluginError on plugin related errors,
 *   - Irccd.SystemError on other errors.
 */
auto Plugin_load(duk_context* ctx) -> duk_idx_t
{
	return wrap(ctx, [&] {
		duk::type_traits<bot>::self(ctx).get_plugins().load(
			duk::require<std::string_view>(ctx, 0), "");

		return 0;
	});
}

// }}}

// {{{ Irccd.Plugin.reload

/*
 * Function: Irccd.Plugin.reload(name)
 * ------------------------------------------------------------------
 *
 * Reload a plugin by name.
 *
 * Arguments:
 *   - name, the plugin identifier.
 * Throws:
 *   - Irccd.PluginError on plugin related errors,
 *   - Irccd.SystemError on other errors.
 */
auto Plugin_reload(duk_context* ctx) -> duk_idx_t
{
	return wrap(ctx, [&] {
		duk::type_traits<bot>::self(ctx).get_plugins().reload(duk::require<std::string>(ctx, 0));

		return 0;
	});
}

// }}}

// {{{ Irccd.Plugin.unload

/*
 * Function: Irccd.Plugin.unload(name)
 * ------------------------------------------------------------------
 *
 * Unload a plugin by name.
 *
 * Arguments:
 *   - name, the plugin identifier.
 * Throws:
 *   - Irccd.PluginError on plugin related errors,
 *   - Irccd.SystemError on other errors.
 */
auto Plugin_unload(duk_context* ctx) -> duk_idx_t
{
	return wrap(ctx, [&] {
		duk::type_traits<bot>::self(ctx).get_plugins().unload(duk::require<std::string>(ctx, 0));

		return 0;
	});
}

// }}}

// {{{ Irccd.PluginError [constructor]

/*
 * Function: Irccd.PluginError(code, message)
 * ------------------------------------------------------------------
 *
 * Create an Irccd.PluginError object.
 *
 * Arguments:
 *   - code, the error code,
 *   - message, the error message.
 */
auto PluginError_constructor(duk_context* ctx) -> duk_ret_t
{
	duk_push_this(ctx);
	duk_push_int(ctx, duk_require_int(ctx, 0));
	duk_put_prop_string(ctx, -2, "code");
	duk_push_string(ctx, duk_require_string(ctx, 1));
	duk_put_prop_string(ctx, -2, "message");
	duk_push_string(ctx, "PluginError");
	duk_put_prop_string(ctx, -2, "name");
	duk_pop(ctx);

	return 0;
}

// }}}

const duk_function_list_entry functions[] = {
	{ "info",       Plugin_info,    DUK_VARARGS     },
	{ "list",       Plugin_list,    0               },
	{ "load",       Plugin_load,    1               },
	{ "reload",     Plugin_reload,  1               },
	{ "unload",     Plugin_unload,  1               },
	{ nullptr,      nullptr,        0               }
};

} // !namespace

auto plugin_api::get_name() const noexcept -> std::string_view
{
	return "Irccd.Plugin";
}

void plugin_api::load(bot&, plugin& plugin)
{
	duk::stack_guard sa(plugin.get_context());

	// Store plugin.
	duk_push_pointer(plugin.get_context(), &plugin);
	duk_put_global_string(plugin.get_context(), signature.data());

	duk_get_global_string(plugin.get_context(), "Irccd");
	duk_push_object(plugin.get_context());
	duk_put_function_list(plugin.get_context(), -1, functions);

	// 'config' property.
	duk_push_string(plugin.get_context(), "config");
	duk_push_c_function(plugin.get_context(), get_config, 0);
	duk_push_c_function(plugin.get_context(), set_config, 1);
	duk_def_prop(plugin.get_context(), -4, DUK_DEFPROP_HAVE_GETTER | DUK_DEFPROP_HAVE_SETTER);

	// 'templates' property.
	duk_push_string(plugin.get_context(), "templates");
	duk_push_c_function(plugin.get_context(), get_template, 0);
	duk_push_c_function(plugin.get_context(), set_template, 1);
	duk_def_prop(plugin.get_context(), -4, DUK_DEFPROP_HAVE_GETTER | DUK_DEFPROP_HAVE_SETTER);

	// 'paths' property.
	duk_push_string(plugin.get_context(), "paths");
	duk_push_c_function(plugin.get_context(), get_paths, 0);
	duk_push_c_function(plugin.get_context(), set_paths, 1);
	duk_def_prop(plugin.get_context(), -4, DUK_DEFPROP_HAVE_GETTER | DUK_DEFPROP_HAVE_SETTER);

	// PluginError function.
	duk_push_c_function(plugin.get_context(), PluginError_constructor, 2);
	duk_push_object(plugin.get_context());
	duk_get_global_string(plugin.get_context(), "Error");
	duk_get_prop_string(plugin.get_context(), -1, "prototype");
	duk_remove(plugin.get_context(), -2);
	duk_set_prototype(plugin.get_context(), -2);
	duk_put_prop_string(plugin.get_context(), -2, "prototype");
	duk_put_prop_string(plugin.get_context(), -2, "PluginError");

	duk_put_prop_string(plugin.get_context(), -2, "Plugin");
	duk_pop(plugin.get_context());
}

namespace duk {

auto type_traits<plugin>::self(duk_context* ctx) -> plugin&
{
	duk::stack_guard sa(ctx);

	duk_get_global_string(ctx, signature.data());
	auto plg = static_cast<plugin*>(duk_to_pointer(ctx, -1));
	duk_pop(ctx);

	return *plg;
}

void type_traits<plugin_error>::raise(duk_context* ctx, const plugin_error& ex)
{
	duk::stack_guard sa(ctx, 1);

	duk_get_global_string(ctx, "Irccd");
	duk_get_prop_string(ctx, -1, "PluginError");
	duk_remove(ctx, -2);
	duk::push(ctx, ex.code().value());
	duk::push(ctx, ex.code().message());
	duk_new(ctx, 2);

	(void)duk_throw(ctx);
}

} // !duk

} // !irccd::js
