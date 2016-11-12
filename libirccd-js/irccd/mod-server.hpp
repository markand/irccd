/*
 * mod-server.hpp -- Irccd.Server API
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

#ifndef IRCCD_MOD_SERVER_HPP
#define IRCCD_MOD_SERVER_HPP

/**
 * \file mod-server.hpp
 * \brief Irccd.Server JavaScript API.
 */

#include "duktape.hpp"
#include "module.hpp"
#include "server.hpp"

namespace irccd {

/**
 * \brief Irccd.Server JavaScript API.
 * \ingroup modules
 */
class ServerModule : public Module {
public:
    /**
     * Irccd.Server.
     */
    IRCCD_EXPORT ServerModule() noexcept;

    /**
     * \copydoc Module::load
     */
    IRCCD_EXPORT void load(Irccd &irccd, JsPlugin &plugin) override;
};

/**
 * Push a server.
 *
 * \pre server != nullptr
 * \param ctx the context
 * \param server the server
 */
IRCCD_EXPORT void dukx_push_server(duk_context *ctx, std::shared_ptr<Server> server);

/**
 * Require a server. Raise a JavaScript error if not a Server.
 *
 * \param ctx the context
 * \param index the index
 * \return the server
 */
IRCCD_EXPORT std::shared_ptr<Server> dukx_require_server(duk_context *ctx, duk_idx_t index);

} // !irccd

#endif // !IRCCD_JS_SERVER_HPP
