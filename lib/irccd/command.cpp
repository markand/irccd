/*
 * command.cpp -- remote command
 *
 * Copyright (c) 2013-2016 David Demelier <markand@malikania.fr>
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

#include <iomanip>
#include <numeric>
#include <sstream>

#include <format.h>

#include "command.hpp"
#include "logger.hpp"
#include "system.hpp"

using namespace std::string_literals;

using namespace fmt::literals;

using json = nlohmann::json;

namespace irccd {

namespace {

/*
 * typeName
 * ------------------------------------------------------------------
 *
 * Convert a JSON value type to string for convenience.
 */
std::string typeName(nlohmann::json::value_t type) noexcept
{
    switch (type) {
    case nlohmann::json::value_t::array:
        return "array";
    case nlohmann::json::value_t::boolean:
        return "bool";
    case nlohmann::json::value_t::number_float:
        return "float";
    case nlohmann::json::value_t::number_integer:
        return "integer";
    case nlohmann::json::value_t::number_unsigned:
        return "unsigned";
    case nlohmann::json::value_t::null:
        return "null";
    case nlohmann::json::value_t::object:
        return "object";
    case nlohmann::json::value_t::string:
        return "string";
    default:
        return "";
    }
}

/*
 * typeNameList
 * ------------------------------------------------------------------
 *
 * Construct a list of names to send a convenient error message if properties are invalid, example: string, int or bool expected.
 */

std::string typeNameList(const std::vector<json::value_t> &types)
{
    std::ostringstream oss;

    if (types.size() == 1)
        return typeName(types[0]);

    for (std::size_t i = 0; i < types.size(); ++i) {
        oss << typeName(types[i]);

        if (i == types.size() - 2)
            oss << " or ";
        else if (i < types.size() - 1)
            oss << ", ";
    }

    return oss.str();
}

} // !namespace

/*
 * JSON errors
 * ------------------------------------------------------------------
 */

MissingPropertyError::MissingPropertyError(std::string name, std::vector<nlohmann::json::value_t> types)
    : m_name(std::move(name))
    , m_types(std::move(types))
{
    m_message = "missing '" + m_name + "' property (" + typeNameList(m_types) + " expected)";
}

InvalidPropertyError::InvalidPropertyError(std::string name, nlohmann::json::value_t expected, nlohmann::json::value_t result)
    : m_name(std::move(name))
    , m_expected(expected)
    , m_result(result)
{
    m_message += "invalid '" + m_name + "' property ";
    m_message += "(" + typeName(expected) + " expected, ";
    m_message += "got " + typeName(result) + ")";
}

PropertyRangeError::PropertyRangeError(std::string name, std::uint64_t min, std::uint64_t max, std::uint64_t value)
    : m_name(std::move(name))
    , m_min(min)
    , m_max(max)
    , m_value(value)
{
    assert(value < min || value > max);

    m_message += "property '" + m_name + "' is out of range ";
    m_message += std::to_string(min) + ".." + std::to_string(max) + ", got " + std::to_string(value);
}

PropertyError::PropertyError(std::string name, std::string message)
    : m_name(std::move(name))
{
    m_message += "property '" + m_name + "': " + message;
}

/*
 * Command implementation
 * ------------------------------------------------------------------
 */

std::string Command::usage() const
{
    std::ostringstream oss;

    oss << "usage: " << sys::programName() << " " << m_name;

    // Options summary.
    if (options().size() > 0)
        oss << " [options...]";

    // Arguments summary.
    if (args().size() > 0) {
        oss << " ";

        for (const auto &arg : args())
            oss << (arg.required() ? "" : "[") << arg.name() << (arg.required() ? "" : "]") << " ";
    }

    // Description.
    oss << "\n\n" << help() << "\n\n";

    // Options.
    if (options().size() > 0) {
        oss << "Options:\n";

        for (const auto &opt : options()) {
            std::ostringstream optoss;

            // Construct the line for the option in a single string to pad it correctly.
            optoss << "  ";
            optoss << (!opt.simpleKey().empty() ? ("-"s + opt.simpleKey() + " ") : "   ");
            optoss << (!opt.longKey().empty() ? ("--"s + opt.longKey() + " "s) : "");
            optoss << opt.arg();

            // Add it padded with spaces.
            oss << std::left << std::setw(28) << optoss.str();
            oss << opt.description() << "\n";
        }
    }

    return oss.str();
}

unsigned Command::min() const noexcept
{
    auto list = args();

    return std::accumulate(list.begin(), list.end(), 0U, [] (unsigned i, const auto &arg) noexcept -> unsigned {
        return i + (arg.required() ? 1 : 0);
    });
}

unsigned Command::max() const noexcept
{
    return (unsigned)args().size();
}

nlohmann::json Command::request(Irccdctl &, const CommandRequest &) const
{
    return nlohmann::json::object({});
}

nlohmann::json Command::exec(Irccd &, const nlohmann::json &request) const
{
    // Verify that requested properties are present in the request.
    for (const auto &prop : properties()) {
        auto it = request.find(prop.name());

        if (it == request.end())
            throw std::invalid_argument("missing '{}' property"_format(prop.name()));

        if (std::find(prop.types().begin(), prop.types().end(), it->type()) == prop.types().end()) {
            auto expected = typeNameList(prop.types());
            auto got = typeName(it->type());

            throw std::invalid_argument("invalid '{}' property ({} expected, got {})"_format(prop.name(), expected, got));
        }
    }

    return nlohmann::json::object({});
}

void Command::result(Irccdctl &, const nlohmann::json &response) const
{
    auto it = response.find("error");

    if (it != response.end() && it->is_string())
        log::warning() << "irccdctl: " << it->dump() << std::endl;
}

} // !irccd
