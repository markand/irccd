/*
 * js_api.cpp -- Javascript API module
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

#include "directory_js_api.hpp"
#include "elapsed_timer_js_api.hpp"
#include "file_js_api.hpp"
#include "irccd_js_api.hpp"
#include "logger_js_api.hpp"
#include "plugin_js_api.hpp"
#include "server_js_api.hpp"
#include "system_js_api.hpp"
#include "timer_js_api.hpp"
#include "unicode_js_api.hpp"
#include "util_js_api.hpp"

namespace irccd::js {

namespace {

template <typename T>
auto bind() noexcept -> js_api::constructor
{
	return [] () noexcept {
		return std::make_unique<T>();
	};
}

} // !namespace

auto js_api::registry() noexcept -> const std::vector<constructor>&
{
	static const std::vector<constructor> list {
		// Irccd API must be loaded first.
		bind<irccd_js_api>(),
		bind<directory_js_api>(),
		bind<elapsed_timer_js_api>(),
		bind<file_js_api>(),
		bind<logger_js_api>(),
		bind<plugin_js_api>(),
		bind<server_js_api>(),
		bind<system_js_api>(),
		bind<timer_js_api>(),
		bind<unicode_js_api>(),
		bind<util_js_api>()
	};

	return list;
}

} // !irccd::js
