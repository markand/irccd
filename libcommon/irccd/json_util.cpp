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

#include "json_util.hpp"
#include "string_util.hpp"

namespace irccd {

namespace json_util {

boost::optional<nlohmann::json> get(const nlohmann::json& json,
                                    const nlohmann::json::json_pointer& key) noexcept
{
    // Unfortunately, there is no find using pointer yet.
    try {
        return json.at(key);
    } catch (...) {
        return boost::none;
    }
}

boost::optional<bool> get_bool(const nlohmann::json& json,
                               const nlohmann::json::json_pointer& key) noexcept
{
    const auto v = get(json, key);

    if (!v || !v->is_boolean())
        return boost::none;

    return v->get<bool>();
}

boost::optional<std::uint64_t> get_int(const nlohmann::json& json,
                                       const nlohmann::json::json_pointer& key) noexcept
{
    const auto v = get(json, key);

    if (!v || !v->is_number_integer())
        return boost::none;

    return v->get<std::uint64_t>();
}

boost::optional<std::uint64_t> get_uint(const nlohmann::json& json,
                                        const nlohmann::json::json_pointer& key) noexcept
{
    const auto v = get(json, key);

    if (!v || !v->is_number_unsigned())
        return boost::none;

    return v->get<std::uint64_t>();
}

boost::optional<std::string> get_string(const nlohmann::json& json,
                                        const nlohmann::json::json_pointer& key) noexcept
{
    const auto v = get(json, key);

    if (!v || !v->is_string())
        return boost::none;

    return v->get<std::string>();
}

boost::optional<bool> optional_bool(const nlohmann::json& json,
                                    const nlohmann::json::json_pointer& key,
                                    bool def) noexcept
{
    const auto v = get(json, key);

    if (!v)
        return def;
    if (!v->is_boolean())
        return boost::none;

    return v->get<bool>();
}

boost::optional<std::int64_t> optional_int(const nlohmann::json& json,
                                           const nlohmann::json::json_pointer& key,
                                           std::int64_t def) noexcept
{
    const auto v = get(json, key);

    if (!v)
        return def;
    if (!v->is_number_integer())
        return boost::none;

    return v->get<std::int64_t>();
}

boost::optional<std::uint64_t> optional_uint(const nlohmann::json& json,
                                             const nlohmann::json::json_pointer& key,
                                             std::uint64_t def) noexcept
{
    const auto v = get(json, key);

    if (!v)
        return def;
    if (!v->is_number_unsigned())
        return boost::none;

    return v->get<std::uint64_t>();
}

boost::optional<std::string> optional_string(const nlohmann::json& json,
                                             const nlohmann::json::json_pointer& key,
                                             const std::string& def) noexcept
{
    const auto v = get(json, key);

    if (!v)
        return def;
    if (!v->is_string())
        return boost::none;

    return v->get<std::string>();
}

std::string pretty(const nlohmann::json& value)
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

bool contains(const nlohmann::json& array, const nlohmann::json& value) noexcept
{
    for (const auto &v : array)
        if (v == value)
            return true;

    return false;
}

} // !json_util

} // !irccd
