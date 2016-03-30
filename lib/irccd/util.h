/*
 * util.h -- some utilities
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

#ifndef _IRCCD_UTIL_H_
#define _IRCCD_UTIL_H_

#include <ctime>
#include <initializer_list>
#include <regex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace irccd {

namespace util {

/**
 * @enum MessageType
 * @brief Describe which type of message has been received
 *
 * On channels and queries, you may have a special command or a standard message depending on the
 * beginning of the message.
 *
 * Example: `!reminder help' may invoke the command event if a plugin reminder exists.
 */
enum class MessageType {
	Command,		//!< special command
	Message			//!< standard message
};

/**
 * @brief Combine the type of message and its content.
 */
using MessagePair = std::pair<std::string, MessageType>;

/**
 * @class Substitution
 * @brief Used for format() function.
 */
class Substitution {
public:
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
 * Syntax
 * ======
 *
 * The syntax is ?{} where ? is replaced by one of the token defined below. Braces are mandatory and cannot be ommited.
 * To write a literal template construct, prepend the token twice.
 *
 * Availables templates
 * ====================
 *
 * The following templates are available:
 *
 * - #{name}, name will be substituted from the keywords in params
 * - ${name}, name will be substituted from the environment variable
 * - @{attributes}, the attributes will be substituted to IRC colors (see below)
 * - %, any format accepted by strftime(3)
 *
 * Attributes
 * ==========
 *
 * The attribute format is composed of three parts, foreground, background and modifiers, each separated by a comma.
 *
 * **Note:** you cannot omit parameters, to specify the background, you must specify the foreground.
 *
 * Examples
 * ========
 *
 * Valid constructs
 * ----------------
 *
 * - `#{target}, welcome`: if target is set to "irccd", becomes "irccd, welcome",
 * - `@{red}#{target}`: if target is specified, it is written in red,
 *
 * Invalid or literals constructs
 * ------------------------------
 *
 * - `##{target}`: will output "#{target}",
 * - `##`: will output "##",
 * - `#target`: will output "#target",
 * - `#{target`: will throw std::invalid_argument.
 *
 * Colors & attributes
 * -------------------
 *
 * - `@{red,blue}`: will write text red on blue background,
 * - `@{default,yellow}`: will write default color text on yellow background
 * - `@{white,black,bold,underline}`: will write white text on black in both bold and underline
 */
std::string format(std::string text, const Substitution &params = {});

/**
 * Remove leading and trailing spaces.
 *
 * @param str the string
 * @return the removed white spaces
 */
std::string strip(std::string str);

/**
 * Split a string by delimiters.
 *
 * @param list the string to split
 * @param delimiter a list of delimiters
 * @param max max number of split
 * @return a list of string splitted
 */
std::vector<std::string> split(const std::string &list, const std::string &delimiters, int max = -1);

/**
 * Join values by a separator and return a string.
 *
 * @param first the first iterator
 * @param last the last iterator
 * @param delim the optional delimiter
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
 * @param list the initializer list
 * @param delim the delimiter
 * @return the string
 */
template <typename T, typename DelimType = char>
inline std::string join(std::initializer_list<T> list, DelimType delim = ':')
{
	return join(list.begin(), list.end(), delim);
}

/**
 * Parse IRC message and determine if it's a command or a simple message.
 *
 * @param message the message line
 * @param commandChar the command char (e.g '!')
 * @param plugin the plugin name
 * @return the pair
 */
MessagePair parseMessage(std::string message, const std::string &commandChar, const std::string &plugin);

/**
 * Server and identities must have strict names. This function can
 * be used to ensure that they are valid.
 *
 * @param name the identifier name
 * @return true if is valid
 */
inline bool isIdentifierValid(const std::string &name)
{
	return std::regex_match(name, std::regex("[A-Za-z0-9-_]+"));
}

/**
 * Check if the value is a boolean, 1, yes and true are accepted.
 *
 * @param value the value
 * @return true if is boolean
 * @note this function is case-insensitive
 */
bool isBoolean(std::string value) noexcept;

/**
 * Check if the string is an integer.
 *
 * @param value the input
 * @param base the optional base
 * @return true if integer
 */
bool isInt(const std::string &value, int base = 10) noexcept;

/**
 * Check if the string is real.
 *
 * @param value the value
 * @return true if real
 */
bool isReal(const std::string &value) noexcept;

/**
 * Check if the string is a number.
 *
 * @param value the value
 * @return true if it is a number
 */
inline bool isNumber(const std::string &value) noexcept
{
	return isInt(value) || isReal(value);
}

/**
 * Parse a network message from an input buffer and remove it from it.
 *
 * @param input the buffer, will be updated
 * @return the message or empty string if there is nothing
 */
std::string nextNetwork(std::string &input);

} // !util

} // !irccd

#endif // !_IRCCD_UTIL_H_