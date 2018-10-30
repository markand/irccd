/*
 * ini.hpp -- extended .ini file parser
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

#ifndef IRCCD_INI_HPP
#define IRCCD_INI_HPP

/**
 * \file ini.hpp
 * \brief Extended .ini file parser.
 * \author David Demelier <markand@malikania.fr>
 * \version 2.0.0
 */

/**
 * \page Ini Ini
 * \brief Extended .ini file parser.
 * - \subpage ini-syntax
 */

/**
 * \page ini-syntax Syntax
 * \brief File syntax.
 *
 * The syntax is similar to most of `.ini` implementations as:
 *
 * - a section is delimited by `[name]` can be redefined multiple times,
 * - an option **must** always be defined in a section,
 * - empty options must be surrounded by quotes,
 * - lists can not include trailing commas,
 * - include statements must always live at the beginning of files
 *   (in no sections),
 * - comments start with # until the end of line,
 * - options with spaces **must** use quotes.
 *
 * # Basic file
 *
 * ````ini
 * # This is a comment.
 * [section]
 * option1 = value1
 * option2 = "value 2 with spaces"	# comment is also allowed here
 * ````
 *
 * # Redefinition
 *
 * Sections can be redefined multiple times and are kept the order they are
 * seen.
 *
 * ````ini
 * [section]
 * value = "1"
 *
 * [section]
 * value = "2"
 * ````
 *
 * The ini::document object will contains two ini::section.
 *
 * # Lists
 *
 * Lists are defined using `()` and commas, like values, they may have quotes.
 *
 * ````ini
 * [section]
 * names = ( "x1", "x2" )
 *
 * # This is also allowed.
 * biglist = (
 *   "abc",
 *   "def"
 * )
 * ````
 *
 * # Include statement
 *
 * You can split a file into several pieces, if the include statement contains a
 * relative path, the path will be relative to the current file being parsed.
 *
 * You **must** use the include statement before any section.
 *
 * If the file contains spaces, use quotes.
 *
 * ````ini
 * # main.conf
 * @include "foo.conf"
 *
 * # foo.conf
 * [section]
 * option1 = value1
 * ````
 */

#include <algorithm>
#include <exception>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

/**
 * Namespace for ini related classes.
 */
namespace irccd::ini {

class document;

/**
 * \brief exception in a file.
 */
class exception : public std::exception {
private:
	unsigned line_;
	unsigned column_;
	std::string message_;

public:
	/**
	 * Constructor.
	 *
	 * \param line the line
	 * \param column the column
	 * \param msg the message
	 */
	exception(unsigned line, unsigned column, std::string msg) noexcept;

	/**
	 * Get the line number.
	 *
	 * \return the line
	 */
	auto line() const noexcept -> unsigned;

	/**
	 * Get the column number.
	 *
	 * \return the column
	 */
	auto column() const noexcept -> unsigned;

	/**
	 * Return the raw exception message (no line and column shown).
	 *
	 * \return the exception message
	 */
	auto what() const noexcept -> const char* override;
};

/**
 * \brief Describe a token read in the .ini source.
 *
 * This class can be used when you want to parse a .ini file yourself.
 *
 * \see analyse
 */
class token {
public:
	/**
	 * \brief token type.
	 */
	enum type {
		include,                //!< include statement
		tryinclude,             //!< tryinclude statement
		section,                //!< [section]
		word,                   //!< word without quotes
		quoted_word,            //!< word with quotes
		assign,                 //!< = assignment
		list_begin,             //!< begin of list (
		list_end,               //!< end of list )
		comma                   //!< list separation
	};

private:
	type type_;
	unsigned line_;
	unsigned column_;
	std::string value_;

public:
	/**
	 * Construct a token.
	 *
	 * \param type the type
	 * \param line the line
	 * \param column the column
	 * \param value the value
	 */
	token(type type, unsigned line, unsigned column, std::string value = "") noexcept;

	/**
	 * Get the type.
	 *
	 * \return the type
	 */
	auto get_type() const noexcept -> type;

	/**
	 * Get the line.
	 *
	 * \return the line
	 */
	auto get_line() const noexcept -> unsigned;

	/**
	 * Get the column.
	 *
	 * \return the column
	 */
	auto get_column() const noexcept -> unsigned;

	/**
	 * Get the value. For words, quoted words and section, the value is the
	 * content. Otherwise it's the characters parsed.
	 *
	 * \return the value
	 */
	auto get_value() const noexcept -> const std::string&;
};

/**
 * List of tokens in order they are analyzed.
 */
using tokens = std::vector<token>;

/**
 * \brief option definition.
 */
class option : public std::vector<std::string> {
private:
	std::string key_;

public:
	/**
	 * Construct an empty option.
	 *
	 * \pre key must not be empty
	 * \param key the key
	 */
	option(std::string key) noexcept;

	/**
	 * Construct a single option.
	 *
	 * \pre key must not be empty
	 * \param key the key
	 * \param value the value
	 */
	option(std::string key, std::string value) noexcept;

