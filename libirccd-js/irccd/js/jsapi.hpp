/*
 * jsapi.hpp -- Javascript API module
 *
 * Copyright (c) 2013-2017 David Demelier <markand@malikania.fr>
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

#ifndef IRCCD_JS_JSAPI_HPP
#define IRCCD_JS_JSAPI_HPP

/**
 * \file jsapi.hpp
 * \brief Javascript API module.
 */

/**
 * \defgroup Javascript modules.
 * \brief Modules for the Javascript API.
 */

#include <memory>
#include <string>

#include "duktape.hpp"

namespace irccd {

class irccd;
class js_plugin;

/**
 * \brief Javascript API module.
 */
class jsapi {
public:
    /**
     * Default constructor.
     */
    jsapi() noexcept = default;

    /**
     * Virtual destructor defaulted.
     */
    virtual ~jsapi() noexcept = default;

    /**
     * Get the module name.
     *
     * \return the name
     */
    virtual std::string name() const = 0;

    /**
     * Load the module into the Javascript plugin.
     *
     * \param irccd the irccd instance
     * \param plugin the plugin
     */
    virtual void load(irccd& irccd, std::shared_ptr<js_plugin> plugin) = 0;
};

} // !irccd

#endif // !IRCCD_JS_JSAPI_HPP