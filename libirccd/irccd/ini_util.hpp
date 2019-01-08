/*
 * ini_util.hpp -- ini utilities
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

#ifndef IRCCD_INI_UTIL_HPP
#define IRCCD_INI_UTIL_HPP

/**
 * \file ini_util.hpp
 * \brief Ini utilities.
 */

#include <optional>

#include "ini.hpp"
#include "string_util.hpp"

namespace irccd {

/**
 * \brief Ini utilities.
 */
namespace ini_util {

/**
 * Get an unsigned integer from the configuration section.
 *
 * \param sc the section
 * \param name the option name
 * \return the value or none if not able to convert
 */
template <typename Int>
inline auto get_uint(const ini::section& sc, std::string_view name) noexcept -> std::optional<Int>
{
	return string_util::to_uint<Int>(sc.get(name).get_value());
}

/**
 * Get an optional string or the default value if not given.
 *
 * \param sc the section
 * \param name the option name
 * \param def the default value
 * \return the value or def if not found
 */
inline auto optional_string(const ini::section& sc,
                            std::string_view name,
                            std::string_view def) noexcept -> std::string
{
	const auto it = sc.find(name);

	if (it == sc.end())
		return std::string(def);

	return it->get_value();
}

/**
 * Get an optional unsigned integer from the configuration section.
 *
 * \param sc the section
 * \param name the option name
 * \param def the default value
 * \return the value or none if not able to convert
 */
template <typename Int>
inline auto optional_uint(const ini::section& sc,
                          std::string_view name,
                          Int def) noexcept -> std::optional<Int>
{
	const auto it = sc.find(name);

	if (it == sc.end())
		return def;

	return string_util::to_uint<Int>(it->get_value());
}

} // !ini_util

} // !irccd

#endif // !IRCCD_INI_UTIL_HPP
