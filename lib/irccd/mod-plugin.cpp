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
#include "mod-plugin.hpp"

namespace irccd {

namespace {

/*
 * Wrap function for these functions because they all takes the same arguments.
 *
 * - load,
 * - reload,
 * - unload.
 */
template <typename Func>
duk::Ret wrap(duk::ContextPtr ctx, int nret, Func &&func)
{
	std::string name = duk::require<std::string>(ctx, 0);

	try {
		func(*duk::getGlobal<duk::RawPointer<Irccd>>(ctx, "\xff""\xff""irccd"), name);
	} catch (const std::out_of_range &ex) {
		duk::raise(ctx, duk::ReferenceError(ex.what()));
	} catch (const std::exception &ex) {
		duk::raise(ctx, duk::Error(ex.what()));
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
 * Returns:
 *   The plugin information or undefined if the plugin was not found.
 */
duk::Ret info(duk::ContextPtr ctx)
{
	Plugin *plugin = nullptr;

	if (duk::top(ctx) >= 1) {
		plugin = duk::getGlobal<duk::RawPointer<Irccd>>(ctx, "\xff""\xff""irccd")->pluginService().get(duk::require<std::string>(ctx, 0)).get();
	} else {
		plugin = duk::getGlobal<duk::RawPointer<Plugin>>(ctx, "\xff""\xff""plugin");
	}

	if (!plugin) {
		return 0;
	}

	duk::push(ctx, duk::Object{});
	duk::putProperty(ctx, -1, "name", plugin->name());
	duk::putProperty(ctx, -1, "author", plugin->author());
	duk::putProperty(ctx, -1, "license", plugin->license());
	duk::putProperty(ctx, -1, "summary", plugin->summary());
	duk::putProperty(ctx, -1, "version", plugin->version());

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
duk::Ret list(duk::ContextPtr ctx)
{
	duk::push(ctx, duk::Array{});

	int i = 0;
	for (const auto &plugin : duk::getGlobal<duk::RawPointer<Irccd>>(ctx, "\xff""\xff""irccd")->pluginService().plugins()) {
		duk::putProperty(ctx, -1, i++, plugin->name());
	}

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
duk::Ret load(duk::ContextPtr ctx)
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
duk::Ret reload(duk::ContextPtr ctx)
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
duk::Ret unload(duk::ContextPtr ctx)
{
	return wrap(ctx, 0, [&] (Irccd &irccd, const std::string &name) {
		irccd.pluginService().unload(name);
	});
}

const duk::FunctionMap functions{
	{ "info",	{ info,		DUK_VARARGS	} },
	{ "list",	{ list,		0		} },
	{ "load",	{ load,		1		} },
	{ "reload",	{ reload,	1		} },
	{ "unload",	{ unload,	1		} }
};

} // !namespace

PluginModule::PluginModule() noexcept
	: Module("Irccd.Plugin")
{
}

void PluginModule::load(Irccd &, JsPlugin &plugin)
{
	duk::StackAssert sa(plugin.context());

	duk::getGlobal<void>(plugin.context(), "Irccd");
	duk::push(plugin.context(), duk::Object{});
	duk::push(plugin.context(), functions);
	duk::push(plugin.context(), duk::Object{});
	duk::putProperty(plugin.context(), -2, "config");
	duk::putProperty(plugin.context(), -2, "Plugin");
	duk::pop(plugin.context());
}

} // !irccd
