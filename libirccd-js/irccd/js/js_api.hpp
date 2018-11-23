/*
 * js_api.hpp -- Javascript API module
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

#ifndef IRCCD_JS_JS_API_HPP
#define IRCCD_JS_JS_API_HPP

/**
 * \file js_api.hpp
 * \brief Javascript API module.
 */

#include <functional>
#include <memory>
#include <string_view>
#include <vector>

#include "duk.hpp"

namespace irccd {

namespace daemon {

class bot;

} // !daemon

namespace js {

class js_plugin;

/**
 * \ingroup js-api
 * \brief Javascript API module.
 */
class js_api {
public:
	/**
	 * \brief Command constructor factory.
	 */
	using constructor = std::function<std::unique_ptr<js_api> ()>;

	/**
	 * \brief Registry of all commands.
	 */
	static auto registry() noexcept -> const std::vector<constructor>&;

	/**
	 * Default constructor.
	 */
	js_api() noexcept = default;

	/**
	 * Virtual destructor defaulted.
	 */
	virtual ~js_api() noexcept = default;

	/**
	 * Get the module name.
	 *
	 * \return the name
	 */
	virtual auto get_name() const noexcept -> std::string_view = 0;

	/**
	 * Load the module into the Javascript plugin.
	 *
	 * \param bot the irccd instance
	 * \param plugin the plugin
	 */
	virtual void load(daemon::bot& bot, std::shared_ptr<js_plugin> plugin) = 0;
};

} // !js

} // !irccd

#endif // !IRCCD_JS_JS_API_HPP
