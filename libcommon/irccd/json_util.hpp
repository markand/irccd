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

#ifndef IRCCD_JSON_UTIL_HPP
#define IRCCD_JSON_UTIL_HPP

/**
 * \file json_util.hpp
 * \brief Utilities for JSON.
 */

#include <cstdint>
#include <limits>
#include <string>
#include <type_traits>

#include <boost/optional.hpp>

#include <json.hpp>

namespace irccd {

/**
 * \brief Utilities for JSON.
 */
namespace json_util {

/**
 * \cond JSON_UTIL_HIDDEN_SYMBOLS
 */

namespace detail {

template <typename Int>
class parser_type_traits_uint : public std::true_type {
public:
    static boost::optional<Int> get(const nlohmann::json& value) noexcept
    {
        if (!value.is_number_unsigned())
            return boost::none;

        const auto ret = value.get<std::uint64_t>();

        if (ret > std::numeric_limits<Int>::max())
            return boost::none;

        return static_cast<Int>(ret);
    }
};

template <typename Int>
class parser_type_traits_int : public std::true_type {
public:
    static boost::optional<Int> get(const nlohmann::json& value) noexcept
    {
        if (!value.is_number_integer())
            return boost::none;

        const auto ret = value.get<std::int64_t>();

        if (ret < std::numeric_limits<Int>::min() || ret > std::numeric_limits<Int>::max())
            return boost::none;

        return static_cast<Int>(ret);
    }
};

} // !detail

/**
 * \endcond
 */

/**
 * \brief Describe how to convert a JSON value.
 *
 * This class must be specialized for every type you want to convert from JSON
 * to its native type.
 *
 * You only need to implement the get function with the following signature:
 *
 * ```cpp
 * static boost::optional<T> get(const nlohmann::json& value);
 * ```
 *
 * The implementation should not throw an exception but return a null optional
 * instead.
 *
 * This class is already specialized for the given types:
 *
 * - bool
 * - double
 * - std::uint(8, 16, 32, 64)
 * - std::string
 */
template <typename T>
class parser_type_traits : public std::false_type {
};

/**
 * \brief Specialization for `bool`.
 */
template <>
class parser_type_traits<bool> : public std::true_type {
public:
    /**
     * Convert the JSON value to bool.
     *
     * \return the bool or none if not a boolean type
     */
    static boost::optional<bool> get(const nlohmann::json& value) noexcept
    {
        if (!value.is_boolean())
            return boost::none;

        return value.get<bool>();
    }
};

/**
 * \brief Specialization for `double`.
 */
template <>
class parser_type_traits<double> : public std::true_type {
public:
    /**
     * Convert the JSON value to bool.
     *
     * \return the double or none if not a double type
     */
    static boost::optional<double> get(const nlohmann::json& value) noexcept
    {
        if (!value.is_number_float())
            return boost::none;

        return value.get<double>();
    }
};

/**
 * \brief Specialization for `std::string`.
 */
template <>
class parser_type_traits<std::string> : public std::true_type {
public:
    /**
     * Convert the JSON value to bool.
     *
     * \return the string or none if not a string type
     */
    static boost::optional<std::string> get(const nlohmann::json& value)
    {
        if (!value.is_string())
            return boost::none;

        return value.get<std::string>();
    }
};

/**
 * \brief Specialization for `std::int8_t`.
 */
template <>
class parser_type_traits<std::int8_t> : public detail::parser_type_traits_int<std::int8_t> {
};

/**
 * \brief Specialization for `std::int16_t`.
 */
template <>
class parser_type_traits<std::int16_t> : public detail::parser_type_traits_int<std::int16_t> {
};

/**
 * \brief Specialization for `std::int32_t`.
 */
template <>
class parser_type_traits<std::int32_t> : public detail::parser_type_traits_int<std::int32_t> {
};

/**
 * \brief Specialization for `std::int64_t`.
 */
template <>
class parser_type_traits<std::int64_t> : public std::true_type {
public:
    /**
     * Convert the JSON value to std::int64_t.
     *
     * \return the int or none if not a int type
     */
    static boost::optional<std::int64_t> get(const nlohmann::json& value) noexcept
    {
        if (!value.is_number_integer())
            return boost::none;

        return value.get<std::int64_t>();
    }
};

/**
 * \brief Specialization for `std::int8_t`.
 */
template <>
class parser_type_traits<std::uint8_t> : public detail::parser_type_traits_uint<std::uint8_t> {
};

/**
 * \brief Specialization for `std::int16_t`.
 */
template <>
class parser_type_traits<std::uint16_t> : public detail::parser_type_traits_uint<std::uint16_t> {
};

/**
 * \brief Specialization for `std::int32_t`.
 */
template <>
class parser_type_traits<std::uint32_t> : public detail::parser_type_traits_uint<std::uint32_t> {
};

/**
 * \brief Specialization for `std::int64_t`.
 */
template <>
class parser_type_traits<std::uint64_t> : public std::true_type {
public:
    /**
     * Convert the JSON value to std::uint64_t.
     *
     * \return the int or none if not a int type
     */
    static boost::optional<std::uint64_t> get(const nlohmann::json& value) noexcept
    {
        if (!value.is_number_unsigned())
            return boost::none;

        return value.get<std::uint64_t>();
    }
};

/**
 * \brief Convenient JSON object parser
 *
 * This class helps destructuring insecure JSON input by returning optional
 * values if they are not present or invalid.
 */
class document : public nlohmann::json {
public:
    /**
     * Constructor.
     *
     * \param object the object
     */
    inline document(nlohmann::json object)
        : nlohmann::json(std::move(object))
    {
    }

    /**
     * Get a value from the document object.
     *
     * \param key the property key
     * \return the value or boost::none if not found or not convertible
     */
    template <typename Type>
    inline boost::optional<Type> get(const std::string& key) const noexcept
    {
        static_assert(parser_type_traits<Type>::value, "type not supported");

        const auto it = find(key);

        if (it == end())
            return boost::none;

        return parser_type_traits<Type>::get(*it);
    }

    /**
     * Get an optional value from the document object.
     *
     * If the value is undefined, the default value is returned. Otherwise, if
     * the value is not in the given type, boost::none is returned.
     *
     * \param key the property key
     * \param def the default value if property is undefined
     * \return the value, boost::none or def
     */
    template <typename Type, typename DefaultValue>
    inline boost::optional<Type> optional(const std::string& key, DefaultValue&& def) const noexcept
    {
        static_assert(parser_type_traits<Type>::value, "type not supported");

        const auto it = find(key);

        if (it == end())
            return boost::optional<Type>(std::forward<DefaultValue>(def));

        return parser_type_traits<Type>::get(*it);
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
