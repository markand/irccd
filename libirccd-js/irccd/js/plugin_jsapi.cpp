/*
 * plugin_jsapi.cpp -- Irccd.Plugin API
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

#include <irccd/daemon/irccd.hpp>
#include <irccd/daemon/plugin_service.hpp>

#include "irccd_jsapi.hpp"
#include "js_plugin.hpp"
#include "plugin_jsapi.hpp"

namespace irccd {

namespace {

const char plugin_ref[] = "\xff""\xff""irccd-plugin-ptr";

/*
 * wrap
 * ------------------------------------------------------------------
 *
 * Wrap function for these functions because they all takes the same arguments.
 *
 * - load,
 * - reload,
 * - unload.
 */
template <typename Func>
duk_idx_t wrap(duk_context* ctx, int nret, Func&& func)
{
    std::string name = duk_require_string(ctx, 0);

    try {
        func(dukx_get_irccd(ctx), name);
    } catch (const std::out_of_range& ex) {
        (void)duk_error(ctx, DUK_ERR_REFERENCE_ERROR, "%s", ex.what());
    } catch (const std::exception &ex) {
        (void)duk_error(ctx, DUK_ERR_ERROR, "%s", ex.what());
    }

    return nret;
}

/*
 * set
 * ------------------------------------------------------------------
 *
 * This setter is used to replace the Irccd.Plugin.(config|format|paths)
 * property when the plugin assign a new one.
 *
 * Because the plugin configuration always has higher priority, when a new
 * object is assigned to 'config' or to the 'format' property, the plugin
 * configuration is merged to the assigned one, adding or replacing any values.
 *
 * Example:
 *
 * Plugin 'xyz' does:
 *
 * Irccd.Plugin.config = {
 *      mode: "simple",
 *      level: "123"
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
 *      mode: "hard",
 *      level: "123",
 *      path: "/var"
 * };
 */
duk_ret_t set(duk_context* ctx, const std::string& name)
{
    if (!duk_is_object(ctx, 0))
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "'%s' property must be object", name.c_str());

    // Merge old table with new one.
    duk_get_global_string(ctx, name.c_str());
    duk_enum(ctx, -1, 0);

    while (duk_next(ctx, -1, true))
        duk_put_prop(ctx, 0);

    // Pop enum and old table.
    duk_pop_2(ctx);

    // Replace the old table with the new assigned one.
    duk_put_global_string(ctx, name.c_str());

    return 0;
}

/*
 * get
 * ------------------------------------------------------------------
 *
 * Get the Irccd.Plugin.(config|format|paths) property.
 */
duk_ret_t get(duk_context* ctx, const std::string& name)
{
    duk_get_global_string(ctx, name.c_str());

    return 1;
}

/*
 * set_config
 * ------------------------------------------------------------------
 *
 * Wrap setter for Irccd.Plugin.config property.
 */
duk_ret_t set_config(duk_context* ctx)
{
    return set(ctx, js_plugin::config_property);
}

/*
 * get_config
 * ------------------------------------------------------------------
 *
 * Wrap getter for Irccd.Plugin.config property.
 */
duk_ret_t get_config(duk_context* ctx)
{
    return get(ctx, js_plugin::config_property);
}

/*
 * set_format
 * ------------------------------------------------------------------
 *
 * Wrap setter for Irccd.Plugin.format property.
 */
duk_ret_t set_format(duk_context* ctx)
{
    return set(ctx, js_plugin::format_property);
}

/*
 * get_format
 * ------------------------------------------------------------------
 *
 * Wrap getter for Irccd.Plugin.format property.
 */
duk_ret_t get_format(duk_context* ctx)
{
    return get(ctx, js_plugin::format_property);
}

/*
 * set_paths
 * ------------------------------------------------------------------
 *
 * Wrap setter for Irccd.Plugin.format property.
 */
duk_ret_t set_paths(duk_context* ctx)
{
    return set(ctx, js_plugin::paths_property);
}

/*
 * get_paths
 * ------------------------------------------------------------------
 *
 * Wrap getter for Irccd.Plugin.format property.
 */
duk_ret_t get_paths(duk_context* ctx)
{
    return get(ctx, js_plugin::paths_property);
}

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
 *     selected.
 * Returns:
 *   The plugin information or undefined if the plugin was not found.
 */
duk_idx_t info(duk_context* ctx)
{
    std::shared_ptr<plugin> plugin;

    if (duk_get_top(ctx) >= 1)
        plugin = dukx_get_irccd(ctx).plugins().get(duk_require_string(ctx, 0));
    else
        plugin = dukx_get_plugin(ctx);

    if (!plugin)
        return 0;

    duk_push_object(ctx);
    dukx_push(ctx, plugin->name());
    duk_put_prop_string(ctx, -2, "name");
    dukx_push(ctx, plugin->author());
    duk_put_prop_string(ctx, -2, "author");
    dukx_push(ctx, plugin->license());
    duk_put_prop_string(ctx, -2, "license");
    dukx_push(ctx, plugin->summary());
    duk_put_prop_string(ctx, -2, "summary");
    dukx_push(ctx, plugin->version());
    duk_put_prop_string(ctx, -2, "version");

    return 1;
}

