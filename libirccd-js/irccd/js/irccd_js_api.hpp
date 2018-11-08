/*
 * irccd_js_api.hpp -- Irccd API
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

#ifndef IRCCD_JS_IRCCD_JS_API_HPP
#define IRCCD_JS_IRCCD_JS_API_HPP

/**
 * \file irccd_js_api.hpp
 * \brief irccd Javascript API.
 */

#include <cerrno>
#include <cstring>
#include <string>
#include <system_error>

#include <boost/system/system_error.hpp>

#include "js_api.hpp"

namespace irccd::js {

/**
 * \brief Irccd Javascript API.
 * \ingroup js_api
 */
class irccd_js_api : public js_api {
public:
	/**
	 * \copydoc js_api::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc js_api::load
	 */
	void load(irccd& irccd, std::shared_ptr<js_plugin> plugin) override;
};

namespace duk {

/**
 * \brief Specialize dukx_type_traits for irccd.
 */
template <>
struct type_traits<irccd> {
	/**
	 * Get irccd instance stored in this context.
	 *
	 * \param ctx the context
	 * \return the irccd reference
	 */
	static auto self(duk_context* ctx) -> irccd&;
};

/**
 * \brief Specialize dukx_type_traits for boost::system::system_error.
 */
template <>
struct type_traits<std::system_error> {
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
struct type_traits<boost::system::system_error> {
	/**
	 * Raise an Irccd.SystemError.
	 *
	 * \param ctx the context
	 * param ex the exception
	 */
	static void raise(duk_context* ctx, const boost::system::system_error& ex);
};

} // !duk

} // !irccd::js

#endif // !IRCCD_JS_IRCCD_JS_API_HPP