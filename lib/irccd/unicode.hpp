/*
 * unicode.hpp -- UTF-8 to UTF-32 conversions and various operations
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

#ifndef UNICODE_HPP
#define UNICODE_HPP

/**
 * \file unicode.hpp
 * \brief UTF-8 to UTF-32 conversions
 * \author David Demelier <markand@malikania.fr>
 * \warning These files are auto-generated!
 */

#include <stdexcept>
#include <string>

namespace irccd {

/**
 * \brief Unicode namespace.
 */
namespace unicode {

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
void decode(char32_t &c, const char *res) noexcept;

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
int nbytesUtf8(char c) noexcept;

/**
 * Get the number of bytes for the unicode point.
 *
 * \param point the unicode point
 * \return the number of bytes [1-4] or -1 if invalid
 */
int nbytesPoint(char32_t point) noexcept;

/**
 * Get real number of character in a string.
 *
 * \param str the string
 * \return the length
 * \throw std::invalid_argument on invalid sequence
 */
unsigned length(const std::string &str);

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
void forEach(const std::string &str, Func function)
{
    for (size_t i = 0; i < str.size(); ) {
        char32_t point = 0;
        int size = nbytesUtf8(str[i]);

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
std::string toUtf8(const std::u32string &array);

/**
 * Convert a UTF-8 string to UTF-32 string.
 *
 * \param str the UTF-8 string
 * \return the UTF-32 string
 * \throw std::invalid_argument on invalid sequence
 */
std::u32string toUtf32(const std::string &str);

/**
 * Check if the unicode character is space.
 *
 * \param c the character
 * \return true if space
 */
bool isspace(char32_t c) noexcept;

/**
 * Check if the unicode character is digit.
 *
 * \param c the character
 * \return true if digit
 */
bool isdigit(char32_t c) noexcept;

/**
 * Check if the unicode character is alpha category.
 *
 * \param c the character
 * \return true if alpha
 */
bool isalpha(char32_t c) noexcept;

/**
 * Check if the unicode character is upper case.
 *
 * \param c the character
 * \return true if upper case
 */
bool isupper(char32_t c) noexcept;

/**
 * Check if the unicode character is lower case.
 *
 * \param c the character
 * \return true if lower case
 */
bool islower(char32_t c) noexcept;

/**
 * Check if the unicode character is title case.
 *
 * \param c the character
 * \return true if title case
 */
bool istitle(char32_t c) noexcept;

/**
 * Convert to upper case.
 *
 * \param c the character
 * \return the upper case character
 */
char32_t toupper(char32_t c) noexcept;

/**
 * Convert to lower case.
 *
 * \param c the character
 * \return the lower case character
 */
char32_t tolower(char32_t c) noexcept;

/**
 * Convert to title case.
 *
 * \param c the character
 * \return the title case character
 */
char32_t totitle(char32_t c) noexcept;

/**
 * Convert the UTF-32 string to upper case.
 *
 * \param str the str
 * \return the upper case string
 */
inline std::u32string toupper(std::u32string str)
{
    for (size_t i = 0; i < str.size(); ++i)
        str[i] = toupper(str[i]);

    return str;
}

/**
 * Convert the UTF-8 string to upper case.
 *
 * \param str the str
 * \return the upper case string
 * \warning very slow at the moment
 */
inline std::string toupper(const std::string &str)
{
    std::string result;
    char buffer[5];

    forEach(str, [&] (char32_t code) {
        encode(toupper(code), buffer);
        result += buffer;
    });

    return result;
}

/**
 * Convert the UTF-32 string to lower case.
 *
 * \param str the str
 * \return the lower case string
 */
inline std::u32string tolower(std::u32string str)
{
    for (size_t i = 0; i < str.size(); ++i)
        str[i] = tolower(str[i]);

    return str;
}

/**
 * Convert the UTF-8 string to lower case.
 *
 * \param str the str
 * \return the lower case string
 * \warning very slow at the moment
 */
inline std::string tolower(const std::string &str)
{
    std::string result;
    char buffer[5];

    forEach(str, [&] (char32_t code) {
        encode(tolower(code), buffer);
        result += buffer;
    });

    return result;
}

} // !unicode

} // !irccd

#endif // !UNICODE_HPP
