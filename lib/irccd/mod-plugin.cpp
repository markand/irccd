/*
 * js-plugin.cpp -- Irccd.Plugin API
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

#include "irccd.hpp"
#include "plugin-js.hpp"
#include "service-plugin.hpp"
#include "mod-irccd.hpp"
#include "mod-plugin.hpp"

namespace irccd {

namespace {

const char *PluginGlobal("\xff""\xff""irccd-plugin-ptr");

/*
 * Wrap function for these functions because they all takes the same arguments.
 *
 * - load,
 * - reload,
 * - unload.
 */
template <typename Func>
duk_idx_t wrap(duk_context *ctx, int nret, Func &&func)
{
    std::string name = duk_require_string(ctx, 0);

    try {
        func(duk_get_irccd(ctx), name);
    } catch (const std::out_of_range &ex) {
        dukx_throw(ctx, ReferenceError(ex.what()));
    } catch (const std::exception &ex) {
        dukx_throw(ctx, Error(ex.what()));
    }

    return nret;
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
 *   - name, the plugin identifier, if not specified the current plugin is selected.
 * duk_idx_turns:
 *   The plugin information or undefined if the plugin was not found.
 */
duk_idx_t info(duk_context *ctx)
{
    std::shared_ptr<Plugin> plugin;

    if (duk_get_top(ctx) >= 1)
        plugin = duk_get_irccd(ctx).pluginService().get(duk_require_string(ctx, 0));
    else
        plugin = duk_get_plugin(ctx);

    if (!plugin)
        return 0;

    duk_push_object(ctx);
    dukx_push_std_string(ctx, plugin->name());
    duk_put_prop_string(ctx, -2, "name");
    dukx_push_std_string(ctx, plugin->author());
    duk_put_prop_string(ctx, -2, "author");
    dukx_push_std_string(ctx, plugin->license());
    duk_put_prop_string(ctx, -2, "license");
    dukx_push_std_string(ctx, plugin->summary());
    duk_put_prop_string(ctx, -2, "summary");
    dukx_push_std_string(ctx, plugin->version());
    duk_put_prop_string(ctx, -2, "version");

    return 1;
}

/*
 * Function: Irccd.Plugin.list()
 * ------------------------------------------------------------------
 *
 * Get the list of plugins, the array returned contains all plugin names.
 *
 * duk_idx_turns:
 *   The list of all plugin names.
 */
duk_idx_t list(duk_context *ctx)
{
    dukx_push_array(ctx, duk_get_irccd(ctx).pluginService().plugins(), [] (auto ctx, auto plugin) {
        dukx_push_std_string(ctx, plugin->name());
    });

    return 1;
}

/*
 * Function: Irccd.Plugin.load(name)
 * ------------------------------------------------------------------
 *
 * Load a plugin by name. This function will search through the standard directories.
 *
 * Arguments:
 *   - name, the plugin identifier.
 * Throws:
 *   - Error on errors,
 *   - ReferenceError if the plugin was not found.
 */
duk_idx_t load(duk_context *ctx)
{
    return wrap(ctx, 0, [&] (Irccd &irccd, const std::string &name) {
        irccd.pluginService().load(name);
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
duk_idx_t reload(duk_context *ctx)
{
    return wrap(ctx, 0, [&] (Irccd &irccd, const std::string &name) {
        irccd.pluginService().reload(name);
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
duk_idx_t unload(duk_context *ctx)
{
    return wrap(ctx, 0, [&] (Irccd &irccd, const std::string &name) {
        irccd.pluginService().unload(name);
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

PluginModule::PluginModule() noexcept
    : Module("Irccd.Plugin")
{
}

void PluginModule::load(Irccd &, const std::shared_ptr<JsPlugin> &plugin)
{
    StackAssert sa(plugin->context());

    duk_push_pointer(plugin->context(), new std::shared_ptr<JsPlugin>(plugin));
    duk_put_global_string(plugin->context(), PluginGlobal);
    duk_get_global_string(plugin->context(), "Irccd");
    duk_push_object(plugin->context());
    duk_put_function_list(plugin->context(), -1, functions);
    duk_get_global_string(plugin->context(), "\xff""\xff""irccd-plugin-config");
    duk_put_prop_string(plugin->context(), -2, "config");
    duk_get_global_string(plugin->context(), "\xff""\xff""irccd-plugin-format");
    duk_put_prop_string(plugin->context(), -2, "format");
    duk_put_prop_string(plugin->context(), -2, "Plugin");
    duk_pop(plugin->context());
}

void PluginModule::unload(Irccd &, const std::shared_ptr<JsPlugin> &plugin)
{
    StackAssert sa(plugin->context());

    duk_push_global_object(plugin->context());
    duk_get_prop_string(plugin->context(), -1, PluginGlobal);
    delete static_cast<std::shared_ptr<JsPlugin> *>(duk_to_pointer(plugin->context(), -1));
    duk_pop(plugin->context());
    duk_del_prop_string(plugin->context(), -1, PluginGlobal);
    duk_pop(plugin->context());
}

std::shared_ptr<JsPlugin> duk_get_plugin(duk_context *ctx)
{
    StackAssert sa(ctx);

    duk_get_global_string(ctx, PluginGlobal);
    auto plugin = static_cast<std::shared_ptr<JsPlugin> *>(duk_to_pointer(ctx, -1));
    duk_pop(ctx);

    return *plugin;
}

} // !irccd
