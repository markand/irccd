/*
 * config.cpp -- irccd configuration loader
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

#include <boost/filesystem.hpp>

#include "config.hpp"
#include "system.hpp"

namespace irccd {

auto config::search(std::string_view name) -> std::optional<config>
{
	for (const auto& path : sys::config_filenames(name)) {
		boost::system::error_code ec;

		if (boost::filesystem::exists(path, ec) && !ec)
			return config(path);
	}

	return std::nullopt;
}

config::config(std::string path)
	: document(path.empty() ? ini::document() : ini::read_file(path))
	, path_(std::move(path))
{
}

auto config::get_path() const noexcept -> const std::string&
{
	return path_;
}

} // !irccd
