/*
 * elapsed_timer_api.hpp -- Irccd.ElapsedTimer API
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

#ifndef IRCCD_JS_ELAPSED_TIMER_API_HPP
#define IRCCD_JS_ELAPSED_TIMER_API_HPP

/**
 * \file elapsed_timer_api.hpp
 * \brief Irccd.ElapsedTimer Javascript API.
 */

#include "api.hpp"

namespace irccd::js {

/**
 * \ingroup js-api
 * \brief Irccd.ElapsedTimer Javascript API.
 */
class elapsed_timer_api : public api {
public:
	/**
	 * \copydoc api::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc api::load
	 */
	void load(daemon::bot& bot, std::shared_ptr<plugin> plugin) override;
};

} // !irccd::js

#endif // !IRCCD_JS_ELAPSED_TIMER_API_HPP
