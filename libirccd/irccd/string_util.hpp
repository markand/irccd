/*
 * string_util.hpp -- string utilities
 *
 * Copyright (c) 2013-2019 David Demelier <markand@malikania.fr>
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

#ifndef IRCCD_STRING_UTIL_HPP
#define IRCCD_STRING_UTIL_HPP

/**
 * \file string_util.hpp
 * \brief String utilities.
 */

#include "sysconfig.hpp"

#include <ctime>
#include <initializer_list>
#include <limits>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <vector>

/**
 * \brief String utilities.
 */
namespace irccd::string_util {

// {{{ subst

/**
 * \brief Disable or enable some features.
 */
enum class subst_flags : unsigned {
	date            = (1 << 0),      //!< date templates
	keywords        = (1 << 1),      //!< keywords
	env             = (1 << 2),      //!< environment variables
	shell           = (1 << 3),      //!< command line command
	irc_attrs       = (1 << 4),      //!< IRC escape codes
	shell_attrs     = (1 << 5)       //!< shell attributes
};

/**
 * \cond IRCCD_ENUM_HIDDEN_SYMBOLS
 */

/**
 * Apply bitwise XOR.
 *
 * \param v1 the first value
 * \param v2 the second value
 * \return the new value
 */
inline auto operator^(subst_flags v1, subst_flags v2) noexcept -> subst_flags
{
	return static_cast<subst_flags>(static_cast<unsigned>(v1) ^ static_cast<unsigned>(v2));
}

/**
 * Apply bitwise AND.
 *
 * \param v1 the first value
 * \param v2 the second value
 * \return the new value
 */
inline auto operator&(subst_flags v1, subst_flags v2) noexcept -> subst_flags
{
	return static_cast<subst_flags>(static_cast<unsigned>(v1) & static_cast<unsigned>(v2));
}

/**
 * Apply bitwise OR.
 *
 * \param v1 the first value
 * \param v2 the second value
 * \return the new value
 */
inline auto operator|(subst_flags v1, subst_flags v2) noexcept -> subst_flags
{
	return static_cast<subst_flags>(static_cast<unsigned>(v1) | static_cast<unsigned>(v2));
}

/**
 * Apply bitwise NOT.
 *
 * \param v the value
 * \return the complement
 */
inline auto operator~(subst_flags v) noexcept -> subst_flags
{
	return static_cast<subst_flags>(~static_cast<unsigned>(v));
}

/**
 * Assign bitwise OR.
 *
 * \param v1 the first value
 * \param v2 the second value
 * \return the new value
 */
inline auto operator|=(subst_flags& v1, subst_flags v2) noexcept -> subst_flags&
{
	return v1 = v1 | v2;
}

/**
 * Assign bitwise AND.
 *
 * \param v1 the first value
 * \param v2 the second value
 * \return the new value
 */
inline auto operator&=(subst_flags& v1, subst_flags v2) noexcept -> subst_flags&
{
	return v1 = v1 & v2;
}

/**
 * Assign bitwise XOR.
 *
 * \param v1 the first value
 * \param v2 the second value
 * \return the new value
 */
inline auto operator^=(subst_flags& v1, subst_flags v2) noexcept -> subst_flags&
{
	return v1 = v1 ^ v2;
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
 *   or shell colors (see below),
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
 * - <strong>\#{target}, welcome</strong>: if target is set to "irccd",
 *   becomes "irccd, welcome",
 * - <strong>\@{red}\#{target}</strong>: if target is specified, it is written
 *   in red,
 *
 * ### Invalid or literals constructs
 *
 * - <strong>\#\#{target}</strong>: will output "\#{target}",
 * - <strong>\#\#</strong>: will output "\#\#",
 * - <strong>\#target</strong>: will output "\#target",
 * - <strong>\#{target</strong>: will throw std::invalid_argument.
 *
 * ### Colors & attributes
 *
 * - <strong>\@{red,blue}</strong>: will write text red on blue background,
 * - <strong>\@{default,yellow}</strong>: will write default color text on
 *   yellow background,
 * - <strong>\@{white,black,bold,underline}</strong>: will write white text on
 *   black in both bold and underline.
 *
 * \param text the text to format
 * \param params the additional options
 * \return the modified text
 */
auto format(std::string text, const subst& params = {}) -> std::string;

// }}}

// {{{ strip

/**
 * Remove leading and trailing spaces.
 *
 * \param str the string
 * \return the removed white spaces
 */
auto strip(std::string str) noexcept -> std::string;

// }}}

// {{{ split

/**
 * Split a string by delimiters.
 *
 * \param list the string to split
 * \param delimiters a list of delimiters
 * \param max max number of split
 * \return a list of string splitted
 */
auto split(std::string_view list, const std::string& delimiters, int max = -1) -> std::vector<std::string>;

// }}}

// {{{ join

/**
 * Join values by a separator and return a string.
 *
 * \param first the first iterator
 * \param last the last iterator
 * \param delim the optional delimiter
 * \return the string
 */
template <typename InputIt, typename DelimType = char>
auto join(InputIt first, InputIt last, DelimType delim = ':') -> std::string
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
 * Overloaded function that takes a container.
 *
 * \param c the container
 * \param delim the optional delimiter
 * \return the string
 */
template <typename Container, typename DelimType = char>
auto join(const Container& c, DelimType delim = ':') -> std::string
{
	return join(c.begin(), c.end(), delim);
}

/**
 * Convenient overload.
 *
 * \param list the initializer list
 * \param delim the delimiter
 * \return the string
 */
template <typename T, typename DelimType = char>
auto join(std::initializer_list<T> list, DelimType delim = ':') -> std::string
{
	return join(list.begin(), list.end(), delim);
}

// }}}

// {{{ is_identifier

/**
 * Check if a string is a valid irccd identifier.
 *
 * \param name the identifier name
 * \return true if is valid
 */
auto is_identifier(std::string_view name) noexcept -> bool;

// }}}

// {{{ is_boolean

/**
 * Check if the value is a boolean, 1, yes and true are accepted.
 *
 * \param value the value
 * \return true if is boolean
 * \note this function is case-insensitive
 */
auto is_boolean(std::string value) noexcept -> bool;

// }}}

// {{{ to_int

/**
 * Convert the given string into a signed integer.
 *
 * \param str the string to convert
 * \param min the minimum value allowed
 * \param max the maximum value allowed
 * \return the value or boost::none if not convertible
 */
template <typename T = int>
auto to_int(const std::string& str,
            T min = std::numeric_limits<T>::min(),
            T max = std::numeric_limits<T>::max()) noexcept -> std::optional<T>
{
	static_assert(std::is_signed<T>::value, "must be signed");

	char* end;
	auto v = std::strtoll(str.c_str(), &end, 10);

	if (*end != '\0' || v < min || v > max)
		return std::nullopt;

	return static_cast<T>(v);
}

// }}}

// {{{ to_uint

/**
 * Convert the given string into a unsigned integer.
 *
 * \note invalid numbers are valid as well
 * \param str the string to convert
 * \param min the minimum value allowed
 * \param max the maximum value allowed
 * \return the value or boost::none if not convertible
 */
template <typename T = unsigned>
auto to_uint(const std::string& str,
             T min = std::numeric_limits<T>::min(),
             T max = std::numeric_limits<T>::max()) noexcept -> std::optional<T>
{
	static_assert(std::is_unsigned<T>::value, "must be unsigned");

	char* end;
	auto v = std::strtoull(str.c_str(), &end, 10);

	if (*end != '\0' || v < min || v > max)
		return std::nullopt;

	return static_cast<T>(v);
}

// }}}

} // !irccd::string_util

#endif // !IRCCD_STRING_UTIL_HPP
