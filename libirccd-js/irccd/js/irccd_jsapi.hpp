/*
 * irccd_jsapi.hpp -- Irccd API
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

#ifndef IRCCD_JS_IRCCD_JSAPI_HPP
#define IRCCD_JS_IRCCD_JSAPI_HPP

/**
 * \file irccd_jsapi.hpp
 * \brief irccd Javascript API.
 */

#include <cerrno>
#include <cstring>
#include <string>
#include <system_error>

#include <boost/system/system_error.hpp>

#include "jsapi.hpp"

namespace irccd {

/**
 * \brief Irccd Javascript API.
 * \ingroup jsapi
 */
class irccd_jsapi : public jsapi {
public:
    /**
     * \copydoc jsapi::get_name
     */
    std::string get_name() const override;

    /**
     * \copydoc jsapi::load
     */
    void load(irccd& irccd, std::shared_ptr<js_plugin> plugin) override;
};

/**
 * \brief Specialize dukx_type_traits for irccd.
 */
template <>
class dukx_type_traits<irccd> : public std::true_type {
public:
    /**
     * Get irccd instance stored in this context.
     *
     * \param ctx the context
     * \return the irccd reference
     */
    static irccd& self(duk_context* ctx);
};

/**
 * \brief Specialize dukx_type_traits for std::system_error.
 */
template <>
class dukx_type_traits<std::system_error> : public std::true_type {
public:
    /**
     * Raise an Irccd.SystemError.
     *
     * \param ctx the context
     * param ex the exception
     */
    static void raise(duk_context* ctx, const std::system_error& ex);
};

/**
 * \brief Specialize dukx_type_traits for boost::system::system_error.
 */
template <>
class dukx_type_traits<boost::system::system_error> : public std::true_type {
public:
    /**
     * Raise an Irccd.SystemError.
     *
     * \param ctx the context
     * param ex the exception
     */
    static void raise(duk_context* ctx, const boost::system::system_error& ex);
};

} // !irccd

#endif // !IRCCD_JS_IRCCD_JSAPI_HPP
