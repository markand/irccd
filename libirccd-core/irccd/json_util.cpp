/*
 * json_util.cpp -- utilities for JSON
 *
 * Copyright (c) 2018 David Demelier <markand@malikania.fr>
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

#include <limits>
#include <type_traits>

#include "json_util.hpp"

using nlohmann::json;

namespace irccd::json_util {

namespace {

template <typename Int>
auto clampi(const json& value) noexcept -> std::optional<Int>
{
	static_assert(std::is_signed<Int>::value, "Int must be signed");

	if (!value.is_number_integer())
		return std::nullopt;

	const auto ret = value.get<std::int64_t>();

	if (ret < std::numeric_limits<Int>::min() || ret > std::numeric_limits<Int>::max())
		return std::nullopt;

	return static_cast<Int>(ret);
}

template <typename Int>
auto clampu(const json& value) noexcept -> std::optional<Int>
{
	static_assert(std::is_unsigned<Int>::value, "Int must be unsigned");

	if (!value.is_number_unsigned())
		return std::nullopt;

	const auto ret = value.get<std::uint64_t>();

	if (ret > std::numeric_limits<Int>::max())
		return std::nullopt;

	return static_cast<Int>(ret);
}

} // !namespace

auto type_traits<bool>::get(const json& value) noexcept -> std::optional<bool>
{
	if (!value.is_boolean())
		return std::nullopt;

	return value.get<bool>();
}

auto type_traits<double>::get(const json& value) noexcept -> std::optional<double>
{
	if (!value.is_number_float())
		return std::nullopt;

	return value.get<double>();
}

auto type_traits<std::string>::get(const json& value) -> std::optional<std::string>
{
	if (!value.is_string())
		return std::nullopt;

	return value.get<std::string>();
}

auto type_traits<std::int8_t>::get(const json& value) -> std::optional<std::int8_t>
{
	return clampi<std::int8_t>(value);
}

auto type_traits<std::int16_t>::get(const json& value) -> std::optional<std::int16_t>
{
	return clampi<std::int16_t>(value);
}

auto type_traits<std::int32_t>::get(const json& value) -> std::optional<std::int32_t>
{
	return clampi<std::int32_t>(value);
}

auto type_traits<std::int64_t>::get(const json& value) noexcept -> std::optional<std::int64_t>
{
	if (!value.is_number_integer())
		return std::nullopt;

	return value.get<std::int64_t>();
}

auto type_traits<std::uint8_t>::get(const json& value) -> std::optional<std::uint8_t>
{
	return clampu<std::uint8_t>(value);
}

auto type_traits<std::uint16_t>::get(const json& value) -> std::optional<std::uint16_t>
{
	return clampu<std::uint16_t>(value);
}

auto type_traits<std::uint32_t>::get(const json& value) -> std::optional<std::uint32_t>
{
	return clampu<std::uint32_t>(value);
}

auto type_traits<std::uint64_t>::get(const json& value) noexcept -> std::optional<std::uint64_t>
{
	if (!value.is_number_unsigned())
		return std::nullopt;

	return value.get<std::uint64_t>();
}

auto pretty(const json& value, int indent) -> std::string
{
	switch (value.type()) {
	case json::value_t::null:
		return "null";
	case json::value_t::string:
		return value.get<std::string>();
	case json::value_t::boolean:
		return value.get<bool>() ? "true" : "false";
	case json::value_t::number_integer:
		return std::to_string(value.get<std::int64_t>());
	case json::value_t::number_unsigned:
		return std::to_string(value.get<std::uint64_t>());
	case json::value_t::number_float:
		return std::to_string(value.get<double>());
	default:
		return value.dump(indent);
	}
}

auto contains(const nlohmann::json& array, const nlohmann::json& value) noexcept -> bool
{
	for (const auto& v : array)
		if (v == value)
			return true;

	return false;
}

} // !irccd::json_util
