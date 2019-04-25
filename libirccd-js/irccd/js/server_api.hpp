/*
 * server_api.hpp -- Irccd.Server API
 *
 * Copyright (c) 2013-2019 David Demelier <markand@malikania.fr>
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

#ifndef IRCCD_JS_SERVER_API_HPP
#define IRCCD_JS_SERVER_API_HPP

/**
 * \file server_api.hpp
 * \brief Irccd.Server Javascript API.
 */

#include <irccd/daemon/server.hpp>

#include "api.hpp"

namespace irccd::js {

/**
 * \ingroup js-api
 * \brief Irccd.Server Javascript API.
 */
class server_api : public api {
public:
	/**
	 * \copydoc api::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc api::load
	 */
	void load(daemon::bot& bot, js::plugin& plugin) override;
};

namespace duk {

/**
 * \brief Specialization for servers as shared_ptr.
 *
 * Supports push, require.
 */
template <>
struct type_traits<std::shared_ptr<daemon::server>> {
	/**
	 * Push a server.
	 *
	 * \pre server != nullptr
	 * \param ctx the context
	 * \param server the server
	 */
	static void push(duk_context* ctx, std::shared_ptr<daemon::server> server);

	/**
	 * Require a server. Raise a Javascript error if not a Server.
	 *
	 * \param ctx the context
	 * \param index the index
	 * \return the server
	 */
	static auto require(duk_context* ctx, duk_idx_t index) -> std::shared_ptr<daemon::server>;
};

/**
 * \brief Specialization for server_error.
 */
template <>
struct type_traits<daemon::server_error> {
	/**
	 * Raise a server_error.
	 *
	 * \param ctx the context
	 * \param error the error
	 */
	static void raise(duk_context* ctx, const daemon::server_error& error);
};

} // !duk

} // !irccd::js

#endif // !IRCCD_JS_SERVER_API_HPP
