/*
 * plugin_js_api.hpp -- Irccd.Plugin API
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

#ifndef IRCCD_JS_PLUGIN_JS_API_HPP
#define IRCCD_JS_PLUGIN_JS_API_HPP

/**
 * \file plugin_js_api.hpp
 * \brief Irccd.Plugin Javascript API.
 */

#include "js_api.hpp"
#include "js_plugin.hpp"

namespace irccd::js {

/**
 * \brief Irccd.Plugin Javascript API.
 * \ingroup js_api
 */
class plugin_js_api : public js_api {
public:
    /**
     * \copydoc js_api::get_name
     */
    auto get_name() const noexcept -> std::string_view override;

    /**
     * \copydoc Module::load
     */
    void load(irccd& irccd, std::shared_ptr<js_plugin> plugin) override;
};

namespace duk {

/**
 * \brief Specialize dukx_type_traits for plugin.
 */
template <>
struct type_traits<js_plugin> {
    /**
     * Access the plugin stored in this context.
     *
     * \param ctx the context
     * \return the plugin
     */
    static auto self(duk_context* ctx) -> js_plugin&;
};

/**
 * \brief Specialization for plugin_error.
 */
template <>
struct type_traits<plugin_error> {
    /**
     * Raise a plugin_error.
     *
     * \param ctx the context
     * \param error the error
     */
    static void raise(duk_context* ctx, const plugin_error& error);
};

} // !duk

} // !irccd::js

#endif // !IRCCD_JS_PLUGIN_JS_API_HPP
