/*
 * system_api.hpp -- Irccd.System API
 *
 * Copyright (c) 2013-2020 David Demelier <markand@malikania.fr>
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

#ifndef IRCCD_JS_SYSTEM_API_HPP
#define IRCCD_JS_SYSTEM_API_HPP

/**
 * \file system_api.hpp
 * \brief Irccd.System Javascript API.
 */

#include "api.hpp"

namespace irccd::js {

/**
 * \ingroup js-api
 * \brief Irccd.System Javascript API.
 */
class system_api : public api {
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

} // !irccd::js

#endif // !IRCCD_JS_SYSTEM_API_HPP
