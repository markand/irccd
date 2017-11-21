/*
 * string_util.hpp -- string utilities
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

#ifndef IRCCD_COMMON_STRING_UTIL_HPP
#define IRCCD_COMMON_STRING_UTIL_HPP

/**
 * \file string_util.hpp
 * \brief String utilities.
 */

#include "sysconfig.hpp"

#include <cassert>
#include <ctime>
#include <initializer_list>
#include <limits>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>

#include <boost/format.hpp>

namespace irccd {

namespace string_util {

/**
 * \brief Pack a message and its type
 *
 * On channels and queries, you may have a special command or a standard message
 * depending on the beginning of the message.
 *
 * Example: `!reminder help' may invoke the command event if a plugin reminder
 * exists.
 */
struct message_pack {
    /**
     * \brief Describe which type of message has been received
     */
    enum class type {
        command,                        //!< special command
        message                         //!< standard message
    } type;

    /**
     * Message content.
     */
    std::string message;
};

/**
 * \brief Disable or enable some features.
 */
enum class subst_flags : std::uint8_t {
    date        = (1 << 0),     //!< date templates
    keywords    = (1 << 1),     //!< keywords
    env         = (1 << 2),     //!< environment variables
    shell       = (1 << 3),     //!< command line command
    irc_attrs   = (1 << 4)      //!< IRC escape codes
};

/**
 * \cond ENUM_HIDDEN_SYMBOLS
 */

inline subst_flags operator^(subst_flags v1, subst_flags v2) noexcept
{
    return static_cast<subst_flags>(static_cast<unsigned>(v1) ^ static_cast<unsigned>(v2));
}

inline subst_flags operator&(subst_flags v1, subst_flags v2) noexcept
{
    return static_cast<subst_flags>(static_cast<unsigned>(v1)&  static_cast<unsigned>(v2));
}

inline subst_flags operator|(subst_flags v1, subst_flags v2) noexcept
{
    return static_cast<subst_flags>(static_cast<unsigned>(v1) | static_cast<unsigned>(v2));
}

inline subst_flags operator~(subst_flags v) noexcept
{
    return static_cast<subst_flags>(~static_cast<unsigned>(v));
}

inline subst_flags& operator|=(subst_flags& v1, subst_flags v2) noexcept
{
    v1 = static_cast<subst_flags>(static_cast<unsigned>(v1) | static_cast<unsigned>(v2));

    return v1;
}

inline subst_flags& operator&=(subst_flags& v1, subst_flags v2) noexcept
{
    v1 = static_cast<subst_flags>(static_cast<unsigned>(v1)&  static_cast<unsigned>(v2));

    return v1;
}

inline subst_flags& operator^=(subst_flags& v1, subst_flags v2) noexcept
{
    v1 = static_cast<subst_flags>(static_cast<unsigned>(v1) ^ static_cast<unsigned>(v2));

    return v1;
}

/**
 * \endcond
 */

/**
 * \brief Used for format() function.
 */
class subst {
public:
    /**
     * Flags for selecting templates.
     */
    subst_flags flags{
        subst_flags::date |
        subst_flags::keywords |
        subst_flags::env |
        subst_flags::irc_attrs
    };

    /**
     * Fill that field if you want a date.
     */
    std::time_t time{std::time(nullptr)};

