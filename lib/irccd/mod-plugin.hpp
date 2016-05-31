/*
 * mod-plugin.hpp -- Irccd.Plugin API
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

#ifndef IRCCD_MOD_PLUGIN_HPP
#define IRCCD_MOD_PLUGIN_HPP

/**
 * \file mod-plugin.hpp
 * \brief Irccd.Plugin JavaScript API.
 */

#include "js.hpp"
#include "module.hpp"

namespace irccd {

namespace duk {

/**
 * \brief Custom specialization for JsPlugin.
 * \note Use the dedicated get function to get the underlying JsPlugin.
 */
template <>
class TypeTraits<std::shared_ptr<JsPlugin>> {
public:
	/**
	 * Get the plugin.
	 *
	 * \param ctx the context
	 * \return the plugin
	 */
	IRCCD_EXPORT static std::shared_ptr<JsPlugin> get(duk::Context *ctx);
};

} // !duk

/**
 * \brief Irccd.Plugin JavaScript API.
 * \ingroup modules
 */
class PluginModule : public Module {
public:
	/**
	 * Irccd.Plugin.
	 */
	IRCCD_EXPORT PluginModule() noexcept;

	/**
	 * \copydoc Module::load
	 */
	IRCCD_EXPORT void load(Irccd &irccd, JsPlugin &plugin) override;
};

} // !irccd

#endif // !IRCCD_MOD_PLUGIN_HPP
