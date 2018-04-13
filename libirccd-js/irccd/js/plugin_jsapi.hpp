/*
 * plugin_jsapi.hpp -- Irccd.Plugin API
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

#ifndef IRCCD_JS_PLUGIN_JSAPI_HPP
#define IRCCD_JS_PLUGIN_JSAPI_HPP

/**
 * \file plugin_jsapi.hpp
 * \brief Irccd.Plugin Javascript API.
 */

#include "jsapi.hpp"
#include "js_plugin.hpp"

namespace irccd {

/**
 * \brief Irccd.Plugin Javascript API.
 * \ingroup jsapi
 */
class plugin_jsapi : public jsapi {
public:
    /**
     * \copydoc jsapi::get_name
     */
    std::string get_name() const override;

    /**
     * \copydoc Module::load
     */
    void load(irccd& irccd, std::shared_ptr<js_plugin> plugin) override;
};

/**
 * \brief Specialize dukx_type_traits for plugin.
 */
template <>
class dukx_type_traits<js_plugin> : public std::true_type {
public:
    /**
     * Access the plugin stored in this context.
     *
     * \param ctx the context
     * \return the plugin
     */
    static std::shared_ptr<js_plugin> self(duk_context* ctx);
};

/**
 * \brief Specialization for plugin_error.
 */
template <>
class dukx_type_traits<plugin_error> : public std::true_type {
public:
    /**
     * Raise a plugin_error.
     *
     * \param ctx the context
     * \param error the error
     */
    static void raise(duk_context* ctx, const plugin_error& error);
};

} // !irccd

#endif // !IRCCD_JS_PLUGIN_JSAPI_HPP