    /**
     * Fill that map if you want to replace keywords.
     */
    std::unordered_map<std::string, std::string> keywords;
};

/**
 * Format a string and update all templates.
 *
 * ## Syntax
 *
 * The syntax is <strong>?{}</strong> where <strong>?</strong> is replaced by
 * one of the token defined below. Braces are mandatory and cannot be ommited.
 *
 * To write a literal template construct, prepend the token twice.
 *
 * ## Availables templates
 *
 * The following templates are available:
 *
 * - <strong>\#{name}</strong>: name will be substituted from the keywords in
 *   params,
 * - <strong>\${name}</strong>: name will be substituted from the environment
 *   variable,
 * - <strong>\@{attributes}</strong>: the attributes will be substituted to IRC
 *   colors (see below),
 * - <strong>%</strong>, any format accepted by strftime(3).
 *
 * ## Attributes
 *
 * The attribute format is composed of three parts, foreground, background and
 * modifiers, each separated by a comma.
 *
 * **Note:** you cannot omit parameters, to specify the background, you must
 * specify the foreground.
 *
 * ## Examples
 *
 * ### Valid constructs
 *
 *   - <strong>\#{target}, welcome</strong>: if target is set to "irccd",
 *     becomes "irccd, welcome",
 *   - <strong>\@{red}\#{target}</strong>: if target is specified, it is written
 *     in red,
 *
 * ### Invalid or literals constructs
 *
 *   - <strong>\#\#{target}</strong>: will output "\#{target}",
 *   - <strong>\#\#</strong>: will output "\#\#",
 *   - <strong>\#target</strong>: will output "\#target",
 *   - <strong>\#{target</strong>: will throw std::invalid_argument.
 *
 * ### Colors & attributes
 *
 *   - <strong>\@{red,blue}</strong>: will write text red on blue background,
 *   - <strong>\@{default,yellow}</strong>: will write default color text on
 *     yellow background,
 *   - <strong>\@{white,black,bold,underline}</strong>: will write white text on
 *     black in both bold and underline.
 */
IRCCD_EXPORT std::string format(std::string text, const subst& params = {});

/**
 * Remove leading and trailing spaces.
 *
 * \param str the string
 * \return the removed white spaces
 */
IRCCD_EXPORT std::string strip(std::string str);

/**
 * Split a string by delimiters.
 *
 * \param list the string to split
 * \param delimiters a list of delimiters
 * \param max max number of split
 * \return a list of string splitted
 */
IRCCD_EXPORT std::vector<std::string> split(const std::string& list, const std::string& delimiters, int max = -1);

/**
 * Join values by a separator and return a string.
 *
 * \param first the first iterator
 * \param last the last iterator
 * \param delim the optional delimiter
 */
template <typename InputIt, typename DelimType = char>
std::string join(InputIt first, InputIt last, DelimType delim = ':')
{
    std::ostringstream oss;

    if (first != last) {
        oss << *first;

        while (++first != last)
            oss << delim << *first;
    }

    return oss.str();
}

/**
 * Convenient overload.
 *
 * \param list the initializer list
 * \param delim the delimiter
 * \return the string
 */
template <typename T, typename DelimType = char>
inline std::string join(std::initializer_list<T> list, DelimType delim = ':')
{
    return join(list.begin(), list.end(), delim);
}

/**
 * Parse IRC message and determine if it's a command or a simple message.
 *
 * If it's a command, the plugin invocation command is removed from the
 * original message, otherwise it is copied verbatime.
 *
 * \param message the message line
 * \param cchar the command char (e.g '!')
 * \param plugin the plugin name
 * \return the pair
 */
IRCCD_EXPORT message_pack parse_message(std::string message,
                                        const std::string& cchar,
                                        const std::string& plugin);

/**
 * Server and identities must have strict names. This function can
 * be used to ensure that they are valid.
 *
 * \param name the identifier name
 * \return true if is valid
 */
inline bool is_identifier(const std::string& name)
{
    return std::regex_match(name, std::regex("[A-Za-z0-9-_]+"));
}

/**
 * Check if the value is a boolean, 1, yes and true are accepted.
 *
 * \param value the value
 * \return true if is boolean
 * \note this function is case-insensitive
 */
IRCCD_EXPORT bool is_boolean(std::string value) noexcept;

/**
 * Check if the string is an integer.
 *
 * \param value the input
 * \param base the optional base
 * \return true if integer
 */
IRCCD_EXPORT bool is_int(const std::string& value, int base = 10) noexcept;

/**
 * Check if the string is real.
 *
 * \param value the value
 * \return true if real
 */
IRCCD_EXPORT bool is_real(const std::string& value) noexcept;

/**
 * Check if the string is a number.
 *
 * \param value the value
 * \return true if it is a number
 */
inline bool is_number(const std::string& value) noexcept
{
    return is_int(value) || is_real(value);
}

/**
 * \cond HIDDEN_SYMBOLS
 */

namespace detail {

inline void sprintf(boost::format&)
{
}

template <typename Arg, typename... Args>
void sprintf(boost::format& fmter, const Arg& arg, const Args&... args)
{
    fmter % arg;
    sprintf(fmter, args...);
}

} // !detail

/**
 * \endcond
 */

/**
 * Convenient wrapper arount boost::format in sprintf style.
 *
 * This is identical as calling boost::format(format) % arg1 % arg2 % argN.
 *
 * \param format the format string
 * \param args the arguments
 * \return the string
 */
template <typename Format, typename... Args>
std::string sprintf(const Format& format, const Args&... args)
{
    boost::format fmter(format);

    detail::sprintf(fmter, args...);

    return fmter.str();
}

/**
 * \cond HIDDEN_SYMBOLS
 */

namespace detail {

inline std::invalid_argument make_invalid_argument(const std::string& str)
{
    std::ostringstream oss;

    oss << "invalid number '" << str << "'";

    return std::invalid_argument(oss.str());
}

template <typename T>
inline std::out_of_range make_out_of_range(const std::string& str, T min, T max)
{
    std::ostringstream oss;

    oss << "number '" << str << "' is out of range ";
    oss << min << ".." << max;

    return std::out_of_range(oss.str());
}

} // !detail

/**
 * \endcond
 */

/**
 * Convert the given string into a signed integer.
 *
 * \param str the string to convert
 * \param min the minimum value allowed
 * \param max the maximum value allowed
 * \throw std::invalid_argument if the number was not parsed
 * \throw std::out_or_range if the argument is out of the specified range
 */
template <typename T = int>
T to_int(const std::string& str, T min = std::numeric_limits<T>::min(), T max = std::numeric_limits<T>::max())
{
    static_assert(std::is_signed<T>::value, "must be signed");

    char* end;
    auto v = std::strtoll(str.c_str(), &end, 10);

    if (*end != '\0')
        throw detail::make_invalid_argument(str);
    if (v < min || v > max)
        throw detail::make_out_of_range(str, min, max);

    return static_cast<T>(v);
}

/**
 * Convert the given string into an unsigned integer.
 *
 * In contrast to the [std::strtoull][strtoull] function, this functions
 * verifies if the string starts with minus sign and throws an exception if any.
 *
 * Note, for this you need to have a trimmed string which contains no leading
 * whitespaces.
 *
 * \pre string must be trimmed
 * \param str the string to convert
 * \param min the minimum value allowed
 * \param max the maximum value allowed
 * \throw std::invalid_argument if the number was not parsed
 * \throw std::out_or_range if the argument is out of the specified range
 *
 * [strtoull]: http://en.cppreference.com/w/cpp/string/byte/strtoul
 */
template <typename T = unsigned>
T to_uint(const std::string& str, T min = std::numeric_limits<T>::min(), T max = std::numeric_limits<T>::max())
{
    static_assert(std::is_unsigned<T>::value, "must be unsigned");

    assert(str.empty() || !std::isspace(str[0]));

    if (str.size() > 0U && str[0] == '-')
        throw detail::make_out_of_range(str, min, max);

    char* end;
    auto v = std::strtoull(str.c_str(), &end, 10);

    if (*end != '\0')
        throw detail::make_invalid_argument(str);
    if (v < min || v > max)
        throw detail::make_out_of_range(str, min, max);

    return v;
}

} // !string_util

} // !irccd

#endif // !IRCCD_COMMON_STRING_UTIL_HPP