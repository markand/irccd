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

#include "irccd.h"
#include "js-plugin.h"

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
int wrap(js::Context &ctx, int nret, Func &&func)
{
	std::string name = ctx.require<std::string>(0);

	try {
		func(*ctx.getGlobal<js::RawPointer<Irccd>>("\xff""\xff""irccd"), name);
	} catch (const std::out_of_range &ex) {
		ctx.raise(js::ReferenceError(ex.what()));
	} catch (const std::exception &ex) {
		ctx.raise(js::Error(ex.what()));
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
 *   - The plugin information or undefined if the plugin was not found.
 */
int info(js::Context &ctx)
{
	if (ctx.top() >= 1) {
		try {
			ctx.push(ctx.getGlobal<js::RawPointer<Irccd>>("\xff""\xff""irccd")->requirePlugin(ctx.require<std::string>(0))->info());
		} catch (...) {
			ctx.push(js::Undefined{});
		}
	}

	ctx.push(ctx.getGlobal<js::RawPointer<Plugin>>("\xff""\xff""plugin")->info());

	return 1;
}

/*
 * Function: Irccd.Plugin.list()
 * ------------------------------------------------------------------
 *
 * Get the list of plugins, the array returned contains all plugin names.
 *
 * Returns:
 *   - The list of all plugin names.
 */
int list(js::Context &ctx)
{
	ctx.push(js::Array{});

	int i = 0;
	for (const auto &pair : ctx.getGlobal<js::RawPointer<Irccd>>("\xff""\xff""irccd")->plugins())
		ctx.putProperty(-1, i++, pair.first);

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
int load(js::Context &ctx)
{
	return wrap(ctx, 0, [&] (Irccd &irccd, const std::string &name) {
		irccd.loadPlugin(name, name, true);
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
int reload(js::Context &ctx)
{
	return wrap(ctx, 0, [&] (Irccd &irccd, const std::string &name) {
		irccd.reloadPlugin(name);
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
int unload(js::Context &ctx)
{
	return wrap(ctx, 0, [&] (Irccd &irccd, const std::string &name) {
		irccd.unloadPlugin(name);
	});
}

const js::FunctionMap functions{
	{ "info",	{ info,		DUK_VARARGS	} },
	{ "list",	{ list,		0		} },
	{ "load",	{ load,		1		} },
	{ "reload",	{ reload,	1		} },
	{ "unload",	{ unload,	1		} }
};

} // !namespace

void loadJsPlugin(js::Context &ctx) noexcept
{
	ctx.getGlobal<void>("Irccd");
	ctx.push(js::Object{});
	ctx.push(functions);
	ctx.push(js::Object{});
	ctx.putProperty(-2, "config");
	ctx.putProperty(-2, "Plugin");
	ctx.pop();
}

} // !irccd
