/*
 * json_util.hpp -- utilities for JSON
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

#ifndef IRCCD_COMMON_JSON_UTIL_HPP
#define IRCCD_COMMON_JSON_UTIL_HPP

/**
 * \file json_util.hpp
 * \brief Utilities for JSON.
 */

#include <irccd/sysconfig.hpp>

#include <cstdint>
#include <string>

#include <boost/optional.hpp>

#include <json.hpp>

namespace irccd {

/**
 * \brief Utilities for JSON.
 */
namespace json_util {

/**
 * Get a JSON value from the given object or array.
 *
 * \param json the JSON object/array
 * \param key the pointer to the object
 * \return the value or boost::none if not found
 */
IRCCD_EXPORT
boost::optional<nlohmann::json> get(const nlohmann::json& json,
                                    const nlohmann::json::json_pointer& key) noexcept;

/**
 * Get a bool or null if not found or invalid.
 *
 * \param json the JSON object/array
 * \param key the pointer to the object
 * \return the value or boost::none if not found or invalid
 */
IRCCD_EXPORT
boost::optional<bool> get_bool(const nlohmann::json& json,
                               const nlohmann::json::json_pointer& key) noexcept;

/**
 * Get a 64 bit signed integer or null if not found or invalid.
 *
 * \param json the JSON object/array
 * \param key the pointer to the object
 * \return the value or boost::none if not found or invalid
 */
IRCCD_EXPORT
boost::optional<std::uint64_t> get_int(const nlohmann::json& json,
                                       const nlohmann::json::json_pointer& key) noexcept;

/**
 * Get a 64 bit unsigned integer or null if not found or invalid.
 *
 * \param json the JSON object/array
 * \param key the pointer to the object
 * \return the value or boost::none if not found or invalid
 */
IRCCD_EXPORT
boost::optional<std::uint64_t> get_uint(const nlohmann::json& json,
                                        const nlohmann::json::json_pointer& key) noexcept;

/**
 * Get a string or null if not found or invalid.
 *
 * \param json the JSON object/array
 * \param key the pointer to the object
 * \return the value or boost::none if not found or invalid
 */
IRCCD_EXPORT
boost::optional<std::string> get_string(const nlohmann::json& json,
                                        const nlohmann::json::json_pointer& key) noexcept;

/**
 * Get an optional bool.
 *
 * If the property is not found, return default value. If the property is not
 * a bool, return boost::none, otherwise return the value.
 *
 * \param json the JSON object/array
 * \param key the pointer to the object
 * \param def the default value
 * \return the value, boost::none or def
 */
IRCCD_EXPORT
boost::optional<bool> optional_bool(const nlohmann::json& json,
                                    const nlohmann::json::json_pointer& key,
                                    bool def = false) noexcept;

/**
 * Get an optional integer.
 *
 * If the property is not found, return default value. If the property is not
 * an integer, return boost::none, otherwise return the value.
 *
 * \param json the JSON object/array
 * \param key the pointer to the object
 * \param def the default value
 * \return the value, boost::none or def
 */
IRCCD_EXPORT
boost::optional<std::int64_t> optional_int(const nlohmann::json& json,
                                           const nlohmann::json::json_pointer& key,
                                           std::int64_t def = 0) noexcept;

/**
 * Get an optional unsigned integer.
 *
 * If the property is not found, return default value. If the property is not
 * an unsigned integer, return boost::none, otherwise return the value.
 *
 * \param json the JSON object/array
 * \param key the pointer to the object
 * \param def the default value
 * \return the value, boost::none or def
 */
IRCCD_EXPORT
boost::optional<std::uint64_t> optional_uint(const nlohmann::json& json,
                                             const nlohmann::json::json_pointer& key,
                                             std::uint64_t def = 0) noexcept;

/**
 * Get an optional string.
 *
 * If the property is not found, return default value. If the property is not
 * a string, return boost::none, otherwise return the value.
 *
 * \param json the JSON object/array
 * \param key the pointer to the object
 * \param def the default value
 * \return the value, boost::none or def
 */
IRCCD_EXPORT
boost::optional<std::string> optional_string(const nlohmann::json& json,
                                             const nlohmann::json::json_pointer& key,
                                             const std::string& def = "") noexcept;

/**
 * Print the value as human readable.
 *
 * \param value the value
 * \return the string
 */
IRCCD_EXPORT
std::string pretty(const nlohmann::json& value);

/**
 * Check if the array contains the given value.
 *
 * \param array the array
 * \param value the JSON value to check
 * \return true if present
 */
IRCCD_EXPORT
bool contains(const nlohmann::json& array, const nlohmann::json& value) noexcept;

} // !json_util

} // !irccd

#endif // !JSON_UTIL_HPP
