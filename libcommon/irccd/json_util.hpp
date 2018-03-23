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
inline boost::optional<nlohmann::json> get(const nlohmann::json& json,
                                           const nlohmann::json::json_pointer& key) noexcept
{
    // Unfortunately, there is no find using pointer yet.
    try {
        return json.at(key);
    } catch (...) {
        return boost::none;
    }
}

/**
 * Convenient overload with simple key.
 *
 * \param json the JSON object/array
 * \param key the pointer to the object
 * \return the value or boost::none if not found
 */
inline boost::optional<nlohmann::json> get(const nlohmann::json& json,
                                           const std::string& key) noexcept
{
    const auto it = json.find(key);

    if (it == json.end())
        return boost::none;

    return *it;
}

/**
 * Get a bool or null if not found or invalid.
 *
 * \param json the JSON object/array
 * \param key the pointer or property key
 * \return the value or boost::none if not found or invalid
 */
template <typename Key>
inline boost::optional<bool> get_bool(const nlohmann::json& json, const Key& key) noexcept
{
    const auto v = get(json, key);

    if (!v || !v->is_boolean())
        return boost::none;

    return v->template get<bool>();
}

/**
 * Get a 64 bit signed integer or null if not found or invalid.
 *
 * \param json the JSON object/array
 * \param key the pointer or property key
 * \return the value or boost::none if not found or invalid
 */
template <typename Key>
inline boost::optional<std::int64_t> get_int(const nlohmann::json& json, const Key& key) noexcept
{
    const auto v = get(json, key);

    if (!v || !v->is_number_integer())
        return boost::none;

    return v->template get<std::int64_t>();
}

/**
 * Get a 64 bit unsigned integer or null if not found or invalid.
 *
 * \param json the JSON object/array
 * \param key the pointer or property key
 * \return the value or boost::none if not found or invalid
 */
template <typename Key>
inline boost::optional<std::uint64_t> get_uint(const nlohmann::json& json, const Key& key) noexcept
{
    const auto v = get(json, key);

    if (!v || !v->is_number_unsigned())
        return boost::none;

    return v->template get<std::uint64_t>();
}

/**
 * Get a string or null if not found or invalid.
 *
 * \param json the JSON object/array
 * \param key the pointer or property key
 * \return the value or boost::none if not found or invalid
 */
template <typename Key>
inline boost::optional<std::string> get_string(const nlohmann::json& json, const Key& key) noexcept
{
    const auto v = get(json, key);

    if (!v || !v->is_string())
        return boost::none;

    return v->template get<std::string>();
}

/**
 * Get an optional bool.
 *
 * If the property is not found, return default value. If the property is not
 * a bool, return boost::none, otherwise return the value.
 *
 * \param json the JSON object/array
 * \param key the pointer or property key
 * \param def the default value
 * \return the value, boost::none or def
 */
template <typename Key>
inline boost::optional<bool> optional_bool(const nlohmann::json& json, const Key& key, bool def = false) noexcept
{
    const auto v = get(json, key);

    if (!v)
        return def;
    if (!v->is_boolean())
        return boost::none;

    return v->template get<bool>();
}

/**
 * Get an optional integer.
 *
 * If the property is not found, return default value. If the property is not
 * an integer, return boost::none, otherwise return the value.
 *
 * \param json the JSON object/array
 * \param key the pointer or property key
 * \param def the default value
 * \return the value, boost::none or def
 */
template <typename Key>
inline boost::optional<std::int64_t> optional_int(const nlohmann::json& json,
                                                  const Key& key,
                                                  std::int64_t def = 0) noexcept
{
    const auto v = get(json, key);

    if (!v)
        return def;
    if (!v->is_number_integer())
        return boost::none;

    return v->template get<std::int64_t>();
}

/**
 * Get an optional unsigned integer.
 *
 * If the property is not found, return default value. If the property is not
 * an unsigned integer, return boost::none, otherwise return the value.
 *
 * \param json the JSON object/array
 * \param key the pointer or property key
 * \param def the default value
 * \return the value, boost::none or def
 */
template <typename Key>
inline boost::optional<std::uint64_t> optional_uint(const nlohmann::json& json,
                                                    const Key& key,
                                                    std::uint64_t def = 0) noexcept
{
    const auto v = get(json, key);

    if (!v)
        return def;
    if (!v->is_number_unsigned())
        return boost::none;

    return v->template get<std::uint64_t>();
}

/**
 * Get an optional string.
 *
 * If the property is not found, return default value. If the property is not
 * a string, return boost::none, otherwise return the value.
 *
 * \param json the JSON object/array
 * \param key the pointer or property key
 * \param def the default value
 * \return the value, boost::none or def
 */
template <typename Key>
inline boost::optional<std::string> optional_string(const nlohmann::json& json,
                                                    const Key& key,
                                                    const std::string& def = "") noexcept
{
    const auto v = get(json, key);

    if (!v)
        return def;
    if (!v->is_string())
        return boost::none;

    return v->template get<std::string>();
}

/**
 * Print the value as human readable.
 *
 * \note This only works on flat objects.
 * \param value the value
 * \param indent the optional indent for objects/arrays
 * \return the string
 */
inline std::string pretty(const nlohmann::json& value, int indent = 4)
{
    switch (value.type()) {
    case nlohmann::json::value_t::null:
        return "null";
    case nlohmann::json::value_t::string:
        return value.get<std::string>();
    case nlohmann::json::value_t::boolean:
        return value.get<bool>() ? "true" : "false";
    case nlohmann::json::value_t::number_integer:
        return std::to_string(value.get<std::int64_t>());
    case nlohmann::json::value_t::number_unsigned:
        return std::to_string(value.get<std::uint64_t>());
    case nlohmann::json::value_t::number_float:
        return std::to_string(value.get<double>());
    default:
        return value.dump(indent);
    }
}

/**
 * Check if a JSON array contains a specific value in any order.
 *
 * \param array the JSON array
 * \param value the JSON value
 * \return true if value is present
 */
inline bool contains(const nlohmann::json& array, const nlohmann::json& value) noexcept
{
    for (const auto& v : array)
        if (v == value)
            return true;

    return false;
}



} // !json_util

} // !irccd

#endif // !IRCCD_JSON_UTIL_HPP
