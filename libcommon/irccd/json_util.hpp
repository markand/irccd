/*
 * json_util.hpp -- utilities for JSON
 *
 * Copyright (c) 2013-2017 David Demelier <markand@malikania.fr>
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

#include <json.hpp>

/**
 * \file json_util.hpp
 * \brief Utilities for JSON.
 */

namespace irccd {

/**
 * \brief Utilities for JSON.
 */
namespace json_util {

/**
 * Require a property.
 *
 * \param json the json value
 * \param key the property name
 * \param type the requested property type
 * \return the value
 * \throw std::runtime_error if the property is missing
 */
nlohmann::json require(const nlohmann::json& json, const std::string& key, nlohmann::json::value_t type);

/**
 * Convenient access for booleans.
 *
 * \param json the json object
 * \param key the property key
 * \return the boolean
 * \throw std::runtime_error if the property is missing or not a boolean
 */
inline bool require_bool(const nlohmann::json& json, const std::string& key)
{
    return require(json, key, nlohmann::json::value_t::boolean);
}

/**
 * Convenient access for unique identifiers.
 *
 * \param json the json object
 * \param key the property key
 * \return the identifier
 * \throw std::runtime_error if the property is invalid
 */
std::string require_identifier(const nlohmann::json& json, const std::string& key);

/**
 * Convenient access for ints.
 *
 * \param json the json object
 * \param key the property key
 * \return the int
 * \throw std::runtime_error if the property is missing or not ant int
 */
std::int64_t require_int(const nlohmann::json& json, const std::string& key);

/**
 * Convenient access for unsigned ints.
 *
 * \param json the json object
 * \param key the property key
 * \return the unsigned int
 * \throw std::runtime_error if the property is missing or not ant int
 */
std::uint64_t require_uint(const nlohmann::json& json, const std::string& key);

/**
 * Convenient access for strings.
 *
 * \param json the json object
 * \param key the property key
 * \return the string
 * \throw std::runtime_error if the property is missing or not a string
 */
inline std::string require_string(const nlohmann::json& json, const std::string& key)
{
    return require(json, key, nlohmann::json::value_t::string);
}

/**
 * Convert the json value to boolean.
 *
 * \param json the json value
 * \param def the default value if not boolean
 * \return a boolean
 */
inline bool to_bool(const nlohmann::json& json, bool def = false) noexcept
{
    return json.is_boolean() ? json.get<bool>() : def;
}

/**
 * Convert the json value to int.
 *
 * \param json the json value
 * \param def the default value if not an int
 * \return an int
 */
inline std::int64_t to_int(const nlohmann::json& json, std::int64_t def = 0) noexcept
{
    return json.is_number_integer() ? json.get<std::int64_t>() : def;
}

/**
 * Convert the json value to unsigned.
 *
 * \param json the json value
 * \param def the default value if not a unsigned int
 * \return an unsigned int
 */
inline std::uint64_t to_uint(const nlohmann::json& json, std::uint64_t def = 0) noexcept
{
    return json.is_number_unsigned() ? json.get<std::uint64_t>() : def;
}

/**
 * Convert the json value to string.
 *
 * \param json the json value
 * \param def the default value if not a string
 * \return a string
 */
inline std::string to_string(const nlohmann::json& json, std::string def = "") noexcept
{
    return json.is_string() ? json.get<std::string>() : def;
}

/**
 * Get a property or return null one if not found or if json is not an object.
 *
 * \param json the json value
 * \param property the property key
 * \return the value or null one if not found
 */
inline nlohmann::json get(const nlohmann::json& json, const std::string& property) noexcept
{
    auto it = json.find(property);

    if (it == json.end())
        return nlohmann::json();

    return *it;
}

/**
 * Convenient access for boolean with default value.
 *
 * \param json the json value
 * \param key the property key
 * \param def the default value
 * \return the boolean
 */
inline bool get_bool(const nlohmann::json& json,
                     const std::string& key,
                     bool def = false) noexcept
{
    return to_bool(get(json, key), def);
}

/**
 * Convenient access for ints with default value.
 *
 * \param json the json value
 * \param key the property key
 * \param def the default value
 * \return the int
 */
inline std::int64_t get_int(const nlohmann::json& json,
                            const std::string& key,
                            std::int64_t def = 0) noexcept
{
    return to_int(get(json, key), def);
}

/**
 * Convenient access for unsigned ints with default value.
 *
 * \param json the json value
 * \param key the property key
 * \param def the default value
 * \return the unsigned int
 */
inline std::uint64_t get_uint(const nlohmann::json& json,
                              const std::string& key,
                              std::uint64_t def = 0) noexcept
{
    return to_uint(get(json, key), def);
}

/**
 * Convenient access for strings with default value.
 *
 * \param json the json value
 * \param key the property key
 * \param def the default value
 * \return the string
 */
inline std::string get_string(const nlohmann::json& json,
                              const std::string& key,
                              std::string def = "") noexcept
{
    return to_string(get(json, key), def);
}

/**
 * Print the value as human readable.
 *
 * \param value the value
 * \return the string
 */
inline std::string pretty(const nlohmann::json& value)
{
    switch (value.type()) {
    case nlohmann::json::value_t::boolean:
        return value.get<bool>() ? "true" : "false";
    case nlohmann::json::value_t::string:
        return value.get<std::string>();
    default:
        return value.dump();
    }
}

/**
 * Pretty print a json value in the given object.
 *
 * \param object the object
 * \param prop the property
 * \return the pretty value or empty if key does not exist
 */
inline std::string pretty(const nlohmann::json& object, const std::string& prop)
{
    auto it = object.find(prop);

    if (it == object.end())
        return "";

    return pretty(*it);
}

} // !json_util

} // !irccd

#endif // !IRCCD_COMMON_JSON_UTIL_HPP
