/*
 * server_jsapi.hpp -- Irccd.Server API
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

#ifndef IRCCD_JS_SERVER_JSAPI_HPP
#define IRCCD_JS_SERVER_JSAPI_HPP

/**
 * \file mod-server.hpp
 * \brief irccd.Server Javascript API.
 */

#include "jsapi.hpp"
#include "server.hpp"

namespace irccd {

/**
 * \brief irccd.Server Javascript API.
 * \ingroup jsapi
 */
class server_jsapi : public jsapi {
public:
    /**
     * \copydoc jsapi::name
     */
    std::string name() const override;

    /**
     * \copydoc jsapi::load
     */
    void load(irccd& irccd, std::shared_ptr<js_plugin> plugin) override;
};

/**
 * \brief Specialization for servers as shared_ptr.
 *
 * Supports push, require.
 */
template <>
class dukx_type_traits<std::shared_ptr<server>> : public std::true_type {
public:
    /**
     * Push a server.
     *
     * \pre server != nullptr
     * \param ctx the context
     * \param server the server
     */
    static void push(duk_context* ctx, std::shared_ptr<server> server);

    /**
     * Require a server. Raise a Javascript error if not a Server.
     *
     * \param ctx the context
     * \param index the index
     * \return the server
     */
    static std::shared_ptr<server> require(duk_context* ctx, duk_idx_t index);
};

} // !irccd

#endif // !IRCCD_JS_SERVER_JSAPI_HPP
