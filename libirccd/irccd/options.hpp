/*
 * options.hpp -- C++ similar interface to getopt(3)
 *
 * Copyright (c) 2019 David Demelier <markand@malikania.fr>
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

#ifndef IRCCD_OPTIONS_HPP
#define IRCCD_OPTIONS_HPP

/**
 * \file options.hpp
 * \brief C++ similar interface to getopt(3).
 */

#include <initializer_list>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <vector>

/**
 * \brief C++ similar interface to getopt(3).
 */
namespace irccd::options {

/**
 * Store the positional arguments and options.
 */
using pack = std::tuple<
	std::vector<std::string>,
	std::unordered_multimap<char, std::string>
>;

/**
 * Parse a collection of options and arguments.
 *
 * This function uses the same format as getopt(3) function, you need specify
 * each option in the fmt string and add a colon after the option character if
 * it requires a value.
 *
 * If a -- option appears in the argument list, it stops option parsing and all
 * next tokens are considered arguments even if they start with an hyphen.
 *
 * If the exlamation mark appears in the fmt argument, the function will stop
 * parsing tokens immediately when one argument is not an option.
 *
 * This function explicitly takes references to it and end parameters to allow
 * the user to determine the number of tokens actually parsed.
 *
 * Example of format strings:
 *
 * - "abc": are all three boolean options,
 * - "c:v": v is a boolean option c requires a value.
 *
 * Example of invocation:
 *
 * - `mycli -v -a`: is similar to `-va` if both 'v' and 'a' are boolean options,
 * - `mycli -v -- -c`: -c will be a positional argument rather than an option
 *   but '-v' is still an option.
 *
 * \tparam InputIt must dereference a string type (literal, std::string_view or
 * std::string)
 * \param it the first item
 * \param end the next item
 * \param fmt the format string
 * \return the result
 */
template <typename InputIt>
inline auto parse(InputIt&& it, InputIt&& end, std::string_view fmt) -> pack
{
	pack result;

	for (; it != end; ++it) {
		const std::string_view token(*it);

		/*
		 * Special token that stops parsing options, all next tokens
		 * will be considered as positional arguments.
		 */
		if (token == "--") {
			for (++it; it != end; ++it)
				std::get<0>(result).push_back(std::string(*it));
			break;
		}

		// Is this a positional argument?
		if (token.compare(0U, 1U, "-") != 0) {
			// Stop parsing in case of '!' in format string.
			if (fmt.find('!') != std::string_view::npos)
				break;

			std::get<0>(result).push_back(std::string(token));
			continue;
		}

		const auto sub = token.substr(1);

		for (std::size_t i = 0U; i < sub.size(); ++i) {
			const auto idx = fmt.find(sub[i]);

			if (idx == std::string_view::npos)
				throw std::runtime_error("invalid option");

			// This is a boolean value.
			if (fmt.compare(idx + 1U, 1U, ":") != 0) {
				std::get<1>(result).emplace(sub[i], "");
				continue;
			}

			/*
			 * The value is adjacent to the option (e.g.
			 * -csuper.conf).
			 */
			if (i + 1U < sub.size()) {
				std::get<1>(result).emplace(sub[i], std::string(sub.substr(i + 1)));
				break;
			}

			// Option is the next token (e.g. -c super.conf).
			if (++it == end || std::string_view(*it).compare(0U, 1U, "-") == 0)
				throw std::runtime_error("option require a value");

			std::get<1>(result).emplace(sub[i], std::string(*it));
		}
	}

	return result;
}

/**
 * Convenient overload with an initializer_list.
 *
 * \tparam StringType must be either a std::string or std::string_view
 * \param args the arguments
 * \param fmt the format string
 * \return the result
 */
template <typename String>
inline auto parse(std::initializer_list<String> args, std::string_view fmt) -> pack
{
	auto begin = args.begin();
	auto end = args.end();

	return parse(begin, end, fmt);
}

/**
 * Convenient overload for main() arguments.
 *
 * \param argc the number of arguments
 * \param argv the arguments
 * \param fmt the format string
 * \return the result
 */
inline auto parse(int argc, char** argv, std::string_view fmt) -> pack
{
	std::vector<std::string_view> args(argc);

	for (int i = 0; i < argc; ++i)
		args[i] = argv[i];

	auto begin = args.begin();
	auto end = args.end();

	return parse(begin, end, fmt);
}


} // !irccd::options

#endif // !IRCCD_OPTIONS_HPP
