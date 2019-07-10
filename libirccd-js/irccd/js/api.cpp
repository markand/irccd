/*
 * api.cpp -- Javascript API module
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

#include "directory_api.hpp"
#include "chrono_api.hpp"
#include "file_api.hpp"
#include "irccd_api.hpp"
#include "logger_api.hpp"
#include "plugin_api.hpp"
#include "server_api.hpp"
#include "system_api.hpp"
#include "timer_api.hpp"
#include "unicode_api.hpp"
#include "util_api.hpp"

namespace irccd::js {

namespace {

template <typename T>
auto bind() noexcept -> api::constructor
{
	return [] () noexcept {
		return std::make_unique<T>();
	};
}

} // !namespace

auto api::registry() noexcept -> const std::vector<constructor>&
{
	static const std::vector<constructor> list {
		// Irccd API must be loaded first.
		bind<irccd_api>(),
		bind<directory_api>(),
		bind<chrono_api>(),
		bind<file_api>(),
		bind<logger_api>(),
		bind<plugin_api>(),
		bind<server_api>(),
		bind<system_api>(),
		bind<timer_api>(),
		bind<unicode_api>(),
		bind<util_api>()
	};

	return list;
}

} // !irccd::js