	/**
	 * Construct a list option.
	 *
	 * \pre key must not be empty
	 * \param key the key
	 * \param values the values
	 */
	option(std::string key, std::vector<std::string> values) noexcept;

	/**
	 * Get the option key.
	 *
	 * \return the key
	 */
	auto get_key() const noexcept -> const std::string&;

	/**
	 * Get the option value.
	 *
	 * \return the value
	 */
	auto get_value() const noexcept -> const std::string&;
};

/**
 * \brief Section that contains one or more options.
 */
class section : public std::vector<option> {
private:
	std::string key_;

public:
	/**
	 * Construct a section with its name.
	 *
	 * \pre key must not be empty
	 * \param key the key
	 */
	section(std::string key) noexcept;

	/**
	 * Get the section key.
	 *
	 * \return the key
	 */
	auto get_key() const noexcept -> const std::string&;

	/**
	 * Check if the section contains a specific option.
	 *
	 * \param key the option key
	 * \return true if the option exists
	 */
	auto contains(std::string_view key) const noexcept -> bool;

	/**
	 * Find an option or return an empty one if not found.
	 *
	 * \param key the key
	 * \return the option or empty one if not found
	 */
	auto get(std::string_view key) const noexcept -> option;

	/**
	 * Find an option by key and return an iterator.
	 *
	 * \param key the key
	 * \return the iterator or end() if not found
	 */
	auto find(std::string_view key) noexcept -> iterator;

	/**
	 * Find an option by key and return an iterator.
	 *
	 * \param key the key
	 * \return the iterator or end() if not found
	 */
	auto find(std::string_view key) const noexcept -> const_iterator;

	/**
	 * Access an option at the specified key.
	 *
	 * \param key the key
	 * \return the option
	 * \pre contains(key) must return true
	 */
	auto operator[](std::string_view key) -> option&;

	/**
	 * Overloaded function.
	 *
	 * \param key the key
	 * \return the option
	 * \pre contains(key) must return true
	 */
	auto operator[](std::string_view key) const -> const option&;

	/**
	 * Inherited operators.
	 */
	using std::vector<option>::operator[];
};

/**
 * \brief Ini document description.
 * \see read_file
 * \see read_string
 */
class document : public std::vector<section> {
public:
	/**
	 * Check if a document has a specific section.
	 *
	 * \param key the key
	 * \return true if the document contains the section
	 */
	auto contains(std::string_view key) const noexcept -> bool;

	/**
	 * Find a section or return an empty one if not found.
	 *
	 * \param key the key
	 * \return the section or empty one if not found
	 */
	auto get(std::string_view key) const noexcept -> section;

	/**
	 * Find a section by key and return an iterator.
	 *
	 * \param key the key
	 * \return the iterator or end() if not found
	 */
	auto find(std::string_view key) noexcept -> iterator;

	/**
	 * Find a section by key and return an iterator.
	 *
	 * \param key the key
	 * \return the iterator or end() if not found
	 */
	auto find(std::string_view key) const noexcept -> const_iterator;

	/**
	 * Access a section at the specified key.
	 *
	 * \param key the key
	 * \return the section
	 * \pre contains(key) must return true
	 */
	auto operator[](std::string_view key) -> section&;

	/**
	 * Overloaded function.
	 *
	 * \param key the key
	 * \return the section
	 * \pre contains(key) must return true
	 */
	auto operator[](std::string_view key) const -> const section&;

	/**
	 * Inherited operators.
	 */
	using std::vector<section>::operator[];
};

/**
 * Analyse a stream and detect potential syntax errors. This does not parse the
 * file like including other files in include statement.
 *
 * It does only analysis, for example if an option is defined under no section,
 * this does not trigger an exception while it's invalid.
 *
 * \param it the iterator
 * \param end where to stop
 * \return the list of tokens
 * \throws exception on errors
 */
tokens analyse(std::istreambuf_iterator<char> it, std::istreambuf_iterator<char> end);

/**
 * Overloaded function for stream.
 *
 * \param stream the stream
 * \return the list of tokens
 * \throws exception on errors
 */
tokens analyse(std::istream& stream);

/**
 * Parse the produced tokens.
 *
 * \param tokens the tokens
 * \param path the parent path
 * \return the document
 * \throw exception on errors
 */
document parse(const tokens& tokens, const std::string& path = ".");

/**
 * Parse a file.
 *
 * \param filename the file name
 * \return the document
 * \throw exception on errors
 */
document read_file(const std::string& filename);

/**
 * Parse a string.
 *
 * If the string contains include statements, they are relative to the current
 * working directory.
 *
 * \param buffer the buffer
 * \return the document
 * \throw exception on exceptions
 */
document read_string(const std::string& buffer);

/**
 * Show all tokens and their description.
 *
 * \param tokens the tokens
 */
void dump(const tokens& tokens);

} // !irccd::ini

#endif // !IRCCD_INI_HPP
