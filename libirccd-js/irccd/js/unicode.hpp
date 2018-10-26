/*
 * unicode.hpp -- UTF-8 to UTF-32 conversions and various operations
 *
 * Copyright (c) 2013-2018 David Demelier <markand@malikania.fr>
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

#ifndef IRCCD_JS_UNICODE_HPP
#define IRCCD_JS_UNICODE_HPP

/**
 * \file unicode.hpp
 * \brief UTF-8 to UTF-32 conversions
 * \author David Demelier <markand@malikania.fr>
 * \warning These files are auto-generated!
 */

#include <stdexcept>
#include <string>
#include <string_view>

/**
 * \brief Unicode namespace.
 */
namespace irccd::unicode {

/**
 * Encode the unicode code point into multibyte string.
 *
 * \param point the unicode code point
 * \param res the output buffer
 */
void encode(char32_t point, char res[5]) noexcept;

/**
 * Decode the multibyte buffer into an unicode code point.
 *
 * \param c the code point destination
 * \param res the multibyte string.
 */
void decode(char32_t& c, const char* res) noexcept;

/**
 * Get the number of bytes for the first multi byte character from a
 * utf-8 string.
 *
 * This can be used to iterate a valid UTF-8 string to jump to the next
 * real character.
 *
 * \param c the first multi byte character
 * \return the number of bytes [1-4] or -1 if invalid
 */
auto nbytes_utf8(char c) noexcept -> int;

/**
 * Get the number of bytes for the unicode point.
 *
 * \param point the unicode point
 * \return the number of bytes [1-4] or -1 if invalid
 */
auto nbytes_point(char32_t point) noexcept -> int;

/**
 * Get real number of character in a string.
 *
 * \param str the string
 * \return the length
 * \throw std::invalid_argument on invalid sequence
 */
auto length(std::string_view str) -> unsigned;

/**
 * Iterate over all real characters in the UTF-8 string.
 *
 * The function must have the following signature:
 *  void f(char ch)
 *
 * \param str the UTF-8 string
 * \param function the function callback
 * \throw std::invalid_argument on invalid sequence
 */
template <typename Func>
void for_each(std::string_view str, Func function)
{
	for (size_t i = 0; i < str.size(); ) {
		char32_t point = 0;
		int size = nbytes_utf8(str[i]);

		if (size < 0)
			throw std::invalid_argument("invalid sequence");

		decode(point, str.data() + i);
		function(point);

		i += size;
	}
}

/**
 * Convert a UTF-32 string to UTF-8 string.
 *
 * \param array the UTF-32 string
 * \return the UTF-8 string
 * \throw std::invalid_argument on invalid sequence
 */
auto to_utf8(std::u32string_view array) -> std::string;

/**
 * Convert a UTF-8 string to UTF-32 string.
 *
 * \param str the UTF-8 string
 * \return the UTF-32 string
 * \throw std::invalid_argument on invalid sequence
 */
auto to_utf32(std::string_view str) -> std::u32string;

/**
 * Check if the unicode character is space.
 *
 * \param c the character
 * \return true if space
 */
auto isspace(char32_t c) noexcept -> bool;

/**
 * Check if the unicode character is digit.
 *
 * \param c the character
 * \return true if digit
 */
auto isdigit(char32_t c) noexcept -> bool;

/**
 * Check if the unicode character is alpha category.
 *
 * \param c the character
 * \return true if alpha
 */
auto isalpha(char32_t c) noexcept -> bool;

/**
 * Check if the unicode character is upper case.
 *
 * \param c the character
 * \return true if upper case
 */
auto isupper(char32_t c) noexcept -> bool;

/**
 * Check if the unicode character is lower case.
 *
 * \param c the character
 * \return true if lower case
 */
auto islower(char32_t c) noexcept -> bool;

/**
 * Check if the unicode character is title case.
 *
 * \param c the character
 * \return true if title case
 */
auto istitle(char32_t c) noexcept -> bool;

/**
 * Convert to upper case.
 *
 * \param c the character
 * \return the upper case character
 */
auto toupper(char32_t c) noexcept -> char32_t;

/**
 * Convert to lower case.
 *
 * \param c the character
 * \return the lower case character
 */
auto tolower(char32_t c) noexcept -> char32_t;

/**
 * Convert to title case.
 *
 * \param c the character
 * \return the title case character
 */
auto totitle(char32_t c) noexcept -> char32_t;

/**
 * Convert the UTF-32 string to upper case.
 *
 * \param str the string
 * \return the upper case string
 */
auto toupper(std::u32string_view str) -> std::u32string;

/**
 * Convert the UTF-8 string to upper case.
 *
 * \param str the string
 * \return the upper case string
 * \warning very slow at the moment
 */
auto toupper(std::string_view str) -> std::string;

/**
 * Convert the UTF-32 string to lower case.
 *
 * \param str the string
 * \return the lower case string
 */
auto tolower(std::u32string_view str) -> std::u32string;

/**
 * Convert the UTF-8 string to lower case.
 *
 * \param str the string
 * \return the lower case string
 * \warning very slow at the moment
 */
auto tolower(std::string_view str) -> std::string;

} // !irccd::unicode

#endif // !IRCCD_JS_UNICODE_HPP
