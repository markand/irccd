/*
 * json_util.hpp -- utilities for JSON
 *
 * Copyright (c) 2018-2019 David Demelier <markand@malikania.fr>
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

#ifndef IRCCD_JSON_UTIL_HPP
#define IRCCD_JSON_UTIL_HPP

/**
 * \file json_util.hpp
 * \brief Utilities for JSON.
 */

#include <cstdint>
#include <optional>
#include <string>

#include <json.hpp>

/**
 * \brief Utilities for JSON.
 */
namespace irccd::json_util {

/**
 * \brief Describe how to convert a JSON value.
 *
 * This traits must be specialized for every type you want to convert from JSON
 * to its native type.
 *
 * You only need to implement the get function with the following signature:
 *
 * ```cpp
 * static std::optional<T> get(const nlohmann::json& value);
 * ```
 *
 * The implementation should not throw an exception but return a null optional
 * instead.
 *
 * This traits is already specialized for the given types:
 *
 * - bool
 * - double
 * - std::uint(8, 16, 32, 64)_t
 * - std::string
 */
template <typename T>
struct type_traits;

/**
 * \brief Specialization for `bool`.
 */
template <>
struct type_traits<bool> {
	/**
	 * Convert the JSON value to bool.
	 *
	 * \param value the value
	 * \return the bool or empty if not a boolean type
	 */
	static auto get(const nlohmann::json& value) noexcept -> std::optional<bool>;
};

/**
 * \brief Specialization for `double`.
 */
template <>
struct type_traits<double> {
	/**
	 * Convert the JSON value to bool.
	 *
	 * \param value the value
	 * \return the double or empty if not a double type
	 */
	static auto get(const nlohmann::json& value) noexcept -> std::optional<double>;
};

/**
 * \brief Specialization for `std::string`.
 */
template <>
struct type_traits<std::string> {
	/**
	 * Convert the JSON value to std::string.
	 *
	 * \param value the value
	 * \return the string or empty if not a string type
	 */
	static auto get(const nlohmann::json& value) -> std::optional<std::string>;
};

/**
 * \brief Specialization for `std::int8_t`.
 */
template <>
struct type_traits<std::int8_t> {
	/**
	 * Convert the JSON value to std::int8_t.
	 *
	 * \param value the value
	 * \return the value or empty if value does not fit between the range
	 */
	static auto get(const nlohmann::json& value) -> std::optional<std::int8_t>;
};

/**
 * \brief Specialization for `std::int16_t`.
 */
template <>
struct type_traits<std::int16_t> {
	/**
	 * Convert the JSON value to std::int16_t.
	 *
	 * \param value the value
	 * \return the value or empty if value does not fit between the range
	 */
	static auto get(const nlohmann::json& value) -> std::optional<std::int16_t>;
};

/**
 * \brief Specialization for `std::int32_t`.
 */
template <>
struct type_traits<std::int32_t> {
	/**
	 * Convert the JSON value to std::int32_t.
	 *
	 * \param value the value
	 * \return the value or empty if value does not fit between the range
	 */
	static auto get(const nlohmann::json& value) -> std::optional<std::int32_t>;
};

/**
 * \brief Specialization for `std::int64_t`.
 */
template <>
struct type_traits<std::int64_t> {
	/**
	 * Convert the JSON value to std::int64_t.
	 *
	 * \param value the value
	 * \return the int or empty if not a int type
	 */
	static auto get(const nlohmann::json& value) noexcept -> std::optional<std::int64_t>;
};

/**
 * \brief Specialization for `std::uint8_t`.
 */
template <>
struct type_traits<std::uint8_t> {
	/**
	 * Convert the JSON value to std::uint8_t.
	 *
	 * \param value the value
	 * \return the value or empty if value does not fit between the range
	 */
	static auto get(const nlohmann::json& value) -> std::optional<std::uint8_t>;
};

/**
 * \brief Specialization for `std::uint16_t`.
 */
template <>
struct type_traits<std::uint16_t> {
	/**
	 * Convert the JSON value to std::uint16_t.
	 *
	 * \param value the value
	 * \return the value or empty if value does not fit between the range
	 */
	static auto get(const nlohmann::json& value) -> std::optional<std::uint16_t>;
};

/**
 * \brief Specialization for `std::int32_t`.
 */
template <>
struct type_traits<std::uint32_t> {
	/**
	 * Convert the JSON value to std::uint32_t.
	 *
	 * \param value the value
	 * \return the value or empty if value does not fit between the range
	 */
	static auto get(const nlohmann::json& value) -> std::optional<std::uint32_t>;
};

/**
 * \brief Specialization for `std::uint64_t`.
 */
template <>
struct type_traits<std::uint64_t> {
	/**
	 * Convert the JSON value to std::uint64_t.
	 *
	 * \param value the value
	 * \return the int or empty if not a int type
	 */
	static auto get(const nlohmann::json& value) noexcept -> std::optional<std::uint64_t>;
};

/**
 * \brief Convenient JSON object parser
 *
 * This class helps destructuring insecure JSON input by returning optional
 * values if they are not present or invalid.
 */
class deserializer : public nlohmann::json {
public:
	/**
	 * Constructor.
	 *
	 * \param obj the JSON object
	 */
	deserializer(const nlohmann::json& obj);

	/**
	 * Get a value from the document object.
	 *
	 * \param key the property key
	 * \return the value or std::nullopt if not found or not convertible
	 */
	template <typename Type>
	auto get(const std::string& key) const noexcept -> std::optional<Type>
	{
		const auto it = find(key);

		if (it == end())
			return std::nullopt;

		return type_traits<Type>::get(*it);
	}

	/**
	 * Get an optional value from the document object.
	 *
	 * If the value is undefined, the default value is returned. Otherwise, if
	 * the value is not in the given type, std::nullopt is returned.
	 *
	 * \param key the property key
	 * \param def the default value if property is undefined
	 * \return the value, std::nullopt or def
	 */
	template <typename Type, typename DefaultValue>
	auto optional(const std::string& key, DefaultValue&& def) const noexcept -> std::optional<Type>
	{
		const auto it = find(key);

		if (it == end())
			return std::optional<Type>(std::forward<DefaultValue>(def));

		return type_traits<Type>::get(*it);
	}
};

/**
 * Print the value as human readable.
 *
 * \note This only works on flat objects.
 * \param value the value
 * \param indent the optional indent for objects/arrays
 * \return the string
 */
auto pretty(const nlohmann::json& value, int indent = 4) -> std::string;

/**
 * Check if a JSON array contains a specific value in any order.
 *
 * \param array the JSON array
 * \param value the JSON value
 * \return true if value is present
 */
auto contains(const nlohmann::json& array, const nlohmann::json& value) noexcept -> bool;

} // !irccd::json_util

#endif // !IRCCD_JSON_UTIL_HPP