/*
 * Function: Irccd.Plugin.list()
 * ------------------------------------------------------------------
 *
 * Get the list of plugins, the array returned contains all plugin names.
 *
 * Returns:
 *   The list of all plugin names.
 */
duk_idx_t list(duk_context* ctx)
{
    int i = 0;

    duk_push_array(ctx);

    for (const auto& p : dukx_get_irccd(ctx).plugins().list()) {
        dukx_push(ctx, p->name());
        duk_put_prop_index(ctx, -2, i++);
    }

    return 1;
}

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
 *   - Error on errors,
 *   - ReferenceError if the plugin was not found.
 */
duk_idx_t load(duk_context* ctx)
{
    return wrap(ctx, 0, [&] (irccd& irccd, const std::string& name) {
        irccd.plugins().load(name);
    });
}

/*
 * Function: Irccd.Plugin.reload(name)
 * ------------------------------------------------------------------
 *
 * Reload a plugin by name.
 *
 * Arguments:
 *   - name, the plugin identifier.
 * Throws:
 *   - Error on errors,
 *   - ReferenceError if the plugin was not found.
 */
duk_idx_t reload(duk_context* ctx)
{
    return wrap(ctx, 0, [&] (irccd& irccd, const std::string& name) {
        irccd.plugins().reload(name);
    });
}

/*
 * Function: Irccd.Plugin.unload(name)
 * ------------------------------------------------------------------
 *
 * Unload a plugin by name.
 *
 * Arguments:
 *   - name, the plugin identifier.
 * Throws:
 *   - Error on errors,
 *   - ReferenceError if the plugin was not found.
 */
duk_idx_t unload(duk_context* ctx)
{
    return wrap(ctx, 0, [&] (irccd &irccd, const std::string &name) {
        irccd.plugins().unload(name);
    });
}

const duk_function_list_entry functions[] = {
    { "info",   info,       DUK_VARARGS     },
    { "list",   list,       0               },
    { "load",   load,       1               },
    { "reload", reload,     1               },
    { "unload", unload,     1               },
    { nullptr,  nullptr,    0               }
};

} // !namespace

std::string plugin_jsapi::name() const
{
    return "Irccd.Plugin";
}

void plugin_jsapi::load(irccd&, std::shared_ptr<js_plugin> plugin)
{
    dukx_stack_assert sa(plugin->context());

    duk_push_pointer(plugin->context(), new std::weak_ptr<js_plugin>(plugin));
    duk_push_object(plugin->context());
    duk_push_c_function(plugin->context(), [] (auto ctx) -> duk_ret_t {
        duk_get_global_string(ctx, plugin_ref);
        delete static_cast<std::shared_ptr<js_plugin>*>(duk_to_pointer(ctx, -1));
        duk_pop(ctx);
        duk_push_null(ctx);
        duk_put_global_string(ctx, plugin_ref);
        return 0;
    }, 1);
    duk_set_finalizer(plugin->context(), -2);
    duk_put_global_string(plugin->context(), "\xff""\xff""dummy-shared-ptr");
    duk_put_global_string(plugin->context(), plugin_ref);
    duk_get_global_string(plugin->context(), "Irccd");
    duk_push_object(plugin->context());
    duk_put_function_list(plugin->context(), -1, functions);

    // 'config' property.
    duk_push_string(plugin->context(), "config");
    duk_push_c_function(plugin->context(), get_config, 0);
    duk_push_c_function(plugin->context(), set_config, 1);
    duk_def_prop(plugin->context(), -4, DUK_DEFPROP_HAVE_GETTER | DUK_DEFPROP_HAVE_SETTER);

    // 'format' property.
    duk_push_string(plugin->context(), "format");
    duk_push_c_function(plugin->context(), get_format, 0);
    duk_push_c_function(plugin->context(), set_format, 1);
    duk_def_prop(plugin->context(), -4, DUK_DEFPROP_HAVE_GETTER | DUK_DEFPROP_HAVE_SETTER);

    // 'format' property.
    duk_push_string(plugin->context(), "paths");
    duk_push_c_function(plugin->context(), get_paths, 0);
    duk_push_c_function(plugin->context(), set_paths, 1);
    duk_def_prop(plugin->context(), -4, DUK_DEFPROP_HAVE_GETTER | DUK_DEFPROP_HAVE_SETTER);

    duk_put_prop_string(plugin->context(), -2, "Plugin");
    duk_pop(plugin->context());
}

std::shared_ptr<js_plugin> dukx_get_plugin(duk_context* ctx)
{
    dukx_stack_assert sa(ctx);

    duk_get_global_string(ctx, plugin_ref);
    auto plugin = static_cast<std::weak_ptr<js_plugin>*>(duk_to_pointer(ctx, -1));
    duk_pop(ctx);

    return plugin->lock();
}

} // !irccd
