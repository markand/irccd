/*
 * json_util.cpp -- utilities for JSON
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

#include "json_util.hpp"
#include "string_util.hpp"

namespace irccd {

namespace json_util {

nlohmann::json require(const nlohmann::json& json, const std::string& key, nlohmann::json::value_t type)
{
    auto it = json.find(key);
    auto dummy = nlohmann::json(type);

    if (it == json.end())
        throw std::runtime_error(string_util::sprintf("missing '%s' property", key));
    if (it->type() != type)
        throw std::runtime_error(string_util::sprintf("invalid '%s' property (%s expected, got %s)",
            key, it->type_name(), dummy.type_name()));

    return *it;
}

std::string require_identifier(const nlohmann::json& json, const std::string& key)
{
    auto id = require_string(json, key);

    if (!string_util::is_identifier(id))
        throw std::runtime_error(string_util::sprintf("invalid '%s' identifier property", id));

    return id;
}

std::int64_t require_int(const nlohmann::json& json, const std::string& key)
{
    auto it = json.find(key);

    if (it == json.end())
        throw std::runtime_error(string_util::sprintf("missing '%s' property", key));
    if (it->is_number_integer())
        return it->get<int>();
    if (it->is_number_unsigned() && it->get<unsigned>() <= INT_MAX)
        return static_cast<int>(it->get<unsigned>());

    throw std::runtime_error(string_util::sprintf("invalid '%s' property (%s expected, got %s)",
        key, it->type_name(), nlohmann::json(0).type_name()));
}

std::uint64_t require_uint(const nlohmann::json& json, const std::string& key)
{
    auto it = json.find(key);

    if (it == json.end())
        throw std::runtime_error(string_util::sprintf("missing '%s' property", key));
    if (it->is_number_unsigned())
        return it->get<unsigned>();
    if (it->is_number_integer() && it->get<int>() >= 0)
        return static_cast<unsigned>(it->get<int>());

    throw std::runtime_error(string_util::sprintf("invalid '%s' property (%s expected, got %s)",
        key, it->type_name(), nlohmann::json(0U).type_name()));
}

} // !json_util

} // !irccd
